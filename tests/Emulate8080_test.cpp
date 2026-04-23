#include <gtest/gtest.h>
#include <bitset>

extern "C"
{
#include "Emulate8080.h"
}

class CPU8080EmulatorTest : public ::testing::Test {
protected:
    Cpu8080* cpu;
    State8080* state;

    void SetUp() override {
        const size_t memSize = 0x10000; // 64KB
        const size_t dummyCodeSize = 2;
        uint8_t dummyCode[dummyCodeSize] = { 0x00, 0x00 }; // NOP

        cpu = init_cpu_state(dummyCodeSize, dummyCode, memSize);
        ASSERT_NE(cpu, nullptr) << "CPU allocation failed";
        ASSERT_NE(cpu->state, nullptr) << "State allocation failed";
        ASSERT_NE(cpu->state->memory, nullptr) << "Memory allocation failed";

        state = cpu->state;
    }

    void TearDown() override {
        free_cpu(cpu);
    }
};

TEST_F(CPU8080EmulatorTest, TestByteToFlagsAl) {
    // Test case: All flags set (0b11010101)
    uint8_t flags = 0b11010101;
    byte_to_flags(state, flags);
    EXPECT_TRUE(state->cc[CARRY]) << "Carry flag should be set";
    EXPECT_TRUE(state->cc[PARITY]) << "Parity flag should be set";
    EXPECT_TRUE(state->cc[AC]) << "Auxiliary Carry flag should be set";
    EXPECT_TRUE(state->cc[ZERO]) << "Zero flag should be set";
    EXPECT_TRUE(state->cc[SIGN]) << "Sign flag should be set";

    // Test case: All flags cleared (0b00000000)
    flags = 0b00000000;
    byte_to_flags(state, flags);
    EXPECT_FALSE(state->cc[CARRY]) << "Carry flag should be cleared";
    EXPECT_FALSE(state->cc[PARITY]) << "Parity flag should be cleared";
    EXPECT_FALSE(state->cc[AC]) << "Auxiliary Carry flag should be cleared";
    EXPECT_FALSE(state->cc[ZERO]) << "Zero flag should be cleared";
    EXPECT_FALSE(state->cc[SIGN]) << "Sign flag should be cleared";
}

TEST_F(CPU8080EmulatorTest, TestFlagsToByte) {
    // Test case: All flags set
    state->cc[CARRY] = true;
    state->cc[PARITY] = true;
    state->cc[AC] = true;
    state->cc[ZERO] = true;
    state->cc[SIGN] = true;
    uint8_t expectedFlags = 0b11010111;
    uint8_t resultFlags = flags_to_byte(state);
    EXPECT_EQ(expectedFlags, resultFlags) << "Flags byte should match expected value when all flags are set";

    // Test case: All flags cleared
    state->cc[CARRY] = false;
    state->cc[PARITY] = false;
    state->cc[AC] = false;
    state->cc[ZERO] = false;
    state->cc[SIGN] = false;
    expectedFlags = 0b00000010;
    resultFlags = flags_to_byte(state);
    EXPECT_EQ(expectedFlags, resultFlags) << "Flags byte should match expected value when all flags are cleared";
}

TEST_F(CPU8080EmulatorTest, IsPair_ValidPairs_ShouldReturnTrue) {
    EXPECT_TRUE(is_pair(BC)) << "BC should be a valid register pair";
    EXPECT_TRUE(is_pair(DE)) << "DE should be a valid register pair";
    EXPECT_TRUE(is_pair(HL)) << "HL should be a valid register pair";
}

TEST_F(CPU8080EmulatorTest, IsPair_InvalidPairs_ShouldReturnFalse) {
    EXPECT_FALSE(is_pair(SP)) << "SP should not be a valid register pair";
    EXPECT_FALSE(is_pair(PSW)) << "PSW should not be a valid register pair";
    EXPECT_FALSE(is_pair(PC)) << "PC should not be a valid register pair";
}

TEST_F(CPU8080EmulatorTest, Parity_Validation) {
    int values[] = { 0b00000000, 0b00000001, 0b00000011, 0b11111111, 0b00001111, 0b00001111 };
    int sizes[] = { 8,          8,          8,          8,          4,          3 };
    bool expected[] = { true,       false,      true,       true,       true,       false };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        bool result = parity(values[i], sizes[i]);
        EXPECT_EQ(expected[i], result) << "parity(" << std::bitset<8>(values[i]) << ", " << sizes[i] << ") should be " << (expected[i] ? "true" : "false");
    }
}

