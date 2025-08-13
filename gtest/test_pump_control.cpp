#include <gtest/gtest.h>
extern "C" {
    #include "pump_control.h"
}

class PumpControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(pump_init());
    }

    void TearDown() override {
        pump_enable(false);
    }
};

TEST_F(PumpControlTest, InitializationTest) {
    EXPECT_TRUE(pump_init());
    
    EXPECT_TRUE(pump_init());
}

TEST_F(PumpControlTest, EnableDisableTest) {
    pump_enable(true);
    
    pump_enable(false);
    
    for (int i = 0; i < 5; i++) {
        pump_enable(true);
        pump_enable(false);
    }
}

TEST_F(PumpControlTest, UpdateSpeedWhenDisabledTest) {
    pump_enable(false);
    
    pump_updateSpeed(50.0f);
    pump_updateSpeed(100.0f);
    pump_updateSpeed(0.0f);
}

TEST_F(PumpControlTest, UpdateSpeedWhenEnabledTest) {
    pump_enable(true);
    
    pump_updateSpeed(0.0f);
    pump_updateSpeed(50.0f);
    pump_updateSpeed(100.0f);
    
    pump_updateSpeed(-10.0f);
    pump_updateSpeed(110.0f);
}

TEST_F(PumpControlTest, MaxSpeedTest) {
    EXPECT_TRUE(pump_setMaxSpeed());
    
    pump_enable(false);
    EXPECT_TRUE(pump_setMaxSpeed());
    
    pump_enable(true);
    EXPECT_TRUE(pump_setMaxSpeed());
}

TEST_F(PumpControlTest, SpeedRangeTest) {
    pump_enable(true);
    
    float testValues[] = {
        -50.0f,
        0.0f,
        25.0f,
        50.0f,
        75.0f,
        100.0f,
        150.0f
    };
    
    for (float value : testValues) {
        pump_updateSpeed(value);
    }
}

TEST_F(PumpControlTest, EnableAfterMaxSpeedTest) {
    pump_setMaxSpeed();
    pump_enable(true);
    
    pump_updateSpeed(50.0f);
}

TEST_F(PumpControlTest, DisableAfterMaxSpeedTest) {
    pump_enable(true);
    pump_setMaxSpeed();
    pump_enable(false);
    
    pump_enable(true);
}

TEST_F(PumpControlTest, ConsecutiveSpeedUpdatesTest) {
    pump_enable(true);
    
    for (float speed = 0.0f; speed <= 100.0f; speed += 10.0f) {
        pump_updateSpeed(speed);
    }
    
    for (float speed = 100.0f; speed >= 0.0f; speed -= 10.0f) {
        pump_updateSpeed(speed);
    }
}

TEST_F(PumpControlTest, StateTransitionTest) {
    pump_enable(false);
    
    pump_enable(true);
    pump_updateSpeed(50.0f);
    
    pump_setMaxSpeed();
    
    pump_updateSpeed(75.0f);
    
    pump_enable(false);
    
    pump_updateSpeed(25.0f);
    
    pump_enable(true);
}

TEST_F(PumpControlTest, EdgeCaseValuesTest) {
    pump_enable(true);
    
    pump_updateSpeed(0.1f);
    pump_updateSpeed(-0.1f);
    pump_updateSpeed(99.9f);
    pump_updateSpeed(100.1f);
}

TEST_F(PumpControlTest, RapidEnableDisableTest) {
    for (int i = 0; i < 100; i++) {
        pump_enable(i % 2 == 0);
    }
    
    pump_enable(false);
}

TEST_F(PumpControlTest, InitializationAfterOperationTest) {
    pump_enable(true);
    pump_updateSpeed(75.0f);
    pump_setMaxSpeed();
    
    EXPECT_TRUE(pump_init());
    
    pump_enable(true);
    pump_updateSpeed(50.0f);
}

TEST_F(PumpControlTest, StatusTest) {
    pumpStatus_t status = pump_getStatus();
    
    EXPECT_TRUE(status.pumpState == PUMP_OFF ||
                status.pumpState == PUMP_SPEED_CONTROL ||
                status.pumpState == PUMP_MAX ||
                status.pumpState == PUMP_FAULT);
    
    EXPECT_GE(status.pwmDutyCycle, 0.0f);
    EXPECT_LE(status.pwmDutyCycle, 100.0f);
}

TEST_F(PumpControlTest, StatusConsistencyTest) {
    pump_enable(true);
    pump_updateSpeed(50.0f);
    
    pumpStatus_t status = pump_getStatus();
    EXPECT_TRUE(status.isEnabled);
    EXPECT_EQ(status.pumpState, PUMP_SPEED_CONTROL);
    
    pump_enable(false);
    status = pump_getStatus();
    EXPECT_FALSE(status.isEnabled);
    EXPECT_EQ(status.pumpState, PUMP_OFF);
}