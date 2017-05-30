#include <queue>

#include <libconfig.h++>

#include <bsoncxx/json.hpp>
#include <bsoncxx/exception/exception.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>

#include <logger/LoggerDefines.h>
#include <db/MongodbStorage.h>

namespace db
{

#define GET_COLLECTION(collectionName) \
    mongocxx::pool::entry client; \
    { \
        std::unique_lock<std::mutex> l(m_poolGuard); \
        client = m_pool->acquire(); \
    } \
    mongocxx::database database = (*client)[m_dbName]; \
    mongocxx::collection collection = database[collectionName]

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

MongodbStorage::MongodbStorage()
{
    m_logger = logger::Logger::getLogCategory("DB_MONGO");
}

Result MongodbStorage::configure(const libconfig::Config& cfg)
{
    using namespace libconfig;

    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure storage in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    m_uri = "mongodb://localhost:27017";
    m_dbName = "leaderboard_db";
    m_usersCollectionName = "users";
    m_connectedUsersCollectionName = "connected_users";
    try
    {
        const Setting& setting = cfg.lookup("db");
        if (!setting.lookupValue("address", m_uri))
        {
            LOG_WARN(m_logger, "Canont find 'address' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("db_name", m_dbName))
        {
            LOG_WARN(m_logger, "Canont find 'db_name' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("users_collection_name", m_usersCollectionName))
        {
            LOG_WARN(m_logger, "Canont find 'users_collection_name' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("connected_users_collection_name", m_connectedUsersCollectionName))
        {
            LOG_WARN(m_logger, "Canont find 'connected_users_collection_name' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find 'db' section in configuration. Default values will be used");
    }

    LOG_INFO(m_logger, "Configuration parameters: <uri: %s, db_name: %s, "
        "users_collection_name: %s, connected_users_collection_name: %s>",
        m_uri.c_str(), m_dbName.c_str(), m_usersCollectionName.c_str(), m_connectedUsersCollectionName.c_str());

    m_state = State::CONFIGURED;
    return Result::SUCCESS;
}

Result MongodbStorage::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start storage in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    // this should be called only once
    static mongocxx::instance instance{};

    // create collections
    mongocxx::uri uri(m_uri);
    try
    {
        m_pool.reset(new mongocxx::pool(uri));
    }
    catch (const mongocxx::exception& e)
    {
        LOG_ERROR(m_logger, "Cannot create MongoDB client, exception was thrown %s", e.what());
        return Result::DB_ERROR;
    }

    m_state = State::STARTED;
    return Result::SUCCESS;
}

Result MongodbStorage::storeUser(const int64_t id, const std::string& name)
{
    GET_COLLECTION(m_usersCollectionName);

    mongocxx::stdx::optional<mongocxx::result::insert_one> result;
    try
    {
        result =
            collection.insert_one(
                document{} <<
                "_id" << id <<
                "id" << id <<
                "name" << name <<
                finalize);
    }
    catch (const mongocxx::bulk_write_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot register user <id: %ld, name: %s>. Exception was thrown %s",
            id, name.c_str(), e.what());
        return Result::DB_ERROR;
    }
    if (!result)
    {
        LOG_ERROR(m_logger, "Cannot register user <id: %ld, name: %s>",
            id, name.c_str());
        return Result::USER_REG_ERROR;
    }

    LOG_DEBUG(m_logger, "User was registered <id: %ld, name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result MongodbStorage::renameUser(const int64_t id, const std::string& name)
{
    GET_COLLECTION(m_usersCollectionName);

    mongocxx::stdx::optional<mongocxx::result::update> updateResult;
    try
    {
        updateResult = 
            collection.update_one(
                document{} << "id" << id << finalize,
                document{} << "$set" <<
                open_document <<
                "name" << name <<
                close_document <<
                finalize);
    }
    catch (const mongocxx::bulk_write_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot rename user <id: %ld, name: %s>. Exception was thrown: %s",
            id, name.c_str(), e.what());
        return Result::DB_ERROR;
    }
    catch (const std::logic_error& e)
    {
        LOG_ERROR(m_logger, "Cannot rename user <id: %ld, name: %s>. Exception was thrown: %s",
            id, name.c_str(), e.what());
        return Result::LOGIC_ERROR;
    }
    if (!updateResult || ((*updateResult).modified_count() == 0))
    {
        LOG_ERROR(m_logger, "Cannot rename user <id: %ld, name: %s>. User is not found",
            id, name.c_str());
        return Result::USER_NOT_FOUND;
    }
    LOG_DEBUG(m_logger, "User was renamed <id: %ld new name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result MongodbStorage::storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
{
    using std::chrono::system_clock;

    GET_COLLECTION(m_usersCollectionName);

    mongocxx::stdx::optional<bsoncxx::document::value> userRes;
    try
    {
        userRes = collection.find_one(document{} << "id" << id << finalize);
    }
    catch (const mongocxx::query_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. Exception was thrown: %s",
            id, common::timeToString(t).c_str(), amount, e.what());
        return Result::DB_ERROR;
    }
    if (!userRes)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, common::timeToString(t).c_str(), amount);
        return Result::USER_NOT_FOUND;
    }
    LOG_DEBUG(m_logger, "User was found in mongodb: %s",
        bsoncxx::to_json(*userRes).c_str());

    system_clock::time_point tp = system_clock::from_time_t(t);
    //int64_t timeValue = duration_cast<seconds>(tp.time_since_epoch()).count();

    mongocxx::stdx::optional<mongocxx::result::update> updateResult;
    try
    {
        updateResult = 
            collection.update_one(
                document{} <<
                "id" << id << 
                "scores.time" << bsoncxx::types::b_date(tp) <<
                finalize,
                document{} <<
                "$inc" <<
                open_document <<
                "scores.$.score" << amount <<
                close_document <<
                finalize);
    }
    catch (const mongocxx::bulk_write_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. Exception was thrown: %s",
            id, common::timeToString(t).c_str(), amount, e.what());
        return Result::DB_ERROR;
    }
    catch (const std::logic_error& e)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. Exception was thrown: %s",
            id, common::timeToString(t).c_str(), amount, e.what());
        return Result::LOGIC_ERROR;
    }
    if (!updateResult)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, common::timeToString(t).c_str(), amount);
        return Result::UPDATE_ERROR;
    }
    LOG_DEBUG(m_logger, "Update result: <modified_count: %d; matched_count: %d>",
        (*updateResult).modified_count(), (*updateResult).matched_count());

    if (0 == (*updateResult).modified_count())
    {
        mongocxx::stdx::optional<mongocxx::result::update> addResult;
        try
        {
            addResult =
                collection.update_one(
                    document{} <<
                    "id" << id << 
                    finalize,
                    document{} <<
                    "$addToSet" <<
                    open_document <<
                    "scores" <<
                    open_document <<
                    "time" << bsoncxx::types::b_date(tp) <<
                    "score" << amount <<
                    close_document <<
                    close_document <<
                    finalize);
        }
        catch (const mongocxx::bulk_write_exception& e)
        {
            LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. Exception was thrown: %s",
                id, common::timeToString(t).c_str(), amount, e.what());
            return Result::DB_ERROR;
        }
        if (!addResult)
        {
            LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
                id, common::timeToString(t).c_str(), amount);
            return Result::UPDATE_ERROR;
        }
        LOG_DEBUG(m_logger, "Add result: <modified_count: %d; matched_count: %d>",
            (*addResult).modified_count(), (*addResult).matched_count());
    }

    LOG_DEBUG(m_logger, "User deal was stored <id: %ld, time: %s, amount: %ld>",
        id, common::timeToString(t).c_str(), amount);
    return Result::SUCCESS;
}

