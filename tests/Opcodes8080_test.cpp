#include <gtest/gtest.h>
#include <vector>
#include <optional>
#include <string>
#include <sstream>

extern "C"
{
#include "Opcodes8080.h"
}

#define IMPLEMENT_ENUM_PRE_INCREMENT(ENUM_NAME)             \
inline ENUM_NAME& operator++(ENUM_NAME& e) {                \
    e = static_cast<ENUM_NAME>(static_cast<int>(e) + 1);    \
    return e;                                               \
}

IMPLEMENT_ENUM_PRE_INCREMENT(GENERAL_REGISTER)

struct TestCaseBitwiseOperation {
    uint8_t a_value;
    uint8_t source_value;
    uint8_t expected;

    bool zero;
    bool sign;
    bool parity;

    std::optional<GENERAL_REGISTER> src;
    uint8_t opcode; // needed for immediate mode
    handle_opcode opcode_handler;
    std::string description;
};

class Opcodes8080Tests : public ::testing::Test {
protected:
    Cpu8080* cpu;
    State8080* state;
    const std::vector<std::string> reg_names = { "B", "C", "D", "E", "H", "L", "HL", "A" };

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

    void test_bitwise(const TestCaseBitwiseOperation& test_case) {
        state->general_register[A] = test_case.a_value;
        state->cc[CARRY] = true;

        uint8_t opcode[2];
        uint16_t start_pc = state->pc;
        if (test_case.src.has_value()) {
            // Register-based
            state->general_register[test_case.src.value()] = test_case.source_value;
            opcode[0] = test_case.opcode;  // e.g., ANA + src
        }
        else {
            // Immediate-based
            opcode[0] = test_case.opcode;
            opcode[1] = test_case.source_value;
        }
        test_case.opcode_handler(cpu, state, opcode);

        EXPECT_EQ(test_case.expected, state->general_register[A]) << test_case.description << ": result mismatch";
        EXPECT_FALSE(state->cc[CARRY]) << test_case.description << ": carry not cleared";
        EXPECT_EQ(test_case.zero, state->cc[ZERO]) << test_case.description << ": ZERO incorrect";
        EXPECT_EQ(test_case.sign, state->cc[SIGN]) << test_case.description << ": SIGN incorrect";
        EXPECT_EQ(test_case.parity, state->cc[PARITY]) << test_case.description << ": PARITY incorrect";

        if (!test_case.src.has_value()) {
            EXPECT_EQ(state->pc, (uint16_t)(start_pc + 1)) << test_case.description << ": PC not incremented";
        }
    }
};

TEST_F(Opcodes8080Tests, MOV_ShouldCopyTargetToSource) {
    for (GENERAL_REGISTER target = B; target < NUMBER_OF_OPCODE_REGISTERS; ++target) {
        if (target == HL_GENERAL_REGISTER) continue;
        for (GENERAL_REGISTER source = B; source < NUMBER_OF_OPCODE_REGISTERS; ++source) {
            if (source == HL_GENERAL_REGISTER) continue;
            uint8_t expected = (uint8_t)(source * 16 + target + 1);
            state->general_register[source] = expected;

            uint8_t opcode = MOV + target * NUMBER_OF_CONDITIONS + source;
            handle_mov(cpu, state, &opcode);

            EXPECT_EQ(expected, state->general_register[target]) 
                << "MOV " << reg_names[target] << "," << reg_names[source] 
                << " failed. Opcode=0x" << std::hex << (int)opcode 
                << ", Expected=0x" << (int)expected 
                << ", Actual=0x" << (int)state->general_register[target];
        }
    }
}