TEST_F(CPU8080EmulatorTest, Push_ShouldWriteHighAndLowBytesAndUpdateSP) {
    uint16_t original_sp = state->sp;
    uint8_t low = 0x34;
    uint8_t high = 0x12;

    push(state, low, high);

    // After push, SP should decrease by 2
    EXPECT_EQ((uint16_t)(original_sp - 2), state->sp) << "SP should be decremented by 2";

    // Check memory content at new SP
    EXPECT_EQ(low, state->memory[state->sp]) << "Low byte should be at new SP";
    EXPECT_EQ(high, state->memory[state->sp + 1]) << "High byte should be at SP + 1";
}

TEST_F(CPU8080EmulatorTest, PopByte_ShouldReturnByteAtSPAndIncrementSP) {
    uint16_t initial_sp = state->sp;
    uint8_t test_value = 0xAB;

    // Write test byte at SP
    state->memory[initial_sp] = test_value;

    // Call pop_byte
    uint8_t popped = pop_byte(state);

    // Check returned value
    EXPECT_EQ(test_value, popped) << "pop_byte should return the byte at SP";

    // Check that SP increased by 1
    EXPECT_EQ((uint16_t)(initial_sp + 1), state->sp) << "SP should increment by 1 after pop_byte";
}

TEST_F(CPU8080EmulatorTest, ReadFromMemory_ShouldReturnCorrectBytes) {
    // Write bytes at various memory locations
    state->memory[0x0000] = 0xAA;
    state->memory[0x1234] = 0x55;
    state->memory[0xFFFF] = 0xFF;

    EXPECT_EQ((uint8_t)0xAA, read_from_memory(state, 0x0000)) << "Memory read at 0x0000 should be 0xAA";
    EXPECT_EQ((uint8_t)0x55, read_from_memory(state, 0x1234)) << "Memory read at 0x1234 should be 0x55";
    EXPECT_EQ((uint8_t)0xFF, read_from_memory(state, 0xFFFF)) << "Memory read at 0xFFFF should be 0xFF";
}

TEST_F(CPU8080EmulatorTest, WriteToMemory_ShouldStoreValueAtOffset) {
    uint16_t testOffset = 0x2001;
    uint8_t testValue = 0xAB;

    write_to_memory(state, testOffset, testValue);

    EXPECT_EQ(testValue, state->memory[testOffset]) << "Memory at offset should contain the written value";
}

TEST_F(CPU8080EmulatorTest, BytesToWord_ShouldCombineLowAndHighCorrectly) {
    uint16_t low[] = { 0x34, 0xff, 0x00 };
    uint16_t high[] = { 0x12, 0x00, 0xff };
    uint16_t expected[] = { 0x1234, 0x00ff, 0xff00 };
    for (size_t i = 0; i < sizeof(low) / sizeof(low[0]); ++i) {
        uint16_t result = bytes_to_word((uint8_t)low[i], (uint8_t)high[i]);
        EXPECT_EQ(expected[i], result) << "Should combine low and high bytes into a 16-bit word";
    }
}

TEST_F(CPU8080EmulatorTest, SetRegisterPair_ShouldSetHighAndLowBytes) {
    uint16_t testValue = 0xABCD;

    // Test BC pair
    set_register_pair(state, BC, testValue);
    EXPECT_EQ((uint8_t)0xAB, state->general_register[B]) << "High byte of BC should be set to 0xAB";
    EXPECT_EQ((uint8_t)0xCD, state->general_register[C]) << "Low byte of BC should be set to 0xCD";

    // Test DE pair
    set_register_pair(state, DE, testValue);
    EXPECT_EQ((uint8_t)0xAB, state->general_register[D]) << "High byte of DE should be set to 0xAB";
    EXPECT_EQ((uint8_t)0xCD, state->general_register[E]) << "Low byte of DE should be set to 0xCD";

    // Test HL pair
    set_register_pair(state, HL, testValue);
    EXPECT_EQ((uint8_t)0xAB, state->general_register[H]) << "High byte of HL should be set to 0xAB";
    EXPECT_EQ((uint8_t)0xCD, state->general_register[L]) << "Low byte of HL should be set to 0xCD";
}

TEST_F(CPU8080EmulatorTest, GetRegisterPair_ShouldReturnCombinedWord) {
    // Setup registers for BC pair
    state->general_register[B] = 0x12;
    state->general_register[C] = 0x34;
    uint16_t result = get_register_pair(state, BC);
    EXPECT_EQ((uint16_t)0x1234, result) << "get_register_pair BC should combine B and C correctly";

    // Setup registers for DE pair
    state->general_register[D] = 0xAB;
    state->general_register[E] = 0xCD;
    result = get_register_pair(state, DE);
    EXPECT_EQ((uint16_t)0xABCD, result) << "get_register_pair DE should combine D and E correctly";

    // Setup registers for HL pair
    state->general_register[H] = 0x56;
    state->general_register[L] = 0x78;
    result = get_register_pair(state, HL);
    EXPECT_EQ((uint16_t)0x5678, result) << "get_register_pair HL should combine H and L correctly";
}

