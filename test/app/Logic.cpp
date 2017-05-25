#include <libconfig.h++>
#include <gtest/gtest.h>

#include <logger/LoggerDefines.h>
#include <common/Types.h>
#include <app/Logic.h>

using common::Result;

class LoggerFixture : public ::testing::Test
{
protected:
protected:
    virtual void SetUp() override
    {
        logger::Logger& l = logger::Logger::instance();
        ASSERT_TRUE(l.configure());
        ASSERT_TRUE(l.initialize());
    }

    virtual void TearDown() override
    {
        logger::Logger& l = logger::Logger::instance();
        l.deinitialize();
    }
};

TEST_F(LoggerFixture, LogicStart)
{
    libconfig::Config cfg;
    app::Logic logic;
    ASSERT_TRUE(Result::SUCCESS == logic.initialize());
    ASSERT_TRUE(Result::SUCCESS == logic.configure(cfg));
    ASSERT_TRUE(Result::SUCCESS == logic.start());
    logic.stop();
    logic.deinitialize();
}