Result MongodbStorage::storeConnectedUser(const int64_t id)
{
    GET_COLLECTION(m_connectedUsersCollectionName);

    mongocxx::stdx::optional<mongocxx::result::insert_one> result;
    try
    {
        result =
            collection.insert_one(
                document{} <<
                "_id" << id <<
                "id" << id <<
                finalize);
    }
    catch (const mongocxx::bulk_write_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot store connected user <id: %ld>. Exception was thrown: %s",
            id, e.what());
        return Result::DB_ERROR;
    }
    if (!result)
    {
        LOG_ERROR(m_logger, "Cannot store connected user <id: %ld>", id);
        return Result::USER_CONN_ERROR;
    }

    LOG_DEBUG(m_logger, "Connected user was stored <id: %ld>", id);
    return Result::SUCCESS;
}

Result MongodbStorage::removeConnectedUser(const int64_t id)
{
    GET_COLLECTION(m_connectedUsersCollectionName);

    mongocxx::stdx::optional<mongocxx::result::delete_result> result;
    try
    {
        result =
            collection.delete_one(
                document{} <<
                "id" << id <<
                finalize);
    }
    catch (const mongocxx::bulk_write_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot remove connected user <id: %ld>. Exception was thrown: %s",
            id, e.what());
        return Result::DB_ERROR;
    }
    if (!result)
    {
        LOG_ERROR(m_logger, "Cannot remove connected user <id: %ld>", id);
        return Result::USER_CONN_ERROR;
    }
    if (0 == (*result).deleted_count())
    {
        LOG_ERROR(m_logger, "Cannot remove connected user <id: %ld>", id);
        return Result::USER_CONN_ERROR;
    }

    LOG_DEBUG(m_logger, "Connected user was remove <id: %ld>", id);
    return Result::SUCCESS;
}