TEST_F(Opcodes8080Tests, MOV_ShouldCopySourceToHL) {
    for (GENERAL_REGISTER source = B; source < NUMBER_OF_OPCODE_REGISTERS; ++source) {
        if (source == HL_GENERAL_REGISTER) continue;
        uint8_t expected = (uint8_t)(source * 16 + 5);
        state->general_register[source] = expected;
        uint8_t opcode = MOV + HL_GENERAL_REGISTER * NUMBER_OF_CONDITIONS + source;
        handle_mov(cpu, state, &opcode);

        uint8_t value_at_hl = read_from_register_pair_address(state, HL);
        EXPECT_EQ(expected, value_at_hl) 
            << "MOV " << reg_names[HL_GENERAL_REGISTER] << "," << reg_names[source] 
            << " failed. Opcode=0x" << std::hex << (int)opcode 
            << ", Expected=0x" << (int)expected 
            << ", Actual=0x" << (int)value_at_hl;
    }
}

TEST_F(Opcodes8080Tests, MOV_ShouldCopyHLToTarget) {
    for (GENERAL_REGISTER target = B; target < NUMBER_OF_OPCODE_REGISTERS; ++target) {
        if (target == HL_GENERAL_REGISTER) continue;
        uint8_t expected = (uint8_t)(target * 16 + 5);
        set_register_pair(state, HL, 0x0000); 
        write_to_memory(state, 0x0000, expected); 
        uint8_t opcode = MOV + target * NUMBER_OF_CONDITIONS + HL_GENERAL_REGISTER;
        handle_mov(cpu, state, &opcode);

        EXPECT_EQ(expected, state->general_register[target]) 
            << "MOV " << reg_names[target] << "," << reg_names[HL_GENERAL_REGISTER] 
            << " failed. Opcode=0x" << std::hex << (int)opcode 
            << ", Expected=0x" << (int)expected 
            << ", Actual=0x" << (int)state->general_register[target];
    }
}

TEST_F(Opcodes8080Tests, ADD_WithoutCarry) {
    struct TestCase {
        uint8_t a_value;
        GENERAL_REGISTER src_reg;
        uint8_t src_value;
        uint8_t expected;
        bool carry;
        bool zero;
        bool sign;
        bool parity;
    };

    std::vector<TestCase> cases = {
        { 0x10, C, 0x20, 0x30, false, false, false, true },
        { 0xFF, A, 0xFF, 0xfe, true,  false,  true, false},
        { 0xF0, D, 0x30, 0x20, true,  false, false, false},
        { 0x00, E, 0x00, 0x00, false, true,  false, true},
        { 0x7F, B, 0x01, 0x80, false, false, true,  false},
        { 0x8F, L, 0x01, 0x90, false, false, true,  true},
        { 0x0F, H, 0x01, 0x10, false, false, false, false},
    };

    for (const auto& test : cases) {
        state->general_register[A] = test.a_value;
        state->general_register[test.src_reg] = test.src_value;

        uint8_t opcode = 0x80 + test.src_reg;
        handle_ADD(cpu, state, &opcode);

        std::stringstream msg;
        msg << "ADD A, " << reg_names[test.src_reg]
            << " | A = 0x" << std::hex << (int)test.a_value
            << ", " << reg_names[test.src_reg] << " = 0x" << (int)test.src_value
            << " -> A = 0x" << (int)state->general_register[A];

        EXPECT_EQ(test.expected, state->general_register[A]) << msg.str();
        EXPECT_EQ(test.carry, state->cc[CARRY]) << msg.str();
        EXPECT_EQ(test.zero, state->cc[ZERO]) << msg.str();
        EXPECT_EQ(test.sign, state->cc[SIGN]) << msg.str();
        EXPECT_EQ(test.parity, state->cc[PARITY]) << msg.str();
    }
}

TEST_F(Opcodes8080Tests, ADD_IMMEDIATE) {
    state->general_register[A] = 0x01;
    state->pc = 0;
    uint8_t opcode[2] = { ADD_IMMEDIATE, 0xfe };
    handle_ADD(cpu, state, opcode);

    EXPECT_EQ((uint8_t)0xff, state->general_register[A]);        
    EXPECT_EQ((uint16_t)0x1, state->pc);
}

