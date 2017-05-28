#include <libconfig.h++>
#include <gtest/gtest.h>

#include <common/Types.h>
#include <app/Logic.h>

#include "../fixtures/LoggerFixture.h"

class LogicFixture : public LoggerFixture
{};

using common::Result;

TEST_F(LogicFixture, LogicGoodCycle)
{
    libconfig::Config cfg;
    app::Logic logic;
    ASSERT_EQ(Result::SUCCESS, logic.initialize());
    ASSERT_EQ(Result::SUCCESS, logic.configure(cfg));
    ASSERT_EQ(Result::SUCCESS, logic.start());
    logic.stop();
    logic.deinitialize();
}

TEST_F(LogicFixture, LogicInvalidState)
{
    libconfig::Config cfg;
    app::Logic logic;
    ASSERT_EQ(Result::INVALID_STATE, logic.configure(cfg));
    ASSERT_EQ(Result::INVALID_STATE, logic.start());
    ASSERT_EQ(Result::SUCCESS, logic.initialize());
    ASSERT_EQ(Result::INVALID_STATE, logic.initialize());
    ASSERT_EQ(Result::INVALID_STATE, logic.start());
    ASSERT_EQ(Result::SUCCESS, logic.configure(cfg));
    ASSERT_EQ(Result::INVALID_STATE, logic.initialize());
    ASSERT_EQ(Result::INVALID_STATE, logic.configure(cfg));
    ASSERT_EQ(Result::SUCCESS, logic.start());
    ASSERT_EQ(Result::INVALID_STATE, logic.initialize());
    ASSERT_EQ(Result::INVALID_STATE, logic.configure(cfg));
    ASSERT_EQ(Result::INVALID_STATE, logic.start());

    logic.stop();
    logic.deinitialize();
}
