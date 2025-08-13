#include <gtest/gtest.h>
extern "C" {
    #include "can_manager.h"
}

class CANManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        canConfig_t config;
        config.baudrate = 250000;
        
        ASSERT_TRUE(canManager_init(&config));
    }
};

TEST_F(CANManagerTest, InitializationTest) {
    canConfig_t config;
    config.baudrate = 500000;

    EXPECT_TRUE(canManager_init(&config));
    EXPECT_TRUE(canManager_init(nullptr));
}

TEST_F(CANManagerTest, SendFrameTest) {
    canFrame_t frame;
    frame.id = 0x123;
    frame.dlc = 8;
    frame.extended = false;
    frame.remote = false;
    for (int i = 0; i < 8; i++) {
        frame.data[i] = i;
    }
    
    EXPECT_EQ(canManager_sendFrame(&frame), CAN_STATUS_OK);
    
    EXPECT_EQ(canManager_sendFrame(nullptr), CAN_STATUS_ERROR);
    
    frame.dlc = 9;
    EXPECT_EQ(canManager_sendFrame(&frame), CAN_STATUS_ERROR);
}

TEST_F(CANManagerTest, ReceiveFrameTest) {
    canFrame_t txFrame, rxFrame;
    
    txFrame.id = 0x456;
    txFrame.dlc = 4;
    txFrame.extended = false;
    txFrame.remote = false;
    txFrame.data[0] = 0xAB;
    txFrame.data[1] = 0xCD;
    txFrame.data[2] = 0xEF;
    txFrame.data[3] = 0x12;
    
    EXPECT_EQ(canManager_sendFrame(&txFrame), CAN_STATUS_OK);
    
    EXPECT_EQ(canManager_receiveFrame(&rxFrame), CAN_STATUS_TIMEOUT);
    
    EXPECT_EQ(canManager_receiveFrame(nullptr), CAN_STATUS_ERROR);
}

TEST_F(CANManagerTest, FloatConversionTest) {
    float testValues[] = {0.0f, 1.234f, -5.678f, 100.5f, -0.001f};
    
    for (float originalValue : testValues) {
        uint8_t data[4];
        
        canManager_floatToBytes(originalValue, data);
        
        float convertedValue = canManager_bytesToFloat(data);
        
        EXPECT_FLOAT_EQ(originalValue, convertedValue);
    }
}

TEST_F(CANManagerTest, TempStatusMessageTest) {
    float temperature = 85.5f;
    uint8_t status = 2;

    EXPECT_EQ(canManager_sendTempStatus(temperature, status), CAN_STATUS_OK);
}

TEST_F(CANManagerTest, PumpStatusMessageTest) {
    float dutyCycle = 75.5f;
    uint8_t state = 1;
    bool enabled = true;
    
    EXPECT_EQ(canManager_sendPumpStatus(dutyCycle, state, enabled), CAN_STATUS_OK);
}

TEST_F(CANManagerTest, SystemStatusMessageTest) {
    uint8_t systemState = 3;
    uint8_t ignitionState = 1;
    uint8_t coolantLevel = 1;
    uint8_t faultCode = 0;
    
    EXPECT_EQ(canManager_sendSystemStatus(systemState, (ignitionState_t)ignitionState, (levelState_t)coolantLevel, faultCode), 
              CAN_STATUS_OK);
}

TEST_F(CANManagerTest, PIDParamsMessageTest) {
    float kp = 1.234f;
    float ki = 0.567f;
    float kd = 0.089f;
    float setpoint = 45.6f;
    
    EXPECT_EQ(canManager_sendPidParams(kp, ki, kd, setpoint), CAN_STATUS_OK);
}

TEST_F(CANManagerTest, StatisticsTest) {
    const canStats_t* stats = canManager_getStats();
    EXPECT_NE(stats, nullptr);
    
    uint32_t initialTxCount = stats->txCount;
    
    canFrame_t frame;
    frame.id = 0x789;
    frame.dlc = 2;
    frame.extended = false;
    frame.remote = false;
    frame.data[0] = 0x55;
    frame.data[1] = 0xAA;
    
    canManager_sendFrame(&frame);
    
    EXPECT_EQ(stats->txCount, initialTxCount + 1);
}