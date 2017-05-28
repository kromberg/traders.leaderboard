#include <gtest/gtest.h>

#include <common/Utils.h>
#include <app/MessageParser.h>
#include <rabbitmq/ProcessingItem.h>

#include "../fixtures/LoggerFixture.h"

using common::Result;

class MessageParserFixture : public LoggerFixture
{};

TEST_F(MessageParserFixture, ParserNoCallbacks)
{
    app::MessageParser parser;

    rabbitmq::ProcessingItem item(nullptr, "", "", "", 0, false);

    std::vector<std::pair<std::string, Result> > commands = 
    {
        { "user_registered(1,Arr)", Result::CB_NOT_FOUND },
        { "user_renamed(1,Brr)", Result::CB_NOT_FOUND },
        { "user_deal(1,2017-05-20T10:10:10,100)", Result::CB_NOT_FOUND },
        { "user_deal_won(1,2017-05-20T10:10:10,100)", Result::CB_NOT_FOUND },
        { "user_connected(1)", Result::CB_NOT_FOUND },
        { "user_disconnected(1)", Result::CB_NOT_FOUND },
    };
    for (auto&& cmd : commands)
    {
        item.m_message = cmd.first;
        ASSERT_EQ(cmd.second, parser.parseMessage(std::move(item)));
    }
}

class MessageParserCallbacks
{
private:
    int64_t m_id;
    std::string m_name;
    int64_t m_amount;
    std::time_t m_time;

    bool m_isCalled = false;
    Result m_result = Result::SUCCESS;

public:
    int64_t id() const
    {
        return m_id;
    }
    const std::string& name() const
    {
        return m_name;
    }
    int64_t amount() const
    {
        return m_amount;
    }
    std::time_t time() const
    {
        return m_time;
    }

    void setResult(const Result r)
    {
        m_result = r;
    }

    bool isCalled() const
    {
        return m_isCalled;
    }
    void reset()
    {
        m_isCalled = false;
        m_id = -1;
        m_name = "";
        m_amount = -1;
        m_time = ::time(nullptr);
    }
    // user_registered(id,name)
    Result onUserRegistered(const int64_t id, const std::string& name)
    {
        m_id = id;
        m_name = name;
        m_isCalled = true;
        return m_result;
    }
    // user_renamed(id,name)
    Result onUserRenamed(const int64_t id, const std::string& name)
    {
        m_id = id;
        m_name = name;
        m_isCalled = true;
        return m_result;
    }
    // user_deal(id,time,amount)
    Result onUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
    {
        m_id = id;
        m_time = t;
        m_amount = amount;
        m_isCalled = true;
        return m_result;
    }
    // user_deal_won(id,time,amount)
    Result onUserDealWon(const int64_t id, const std::time_t t, const int64_t amount)
    {
        m_id = id;
        m_time = t;
        m_amount = amount;
        m_isCalled = true;
        return m_result;
    }
    // user_connected(id)
    Result onUserConnected(const int64_t id)
    {
        m_id = id;
        m_isCalled = true;
        return m_result;
    }
    // user_disconnected(id)
    Result onUserDisconnected(const int64_t id)
    {
        m_id = id;
        m_isCalled = true;
        return m_result;
    }
};