std::unordered_set<int64_t> MongodbStorage::getConnectedUsers() const
{
    GET_COLLECTION(m_connectedUsersCollectionName);

    mongocxx::cursor cursor =
        collection.find(
            document{} <<
            finalize);

    std::unordered_set<int64_t> result;
    uint64_t goodDocuments = 0, badDocuments = 0;
    try
    {
        for (const bsoncxx::document::view& view : cursor)
        {
            LOG_DEBUG(m_logger, "Connected users. Got document : %s",
                bsoncxx::to_json(view).c_str());

            try
            {
                bsoncxx::document::element id = view["id"];
                if (id.type() != bsoncxx::type::k_int64)
                {
                    LOG_DEBUG(m_logger, "Cannot get 'id' from the document");
                    ++ badDocuments;
                    continue;
                }
                result.emplace(id.get_int64());
                ++ goodDocuments;
            }
            catch (const bsoncxx::exception& e)
            {
                LOG_DEBUG(m_logger, "Exception '%s' was thrown while parsing document",
                    e.what());
                ++ badDocuments;
                continue;
            }
        }
    }
    catch (const mongocxx::query_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot retreive connected users, exception was thrown %s", e.what());
        return result;
    }
    if (badDocuments > 0)
    {
        LOG_WARN(m_logger, "Connected users. Failed to process %lu documents", badDocuments);
    }
    LOG_DEBUG(m_logger, "Connected users. Processed %lu documents", goodDocuments);

    return result;
}

Result MongodbStorage::getUser(User& user, const int64_t id, mongocxx::collection& collection) const
{
    mongocxx::stdx::optional<bsoncxx::document::value> userRes;
    try
    {
        userRes = collection.find_one(document{} << "id" << id << finalize);
    }
    catch (const mongocxx::query_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot find user <id: %ld>. Exception was thrown: %s",
            id, e.what());
        return Result::DB_ERROR;
    }
    if (!userRes)
    {
        LOG_ERROR(m_logger, "Cannot find user <id: %ld>", id);
        return Result::USER_NOT_FOUND;
    }

    bsoncxx::document::view view = (*userRes).view();

    LOG_DEBUG(m_logger, "Get user. Got document : %s",
        bsoncxx::to_json(view).c_str());

    try
    {
        bsoncxx::document::element id = view["id"];
        if (id.type() != bsoncxx::type::k_int64)
        {
            LOG_ERROR(m_logger, "Cannot get 'id' from the document");
            return Result::DB_ERROR;
        }
        bsoncxx::document::element name = view["name"];
        if (name.type() != bsoncxx::type::k_utf8)
        {
            LOG_ERROR(m_logger, "Cannot get 'name' from the document");
            return Result::DB_ERROR;
        }
        user.m_id = id.get_int64();
        user.m_name = name.get_utf8().value.to_string();
    }
    catch (const bsoncxx::exception& e)
    {
        LOG_ERROR(m_logger, "Exception '%s' was thrown while parsing document",
            e.what());
        return Result::DB_ERROR;
    }
    LOG_DEBUG(m_logger, "Found user: <id: %ld, name: %s>", user.m_id, user.m_name.c_str());
    return Result::SUCCESS;
}

Result MongodbStorage::getUser(User& user, const int64_t id) const
{
    GET_COLLECTION(m_usersCollectionName);

    return getUser(user, id, collection);
}