TEST_F(Opcodes8080Tests, ADD_WithCarry) {
    state->general_register[A] = 0x01;
    state->general_register[B] = 0xfe;
    state->cc[CARRY] = false; 
    uint8_t opcode = ADC + B; 
    handle_ADC(cpu, state, &opcode);

    EXPECT_EQ((uint8_t)0xff, state->general_register[A]) << "ADC B should add B to A without carry";

    state->general_register[A] = 0x01;
    state->cc[CARRY] = true; 
    handle_ADC(cpu, state, &opcode);
    EXPECT_EQ((uint8_t)0x00, state->general_register[A]) << "ADC B should add B to A with carry";
}

TEST_F(Opcodes8080Tests, SUB_IMMEDIATE) {
    state->general_register[A] = 0x02;
    state->pc = 0;
    uint8_t opcode[2] = { SUB_IMMEDIATE, 0x01 };
    handle_SUB(cpu, state, opcode);

    EXPECT_EQ((uint8_t)0x01, state->general_register[A]);
    EXPECT_EQ((uint16_t)0x1, state->pc);
}

TEST_F(Opcodes8080Tests, SUB_WithoutCarry) {
    state->general_register[A] = 0x02;
    state->general_register[C] = 0x01;
    uint8_t opcode = SUB + C;
    handle_SUB(cpu, state, &opcode);

    EXPECT_EQ((uint8_t)0x01, state->general_register[A]);

    state->general_register[A] = 0x00;
    handle_SUB(cpu, state, &opcode);
    EXPECT_EQ((uint8_t)0xff, state->general_register[A]);
}

TEST_F(Opcodes8080Tests, SUB_WithCarry) {
    state->general_register[A] = 0x02;
    state->general_register[C] = 0x01;
    state->cc[CARRY] = false;
    uint8_t opcode = SBB + C;
    handle_SBB(cpu, state, &opcode);

    EXPECT_EQ((uint8_t)0x01, state->general_register[A]);

    state->general_register[A] = 0x01;
    state->cc[CARRY] = true;
    handle_SBB(cpu, state, &opcode);
    EXPECT_EQ((uint8_t)0xff, state->general_register[A]);
}

TEST_F(Opcodes8080Tests, ANA_TEST) {
    // a_value, source_value, expected, zero, sign, parity, src, opcode, handler, description
    TestCaseBitwiseOperation test1 = {0b11110000, 0b00001111, 0x0, true, false, true, {D}, (uint8_t)(ANA + D), handle_ANA, "ANA D"};
    TestCaseBitwiseOperation test2 = {0b11110001, 0b00000001, 0x1, false, false, false, {D}, (uint8_t)(ANA + D), handle_ANA, "ANA D (non-zero)"};
    TestCaseBitwiseOperation test3 = {0b11110000, 0b00001111, 0x0, true, false, true, std::nullopt, ANA_IMMEDIATE, handle_ANA, "ANI"};
    TestCaseBitwiseOperation test4 = {0b11110001, 0b00000001, 0x1, false, false, false, std::nullopt, ANA_IMMEDIATE, handle_ANA, "ANI (non-zero)"};

    test_bitwise(test1);
    test_bitwise(test2);
    test_bitwise(test3);
    test_bitwise(test4);
}

TEST_F(Opcodes8080Tests, XRA_TEST) {
    TestCaseBitwiseOperation test1 = {0b11110000, 0b00001111, 0xff, false, true, true, {D}, (uint8_t)(XRA + D), handle_XRA, "XRA D"};
    TestCaseBitwiseOperation test2 = {0b11110001, 0b00000001, 0xF0, false, true, true, {D}, (uint8_t)(XRA + D), handle_XRA, "XRA D (another)"};
    TestCaseBitwiseOperation test3 = {0b11110000, 0b00001111, 0xff, false, true, true, std::nullopt, XRA_IMMEDIATE, handle_XRA, "XRI"};
    TestCaseBitwiseOperation test4 = {0b11110001, 0b00000001, 0xF0, false, true, true, std::nullopt, XRA_IMMEDIATE, handle_XRA, "XRI (another)"};

    test_bitwise(test1);
    test_bitwise(test2);
    test_bitwise(test3);
    test_bitwise(test4);
}