TEST_F(MessageParserFixture, BadMessages)
{
    app::MessageParser parser;
    MessageParserCallbacks callbacks;
    parser.registerCallbackObject(callbacks);

    rabbitmq::ProcessingItem item(nullptr, "", "", "", 0, false);

    std::vector<std::pair<std::string, Result> > commands =
    {
        { "user_registered(Arrr,Arr)", Result::INVALID_FORMAT },
        { "user_registered(1,Arr", Result::INVALID_FORMAT },
        { "user_registered(Arr)", Result::INVALID_FORMAT },

        { "user_renamed(Brrr,Brr)", Result::INVALID_FORMAT },
        { "user_renamed(1,Brr", Result::INVALID_FORMAT },
        { "user_renamed(Brr)", Result::INVALID_FORMAT },

        { "user_deal(1,1234,100)", Result::INVALID_FORMAT },
        { "user_deal(Arrr,2017-05-20T10:10:10,100)", Result::INVALID_FORMAT },
        { "user_deal(1,2017-05-20T10:10:10,Arrr)", Result::INVALID_FORMAT },
        { "user_deal(1,2017-05-20T10:10:10,100", Result::INVALID_FORMAT },
        { "user_deal(1)", Result::INVALID_FORMAT },

        { "user_deal_won(1,1234,100)", Result::INVALID_FORMAT },
        { "user_deal_won(Arrr,2017-05-20T10:10:10,100)", Result::INVALID_FORMAT },
        { "user_deal_won(1,2017-05-20T10:10:10,Arrr)", Result::INVALID_FORMAT },
        { "user_deal_won(1,2017-05-20T10:10:10,100", Result::INVALID_FORMAT },
        { "user_deal_won(1)", Result::INVALID_FORMAT },

        { "user_connected(Arrr)", Result::INVALID_FORMAT },
        { "user_connected(1", Result::INVALID_FORMAT },
        { "user_connected()", Result::INVALID_FORMAT },

        { "user_disconnected(Arrr)", Result::INVALID_FORMAT },
        { "user_disconnected(1", Result::INVALID_FORMAT },
        { "user_disconnected()", Result::INVALID_FORMAT },
    };
    for (auto&& cmd : commands)
    {
        item.m_message = cmd.first;
        ASSERT_EQ(cmd.second, parser.parseMessage(std::move(item)));
    }
}

TEST_F(MessageParserFixture, ParserBadResult)
{
    app::MessageParser parser;
    MessageParserCallbacks callbacks;
    parser.registerCallbackObject(callbacks);

    rabbitmq::ProcessingItem item(nullptr, "", "", "", 0, false);

    Result res = Result::DB_ERROR;
    std::vector<std::pair<std::string, Result> > commands = 
    {
        { "user_registered(1,Arr)", res },
        { "user_renamed(1,Brr)", res},
        { "user_deal(1,2017-05-20T10:10:10,100)", res},
        { "user_deal_won(1,2017-05-20T10:10:10,100)", res},
        { "user_connected(1)", res},
        { "user_disconnected(1)", res},
    };
    for (auto&& cmd : commands)
    {
        callbacks.setResult(cmd.second);
        item.m_message = cmd.first;
        ASSERT_TRUE(cmd.second == parser.parseMessage(std::move(item)));
        ASSERT_TRUE(callbacks.isCalled());
        callbacks.reset();
    }
}

TEST_F(MessageParserFixture, ParserGoodResult)
{
    app::MessageParser parser;
    MessageParserCallbacks callbacks;
    parser.registerCallbackObject(callbacks);

    rabbitmq::ProcessingItem item(nullptr, "", "", "", 0, false);

    callbacks.setResult(Result::SUCCESS);
    item.m_message = "user_registered(1,Arr)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_TRUE(callbacks.id() == 1);
    ASSERT_TRUE(callbacks.name() == "Arr");
    callbacks.reset();
    item.m_message = "user_renamed(1,Brr)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_EQ(callbacks.id(), 1);
    ASSERT_EQ(callbacks.name(), "Brr");
    callbacks.reset();
    item.m_message = "user_deal(1,2017-05-20T10:10:10,100)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_EQ(callbacks.id(), 1);
    time_t t;
    ASSERT_EQ(Result::SUCCESS, common::timeFromString(t, "2017-05-20T10:10:10"));
    ASSERT_EQ(callbacks.time(), t);
    ASSERT_EQ(callbacks.amount(), 100);
    callbacks.reset();
    item.m_message = "user_deal_won(1,2017-05-20T10:10:10,100)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_EQ(callbacks.id(), 1);
    ASSERT_EQ(Result::SUCCESS, common::timeFromString(t, "2017-05-20T10:10:10"));
    ASSERT_EQ(callbacks.time(), t);
    ASSERT_EQ(callbacks.amount(), 100);
    callbacks.reset();
    item.m_message = "user_connected(1)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_EQ(callbacks.id(), 1);
    callbacks.reset();
    item.m_message = "user_disconnected(1)";
    ASSERT_EQ(Result::SUCCESS, parser.parseMessage(std::move(item)));
    ASSERT_TRUE(callbacks.isCalled());
    ASSERT_EQ(callbacks.id(), 1);
    callbacks.reset();
}