TEST_F(CPU8080EmulatorTest, SetFlags_ShouldSetCarryZeroSignParity) {
    // Manually clear carry before test
    state->cc[CARRY] = false;

    // value = 0: ZERO set, SIGN cleared, PARITY even, CARRY cleared
    set_flags(state, 0x00);
    EXPECT_TRUE(state->cc[ZERO]) << "ZERO flag should be set for zero value";
    EXPECT_FALSE(state->cc[SIGN]) << "SIGN flag should be cleared for zero value";
    EXPECT_TRUE(state->cc[PARITY]) << "PARITY flag should be set for even parity";
    EXPECT_FALSE(state->cc[CARRY]) << "CARRY flag should be cleared for zero value";

    // value = 0x80 (10000000b): SIGN set, ZERO cleared, PARITY odd, CARRY cleared
    set_flags(state, 0x80);
    EXPECT_FALSE(state->cc[ZERO]) << "ZERO flag should be cleared for non-zero value";
    EXPECT_TRUE(state->cc[SIGN]) << "SIGN flag should be set for high bit";
    EXPECT_FALSE(state->cc[PARITY]) << "PARITY flag should be cleared for odd parity";
    EXPECT_FALSE(state->cc[CARRY]) << "CARRY flag should be cleared";

    // value = 0xFFF (all bits set): SIGN set, ZERO cleared, PARITY even, CARRY cleared
    set_flags(state, 0xFFF);
    EXPECT_FALSE(state->cc[ZERO]) << "ZERO flag cleared for non-zero";
    EXPECT_TRUE(state->cc[SIGN]) << "SIGN flag set for high bit";
    EXPECT_TRUE(state->cc[PARITY]) << "PARITY flag set for even parity";
    EXPECT_TRUE(state->cc[CARRY]) << "CARRY flag should be set";

    // value = 0x7F (01111111b): SIGN cleared, ZERO cleared, PARITY odd, CARRY cleared
    set_flags(state, 0b100111101);
    EXPECT_FALSE(state->cc[ZERO]) << "ZERO flag cleared";
    EXPECT_FALSE(state->cc[SIGN]) << "SIGN flag cleared";
    EXPECT_FALSE(state->cc[PARITY]) << "PARITY flag cleared for odd parity";
    EXPECT_TRUE(state->cc[CARRY]) << "CARRY flag should be set";
}

TEST_F(CPU8080EmulatorTest, ReadFromRegisterPairAddress_ShouldReturnMemoryByteAtRegisterPairAddress) {
    // Setup HL to point somewhere in memory
    uint16_t address = 0x1234;
    state->general_register[H] = (address >> 8) & 0xFF;
    state->general_register[L] = address & 0xFF;

    // Write a known byte at that address
    uint8_t expectedValue = 0xAB;
    state->memory[address] = expectedValue;

    // Read using the function
    uint8_t result = read_from_register_pair_address(state, HL);

    EXPECT_EQ(expectedValue, result) << "Value read from memory at HL address should match expected";
}

TEST_F(CPU8080EmulatorTest, WriteToRegisterPairAddress_ShouldWriteValueToMemoryAtRegisterPairAddress) {
    // Setup HL register pair with address 0x4567
    uint16_t address = 0x2001;
    state->general_register[H] = (address >> 8) & 0xFF;
    state->general_register[L] = address & 0xFF;

    uint8_t testValue = 0xCD;

    // Write the value at the address pointed by HL
    write_to_register_pair_address(state, HL, testValue);

    // Verify memory contains the value at that address
    EXPECT_EQ(testValue, state->memory[address]) << "Memory at HL address should contain the written value";
}

TEST_F(CPU8080EmulatorTest, RSTOpcode_ShouldPushPCAndSetPC) {
    // Setup initial PC and SP
    state->pc = 0x2001;
    state->sp = 0x2400;

    // Clear stack memory at sp-1 and sp-2 for verification
    state->memory[state->sp - 1] = 0;
    state->memory[state->sp - 2] = 0;

    // Call op_rst with N=3
    uint8_t N = 3;
    op_rst(state, N);

    // Expected PC pushed bytes
    uint8_t expected_low = 0x01;
    uint8_t expected_high = 0x20;

    // SP should decrease by 2 after push
    EXPECT_EQ((uint16_t)(0x2400 - 2), state->sp) << "Stack pointer should decrease by 2 after push";

    // Stack memory at new SP + 1 contains low byte, at new SP contains high byte
    EXPECT_EQ(expected_low, state->memory[state->sp]) << "Low byte of PC should be pushed onto stack";
    EXPECT_EQ(expected_high, state->memory[state->sp + 1]) << "High byte of PC should be pushed onto stack";

    // PC should be set to N * 8
    EXPECT_EQ((uint16_t)(N * 8), state->pc) << "Program counter should be set to N*8";
}
