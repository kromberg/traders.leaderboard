#include <queue>

#include <bsoncxx/json.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

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

    m_logger = logger::Logger::getLogCategory("DB_MONGO");
}

Result MongodbStorage::storeUser(const int64_t id, const std::string& name)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value userDocument = builder <<
        "id" << id <<
        "name" << name <<
        bsoncxx::builder::stream::finalize;

    mongocxx::stdx::optional<mongocxx::result::insert_one> result =
        m_collection.insert_one(std::move(userDocument));

    mongocxx::stdx::optional<bsoncxx::document::value> maybeResult =
        m_collection.find_one(document{} << "id" << id << finalize);
    if (maybeResult)
    {
        LOG_DEBUG(m_logger, "User was registered in mongodb: %s",
            bsoncxx::to_json(*maybeResult).c_str());
    }

    LOG_DEBUG(m_logger, "User was registered <id: %ld, name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result MongodbStorage::renameUser(const int64_t id, const std::string& name)
{
    mongocxx::stdx::optional<bsoncxx::document::value> maybeResult =
        m_collection.find_one(document{} << "id" << id << finalize);
    if (!maybeResult)
    {
        LOG_ERROR(m_logger, "Cannot rename user <id: %ld, name: %s>. User is not found",
            id, name.c_str());
        return Result::USER_NOT_FOUND;
    }
    LOG_DEBUG(m_logger, "User was found in mongodb: %s",
        bsoncxx::to_json(*maybeResult).c_str());

    m_collection.update_one(
        document{} << "id" << id << finalize,
        document{} << "$set" <<
        open_document <<
        "name" << name <<
        close_document <<
        finalize);
    LOG_DEBUG(m_logger, "User was renamed <id: %ld new name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result MongodbStorage::storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
{
    using std::chrono::system_clock;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    auto userRes = m_collection.find_one(document{} << "id" << id << finalize);
    if (!userRes)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, ctime(&t), amount);
        return Result::USER_NOT_FOUND;
    }
    LOG_DEBUG(m_logger, "User was found in mongodb: %s",
        bsoncxx::to_json(*userRes).c_str());

    system_clock::time_point tp = system_clock::from_time_t(t);
    int64_t timeValue = duration_cast<seconds>(tp.time_since_epoch()).count();

    auto updateResult = m_collection.update_one(
        document{} <<
        "id" << id << 
        "scores.time" << timeValue <<
        finalize,
        document{} <<
        "$inc" <<
        open_document <<
        "scores.$.score" << amount <<
        close_document <<
        finalize);
    if (!updateResult)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, ctime(&t), amount);
        return Result::OPERATION_ERROR;
    }
    LOG_DEBUG(m_logger, "Update result: <modified_count: %d; matched_count: %d>",
        (*updateResult).modified_count(), (*updateResult).matched_count());

    if (0 == (*updateResult).modified_count())
    {
        auto addResult = m_collection.update_one(
            document{} <<
            "id" << id << 
            finalize,
            document{} <<
            "$addToSet" <<
            open_document <<
            "scores" <<
            open_document <<
            "time" << timeValue <<
            "score" << amount <<
            close_document <<
            close_document <<
            finalize);
        if (!addResult)
        {
            LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
                id, ctime(&t), amount);
            return Result::OPERATION_ERROR;
        }
        LOG_DEBUG(m_logger, "Add result: <modified_count: %d; matched_count: %d>",
            (*addResult).modified_count(), (*addResult).matched_count());
    }

    userRes = m_collection.find_one(document{} << "id" << id << finalize);
    if (!userRes)
    {
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, ctime(&t), amount);
        return Result::USER_NOT_FOUND;
    }
    LOG_DEBUG(m_logger, "User deal was stored in mongodb: %s",
        bsoncxx::to_json(*userRes).c_str());

    LOG_DEBUG(m_logger, "User deal was stored <id: %ld, time: %s, amount: %ld>",
        id, ctime(&t), amount);

    return Result::SUCCESS;
}

Result MongodbStorage::getUser(User& user, const int64_t id) const
{
    // todo
    return Result::SUCCESS;
}

Result MongodbStorage::getUserLeaderboard(
    Leaderboard& lb,
    const int64_t id,
    const uint64_t before,
    const uint64_t after)
        const
{
    // todo

    return Result::SUCCESS;
}

Result MongodbStorage::getLeaderboard(
    Leaderboard& lb,
    const int64_t count)
        const
{
    // todo

    return Result::SUCCESS;
}


} // namespace db