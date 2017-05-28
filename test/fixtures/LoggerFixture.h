#include <gtest/gtest.h>
#include <logger/LoggerDefines.h>

class LoggerFixture : public ::testing::Test
{
protected:
protected:
    virtual void SetUp() override
    {
        logger::Logger& l = logger::Logger::instance();
        ASSERT_TRUE(l.configure());
        ASSERT_TRUE(l.start());
    }

    virtual void TearDown() override
    {
        logger::Logger& l = logger::Logger::instance();
        l.stop();
    }
};