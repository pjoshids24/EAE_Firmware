#include <gtest/gtest.h>
extern "C" {
    #include "fan_control.h"
}

class FanControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(fan_init());
    }

    void TearDown() override {
        fan_enable(false);
    }
};

TEST_F(FanControlTest, InitializationTest) {
    EXPECT_TRUE(fan_init());
    
    EXPECT_TRUE(fan_init());
}

TEST_F(FanControlTest, EnableDisableTest) {
    fan_enable(true);
    
    fan_enable(false);
    
    for (int i = 0; i < 5; i++) {
        fan_enable(true);
        fan_enable(false);
    }
}

TEST_F(FanControlTest, UpdateSpeedWhenDisabledTest) {
    fan_enable(false);
    
    fan_updateSpeed(50.0f);
    fan_updateSpeed(100.0f);
    fan_updateSpeed(0.0f);
}

TEST_F(FanControlTest, UpdateSpeedWhenEnabledTest) {
    fan_enable(true);
    
    fan_updateSpeed(0.0f);
    fan_updateSpeed(50.0f);
    fan_updateSpeed(100.0f);
    
    fan_updateSpeed(-10.0f);
    fan_updateSpeed(110.0f);
}

TEST_F(FanControlTest, MaxSpeedTest) {
    EXPECT_TRUE(fan_setMaxSpeed());
    
    fan_enable(false);
    EXPECT_TRUE(fan_setMaxSpeed());
    
    fan_enable(true);
    EXPECT_TRUE(fan_setMaxSpeed());
}

TEST_F(FanControlTest, SpeedRangeTest) {
    fan_enable(true);
    
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
        fan_updateSpeed(value);
    }
}

TEST_F(FanControlTest, EnableAfterMaxSpeedTest) {
    fan_setMaxSpeed();
    fan_enable(true);
    
    fan_updateSpeed(50.0f);
}

TEST_F(FanControlTest, DisableAfterMaxSpeedTest) {
    fan_enable(true);
    fan_setMaxSpeed();
    fan_enable(false);
    
    fan_enable(true);
}

TEST_F(FanControlTest, ConsecutiveSpeedUpdatesTest) {
    fan_enable(true);
    
    for (float speed = 0.0f; speed <= 100.0f; speed += 10.0f) {
        fan_updateSpeed(speed);
    }
    
    for (float speed = 100.0f; speed >= 0.0f; speed -= 10.0f) {
        fan_updateSpeed(speed);
    }
}

TEST_F(FanControlTest, StateTransitionTest) {
    fan_enable(false);
    
    fan_enable(true);
    fan_updateSpeed(50.0f);
    
    fan_setMaxSpeed();
    
    fan_updateSpeed(75.0f);
    
    fan_enable(false);
    
    fan_updateSpeed(25.0f);
    
    fan_enable(true);
}

TEST_F(FanControlTest, EdgeCaseValuesTest) {
    fan_enable(true);
    
    fan_updateSpeed(0.1f);
    fan_updateSpeed(-0.1f);
    fan_updateSpeed(99.9f);
    fan_updateSpeed(100.1f);
}

TEST_F(FanControlTest, RapidEnableDisableTest) {
    for (int i = 0; i < 100; i++) {
        fan_enable(i % 2 == 0);
    }
    
    fan_enable(false);
}

TEST_F(FanControlTest, InitializationAfterOperationTest) {
    fan_enable(true);
    fan_updateSpeed(75.0f);
    fan_setMaxSpeed();
    
    EXPECT_TRUE(fan_init());
    
    fan_enable(true);
    fan_updateSpeed(50.0f);
}

TEST_F(FanControlTest, ComparisonWithPumpControlTest) {
    fan_enable(true);
    
    fan_updateSpeed(0.0f);
    fan_updateSpeed(50.0f);
    fan_updateSpeed(100.0f);
    fan_setMaxSpeed();
    fan_enable(false);
    fan_enable(true);
    fan_updateSpeed(25.0f);
}

TEST_F(FanControlTest, CascadeControlSimulationTest) {
    fan_enable(true);
    
    float pumpOutput = 30.0f;
    float fanSpeed = (pumpOutput > 40.0f) ? (pumpOutput - 40.0f) : 0.0f;
    fan_updateSpeed(fanSpeed);
    
    pumpOutput = 70.0f;
    fanSpeed = (pumpOutput > 40.0f) ? (pumpOutput - 40.0f) : 0.0f;
    fan_updateSpeed(fanSpeed);
}

TEST_F(FanControlTest, StatusTest) {
    fanStatus_t status = fan_getStatus();
    
    EXPECT_TRUE(status.fanState == FAN_OFF ||
                status.fanState == FAN_SPEED_CONTROL ||
                status.fanState == FAN_MAX ||
                status.fanState == FAN_FAULT);
    
    EXPECT_GE(status.pwmDutyCycle, 0.0f);
    EXPECT_LE(status.pwmDutyCycle, 100.0f);
}