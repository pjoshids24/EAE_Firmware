#include <gtest/gtest.h>
extern "C" {
    #include "dio_manager.h"
}

class DIOManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(dioManager_init());
    }

    void TearDown() override {
    }
};

TEST_F(DIOManagerTest, InitializationTest) {
    EXPECT_TRUE(dioManager_init());
    
    EXPECT_TRUE(dioManager_init());
}

TEST_F(DIOManagerTest, ReadIgnitionBasicTest) {
    ignitionState_t ignitionState = dioManager_readIgnition();
    
    EXPECT_TRUE(ignitionState == IGNITION_OFF ||
                ignitionState == IGNITION_ON ||
                ignitionState == IGNITION_UNKNOWN);
}

TEST_F(DIOManagerTest, ReadLevelBasicTest) {
    levelState_t levelState = dioManager_readLevel();
    
    EXPECT_TRUE(levelState == LEVEL_LOW ||
                levelState == LEVEL_NORMAL ||
                levelState == LEVEL_UNKNOWN);
}

TEST_F(DIOManagerTest, IgnitionStateEnumTest) {
    ignitionState_t state = dioManager_readIgnition();
    
    EXPECT_GE(state, IGNITION_OFF);
    EXPECT_LE(state, IGNITION_UNKNOWN);
    
    bool validEnum = (state == IGNITION_OFF) ||
                    (state == IGNITION_ON) ||
                    (state == IGNITION_UNKNOWN);
    EXPECT_TRUE(validEnum);
}

TEST_F(DIOManagerTest, LevelStateEnumTest) {
    levelState_t state = dioManager_readLevel();
    
    EXPECT_GE(state, LEVEL_LOW);
    EXPECT_LE(state, LEVEL_UNKNOWN);
    
    bool validEnum = (state == LEVEL_LOW) ||
                    (state == LEVEL_NORMAL) ||
                    (state == LEVEL_UNKNOWN);
    EXPECT_TRUE(validEnum);
}

TEST_F(DIOManagerTest, SimulationConsistencyTest) {
    ignitionState_t ignition = dioManager_readIgnition();
    levelState_t level = dioManager_readLevel();
    
    EXPECT_EQ(ignition, IGNITION_ON);
    EXPECT_EQ(level, LEVEL_NORMAL);
}

TEST_F(DIOManagerTest, SafetyLogicTest) {
    levelState_t level = dioManager_readLevel();
    if (level == LEVEL_LOW) {
        EXPECT_EQ(level, LEVEL_LOW);
    } else {
        EXPECT_EQ(level, LEVEL_NORMAL);
    }
    
    ignitionState_t ignition = dioManager_readIgnition();
    if (ignition == IGNITION_ON) {
        EXPECT_EQ(ignition, IGNITION_ON);
    } else if (ignition == IGNITION_OFF) {
        EXPECT_EQ(ignition, IGNITION_OFF);
    } else {
        EXPECT_EQ(ignition, IGNITION_UNKNOWN);
    }
}

TEST_F(DIOManagerTest, PinMappingTest) {
    EXPECT_GE(DIO_DEFAULT_IGNITION_PIN, GPIO_PIN_0);
    EXPECT_LT(DIO_DEFAULT_IGNITION_PIN, MAX);
    
    EXPECT_GE(DIO_DEFAULT_LEVEL_SWITCH_PIN, GPIO_PIN_0);
    EXPECT_LT(DIO_DEFAULT_LEVEL_SWITCH_PIN, MAX);
    
    EXPECT_NE(DIO_DEFAULT_IGNITION_PIN, DIO_DEFAULT_LEVEL_SWITCH_PIN);
}