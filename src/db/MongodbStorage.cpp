#include <queue>

#include <bsoncxx/json.hpp>
#include <bsoncxx/exception/exception.hpp>
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

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

MongodbStorage::MongodbStorage()
{
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::uri uri("mongodb://localhost:27017");
    m_client = mongocxx::client(uri);
    m_db = m_client["db"];
    m_collection = m_db["test"];
    m_connectedUsersCollection = m_db["connected_users"];

    m_logger = logger::Logger::getLogCategory("DB_MONGO");
}

Result MongodbStorage::storeUser(const int64_t id, const std::string& name)
{
    mongocxx::stdx::optional<mongocxx::result::insert_one> result;
    try
    {
        result =
            m_collection.insert_one(
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
    mongocxx::stdx::optional<mongocxx::result::update> updateResult;
    try
    {
        updateResult = 
            m_collection.update_one(
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

    mongocxx::stdx::optional<bsoncxx::document::value> userRes;
    try
    {
        userRes = m_collection.find_one(document{} << "id" << id << finalize);
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
            m_collection.update_one(
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
                m_collection.update_one(
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
    mongocxx::stdx::optional<mongocxx::result::insert_one> result;
    try
    {
        result =
            m_connectedUsersCollection.insert_one(
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
    mongocxx::stdx::optional<mongocxx::result::delete_result> result;
    try
    {
        result =
            m_connectedUsersCollection.delete_one(
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
    mongocxx::cursor cursor =
        m_connectedUsersCollection.find(
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
        LOG_WARN(m_logger, "Cannot retreive connected users");
        return result;
    }
    if (badDocuments > 0)
    {
        LOG_WARN(m_logger, "Connected users. Failed to process %lu documents", badDocuments);
    }
    LOG_DEBUG(m_logger, "Connected users. Processed %lu documents", goodDocuments);

    return result;
}

Result MongodbStorage::getUser(User& user, const int64_t id) const
{
    mongocxx::stdx::optional<bsoncxx::document::value> userRes;
    try
    {
        userRes = m_collection.find_one(document{} << "id" << id << finalize);
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
        if (id.type() != bsoncxx::type::k_utf8)
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
    mongocxx::cursor cursor = m_collection.aggregate(pipeline);

    Leaderboard tmpLeaderboard;
    Leaderboard currentLeaderboard;
    std::unordered_map<int64_t, uint16_t> currentIdsToCount;

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

                auto currentIdIt = currentIdsToCount.begin();
                while (currentIdIt != currentIdsToCount.end())
                {
                    auto lbIt = leaderboards.find(currentIdIt->first);
                    if (leaderboards.end() == lbIt)
                    {
                        LOG_ERROR(m_logger, "Cannot find leaderborad for user %ld", lbIt->first);
                        ++ currentIdIt;
                        continue;
                    }
                    lbIt->second.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(score.get_int64()),
                        std::forward_as_tuple(id.get_int64(), name.get_utf8().value.to_string()));

                    if (currentIdIt->second + 1 >= after)
                    {
                        currentIdIt = currentIdsToCount.erase(currentIdIt);
                    }
                    else
                    {
                        ++ currentIdIt->second;
                        ++ currentIdIt;
                    }
                }

                auto idIt = connectedUsers.find(id.get_int64());
                if (connectedUsers.end() != idIt)
                {
                    LOG_DEBUG(m_logger, "User %ld found: adding leaderborad", *idIt);
                    currentIdsToCount.emplace(*idIt, 0);
                    leaderboards.emplace(*idIt, currentLeaderboard);
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
        LOG_WARN(m_logger, "Cannot get leaderboard from DB");
        return Result::DB_ERROR;
    }
    if (badDocuments > 0)
    {
        LOG_WARN(m_logger, "Leaderboard. Failed to process %lu documents", badDocuments);
    }
    LOG_DEBUG(m_logger, "Leaderboard. Processed %lu documents", goodDocuments);

    leaderboards.emplace(-1, std::move(tmpLeaderboard));

    return Result::SUCCESS;
}

} // namespace db