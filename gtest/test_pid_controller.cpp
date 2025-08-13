#include <gtest/gtest.h>
extern "C" {
    #include "pid_controller.h"
}

class PIDControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(pid_init());
    }

    void TearDown() override {
        pid_reset();
    }
};

TEST_F(PIDControllerTest, InitializationTest) {
    EXPECT_TRUE(pid_init());
    
    EXPECT_FLOAT_EQ(pid_getSetpoint(), PID_DEFAULT_SETPOINT);
    
    EXPECT_FLOAT_EQ(pid_getOutput(), 0.0f);
}

TEST_F(PIDControllerTest, SetpointTest) {
    float testSetpoint = 50.0f;
    
    EXPECT_TRUE(pid_setSetpoint(testSetpoint));
    EXPECT_FLOAT_EQ(pid_getSetpoint(), testSetpoint);
}

TEST_F(PIDControllerTest, GainsTest) {
    float kp = 2.0f, ki = 0.5f, kd = 0.1f;
    
    EXPECT_TRUE(pid_setGains(kp, ki, kd));
    
    float getKp, getKi, getKd;
    EXPECT_TRUE(pid_getGains(&getKp, &getKi, &getKd));
    EXPECT_FLOAT_EQ(getKp, kp);
    EXPECT_FLOAT_EQ(getKi, ki);
    EXPECT_FLOAT_EQ(getKd, kd);
    
    pid_setSetpoint(100.0f);
    float output1 = pid_compute(90.0f);
    
    pid_reset();
    EXPECT_TRUE(pid_setGains(kp * 2, ki, kd));
    float output2 = pid_compute(90.0f);
    
    EXPECT_NE(output1, output2);
}

TEST_F(PIDControllerTest, OutputLimitsTest) {
    float outputMin = -50.0f;
    float outputMax = 50.0f;
    
    EXPECT_TRUE(pid_setOutputLimits(outputMin, outputMax));
    
    EXPECT_FALSE(pid_setOutputLimits(50.0f, 50.0f));
    EXPECT_FALSE(pid_setOutputLimits(60.0f, 50.0f));
    
    pid_setGains(100.0f, 10.0f, 1.0f);
    pid_setSetpoint(100.0f);
    
    float output = pid_compute(0.0f);
    EXPECT_LE(output, outputMax);
    EXPECT_GE(output, outputMin);
}

TEST_F(PIDControllerTest, ProportionalResponseTest) {
    pid_setGains(1.0f, 0.0f, 0.0f);
    pid_setSetpoint(100.0f);
    
    float output1 = pid_compute(90.0f);
    float output2 = pid_compute(80.0f);
    
    EXPECT_FLOAT_EQ(output1, 10.0f);
    EXPECT_FLOAT_EQ(output2, 20.0f);
}

TEST_F(PIDControllerTest, ResetTest) {
    pid_setGains(1.0f, 1.0f, 1.0f);
    pid_setSetpoint(100.0f);
    
    pid_compute(90.0f);
    pid_compute(85.0f);
    
    float outputBeforeReset = pid_getOutput();
    
    pid_reset();
    float outputAfterReset = pid_compute(90.0f);
    
    EXPECT_FLOAT_EQ(pid_getOutput(), outputAfterReset);
}

TEST_F(PIDControllerTest, SteadyStateTest) {
    pid_setGains(1.0f, 0.1f, 0.01f);
    pid_setSetpoint(50.0f);
    
    for (int i = 0; i < 10; i++) {
        pid_compute(50.0f);
    }
    
    float finalOutput = pid_getOutput();
    EXPECT_NEAR(finalOutput, 0.0f, 0.1f);
}

TEST_F(PIDControllerTest, IntegralWindupTest) {
    pid_setOutputLimits(-10.0f, 10.0f);
    pid_setGains(1.0f, 1.0f, 0.0f);
    pid_setSetpoint(100.0f);
    
    for (int i = 0; i < 100; i++) {
        float output = pid_compute(0.0f);
        EXPECT_LE(output, 10.0f);
        EXPECT_GE(output, -10.0f);
    }
    
    float output = pid_compute(200.0f);
    EXPECT_LE(output, 10.0f);
    EXPECT_GE(output, -10.0f);
}

TEST_F(PIDControllerTest, ComputeWithVariousInputsTest) {
    pid_setGains(2.0f, 0.5f, 0.1f);
    pid_setSetpoint(75.0f);
    
    float testInputs[] = {70.0f, 72.5f, 75.0f, 77.5f, 80.0f};
    
    for (float input : testInputs) {
        float output = pid_compute(input);
        EXPECT_GE(output, PID_DEFAULT_OUTPUT_MIN);
        EXPECT_LE(output, PID_DEFAULT_OUTPUT_MAX);
    }
}