Result MongodbStorage::getLeaderboards(
    Leaderboards& leaderboards,
    const int64_t count,
    const uint64_t before,
    const uint64_t after)
        const
{
    using std::chrono::system_clock;
    typedef std::chrono::duration<int, std::ratio<24 * 60 * 60> > duration_days;

    std::unordered_set<int64_t> connectedUsers = getConnectedUsers();

    // last week
    system_clock::time_point tp = system_clock::now() - duration_days(7);

    GET_COLLECTION(m_usersCollectionName);
    mongocxx::pipeline pipeline;
    pipeline
        .unwind("$scores")
        .match(
            document{} <<
            "scores.time" <<
            open_document <<
            "$gt" << bsoncxx::types::b_date(tp) <<
            close_document <<
            finalize)
        .group(
            document{} <<
            "_id" << 
            open_document <<
            "id" << "$id" <<
            "name" << "$name" <<
            close_document <<
            "weeklyScore" << 
            open_document <<
            "$sum" << "$scores.score" <<
            close_document <<
            finalize)
        .sort(
            document{} <<
            "weeklyScore" << -1 <<
            finalize);
    mongocxx::cursor cursor = collection.aggregate(pipeline);

    Leaderboard tmpLeaderboard;
    Leaderboard currentLeaderboard;
    std::map<User, uint16_t> userToCount;

    uint64_t goodDocuments = 0;
    uint64_t badDocuments = 0;
    try
    {
        for (const bsoncxx::document::view& view : cursor)
        {
            LOG_DEBUG(m_logger, "Leaderboard. Got document : %s",
                bsoncxx::to_json(view).c_str());

            try
            {
                bsoncxx::document::element _id = view["_id"];
                if (_id.type() != bsoncxx::type::k_document)
                {
                    LOG_DEBUG(m_logger, "Cannot get '_id' from the document");
                    ++ badDocuments;
                    continue;
                }

                bsoncxx::document::view _idView = _id.get_document().view();
                bsoncxx::document::element id = _idView["id"];
                if (id.type() != bsoncxx::type::k_int64)
                {
                    LOG_DEBUG(m_logger, "Cannot get 'id' from the document");
                    ++ badDocuments;
                    continue;
                }

                bsoncxx::document::element name = _idView["name"];
                if (name.type() != bsoncxx::type::k_utf8)
                {
                    LOG_DEBUG(m_logger, "Cannot get 'name' from the document");
                    ++ badDocuments;
                    continue;
                }

                bsoncxx::document::element score = view["weeklyScore"];
                if (score.type() != bsoncxx::type::k_int64)
                {
                    LOG_DEBUG(m_logger, "Cannot get 'weeklyScore' from the document");
                    ++ badDocuments;
                    continue;
                }

                ++ goodDocuments;

                if ((count <= 0) || 
                    (count > 0 && tmpLeaderboard.size() < count))
                {
                    tmpLeaderboard.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(score.get_int64()),
                        std::forward_as_tuple(id.get_int64(), name.get_utf8().value.to_string()));
                }

                currentLeaderboard.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(score.get_int64()),
                    std::forward_as_tuple(id.get_int64(), name.get_utf8().value.to_string()));

                auto userToCountIt = userToCount.begin();
                while (userToCountIt != userToCount.end())
                {
                    auto lbIt = leaderboards.find(userToCountIt->first);
                    if (leaderboards.end() == lbIt)
                    {
                        LOG_ERROR(m_logger, "Cannot find leaderboard for user %ld:%s",
                            userToCountIt->first.m_id, userToCountIt->first.m_name.c_str());
                        ++ userToCountIt;
                        continue;
                    }
                    lbIt->second.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(score.get_int64()),
                        std::forward_as_tuple(id.get_int64(), name.get_utf8().value.to_string()));

                    if (userToCountIt->second + 1 >= after)
                    {
                        userToCountIt = userToCount.erase(userToCountIt);
                    }
                    else
                    {
                        ++ userToCountIt->second;
                        ++ userToCountIt;
                    }
                }

                auto idIt = connectedUsers.find(id.get_int64());
                if (connectedUsers.end() != idIt)
                {
                    User user;
                    Result res = getUser(user, *idIt, collection);
                    if (Result::SUCCESS != res)
                    {
                        LOG_ERROR(m_logger, "Cannot find user with id %ld. Result: %d(%s)",
                            *idIt, static_cast<int32_t>(res), common::resultToStr(res));
                    }
                    else
                    {
                        LOG_DEBUG(m_logger, "User %ld:%s found: adding leaderboard", user.m_id, user.m_name.c_str());
                        userToCount.emplace(user, 0);
                        leaderboards.emplace(user, currentLeaderboard);
                    }
                }

                if (currentLeaderboard.size() > before)
                {
                    currentLeaderboard.erase(currentLeaderboard.begin());
                }
            }
            catch (const bsoncxx::exception& e)
            {
                LOG_DEBUG(m_logger, "Exception '%s' was thrown while parsing document",
                    e.what());
                ++ badDocuments;
                continue;
            }
        }
    }
    catch (const mongocxx::query_exception& e)
    {
        LOG_ERROR(m_logger, "Cannot get leaderboard from DB, exception was thrown %s", e.what());
        return Result::DB_ERROR;
    }
    if (badDocuments > 0)
    {
        LOG_WARN(m_logger, "Leaderboard. Failed to process %lu documents", badDocuments);
    }
    LOG_DEBUG(m_logger, "Leaderboard. Processed %lu documents", goodDocuments);

    leaderboards.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(-1, "Top"),
        std::forward_as_tuple(std::move(tmpLeaderboard)));

    return Result::SUCCESS;
}

} // namespace db