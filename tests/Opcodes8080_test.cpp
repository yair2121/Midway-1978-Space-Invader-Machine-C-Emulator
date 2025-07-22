#include "pch.h"
#include "CppUnitTest.h"
#include <vector>
#include <optional>
#include <codecvt>
#include <locale>

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
//IMPLEMENT_ENUM_PLUS_PLUS(GENERAL_REGISTER);
//IMPLEMENT_ENUM_PLUS_PLUS(SPECIAL_REGISTER);
//IMPLEMENT_ENUM_PLUS_PLUS(CONDITION_CODE);
//IMPLEMENT_ENUM_PLUS_PLUS(OPCODE);
//struct TestCaseBitWiseOperation {
//    GENERAL_REGISTER src;
//    OPCODE opcode;
//    handle_opcode opcode_handler;
//    uint8_t a_value;
//    uint8_t source_value;
//    uint8_t expected;
//    bool carry;
//    bool zero;
//    bool sign;
//    bool parity;
//};
std::wstring ToWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}


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

    //TestCaseBitwiseOperation(
    //    uint8_t a, uint8_t src, uint8_t exp,
    //    bool z, bool s, bool p,
    //    uint8_t op, void (*handler)(Cpu8080*, State8080*, uint8_t*)
    //) :
    //    a_value(a), source_value(src), expected(exp),
    //    zero(z), sign(s), parity(p),
    //    opcode(op), opcode_handler(handler) {
    //}

};



using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace Opcodes8080Tests
{
    //const std::vector<std::string> Opcodes8080Tests::reg_names = { "B", "C", "D", "E", "H", "L", "HL", "A" };
    TEST_CLASS(Opcodes8080Tests)
    {
    private:
        //void test_bitwise(TestCaseBitWiseOperation test_case) {
        //    state->general_register[A] = test_case.a_value;
        //    state->general_register[test_case.src]= test_case.expected;
        //    uint16_t start_pc = state->pc;
        //    state->cc[CARRY] = true;
        //    test_case.opcode_handler(cpu, state, opcode);

        //    Assert::AreEqual(test_case.expected, state->general_register[A], L"Failed to set correct result in Register A", LINE_INFO());
        //    Assert::AreEqual(state->pc, (uint16_t)(start_pc + 1), L"PC was not incremented", LINE_INFO());
        //    Assert::AreEqual(state->cc[CARRY], false, L"CARRY was not reset", LINE_INFO());
        //    Assert::AreEqual(test_case.zero, state->cc[ZERO], L"ZERO flag was not set correctly", LINE_INFO());
        //    Assert::AreEqual(test_case.sign, state->cc[SIGN], L"SIGN flag was not set correctly", LINE_INFO());
        //    Assert::AreEqual(test_case.parity, state->cc[PARITY], L"PARITY flag was not set correctly", LINE_INFO());
        //}
        void test_bitwise(const TestCaseBitwiseOperation& test_case) {
            state->general_register[A] = test_case.a_value;
            state->cc[CARRY] = true;

            uint8_t opcode[2];
            uint16_t start_pc;
            if (test_case.src.has_value()) {
                // Register-based
                state->general_register[test_case.src.value()] = test_case.source_value;
                opcode[0] = test_case.opcode;  // e.g., ANA + src
            }
            else {
                start_pc = state->pc;

                // Immediate-based
                opcode[0] = test_case.opcode;
                opcode[1] = test_case.source_value;
            }
            test_case.opcode_handler(cpu, state, opcode);

            Assert::AreEqual(test_case.expected, state->general_register[A],  ToWide(test_case.description + ": result mismatch").c_str(), LINE_INFO());
            Assert::IsFalse(state->cc[CARRY], ToWide(test_case.description + ": carry not cleared").c_str(), LINE_INFO());
            Assert::AreEqual(test_case.zero, state->cc[ZERO], ToWide(test_case.description + ": ZERO incorrect").c_str(), LINE_INFO());
            Assert::AreEqual(test_case.sign, state->cc[SIGN], ToWide(test_case.description + ": SIGN incorrect").c_str(), LINE_INFO());
            Assert::AreEqual(test_case.parity, state->cc[PARITY], ToWide(test_case.description + ": PARITY incorrect").c_str(), LINE_INFO());

            if (test_case.src.has_value() == false) {
                Assert::AreEqual(state->pc, (uint16_t)(start_pc + 1), ToWide(test_case.description + ": PC not incremented").c_str(), LINE_INFO());
            }
        }

   //     void test_bitwise_immediate(TestCaseBitWiseOperation test_case) {
   //         state->general_register[A] = test_case.a_value;
   //         uint16_t start_pc = state->pc;
   //         uint8_t opcode[] = { test_case.opcode,  test_case.source_value };
   //         state->cc[CARRY] = true;
			//test_case.opcode_handler(cpu, state, opcode);

   //         Assert::AreEqual(test_case.expected, state->general_register[A], L"Failed to set correct result in Register A", LINE_INFO());
   //         Assert::AreEqual(state->pc, (uint16_t) (start_pc + 1), L"PC was not incremented", LINE_INFO());
   //         Assert::AreEqual(state->cc[CARRY], false, L"CARRY was not reset", LINE_INFO());
   //         Assert::AreEqual(test_case.zero, state->cc[ZERO], L"ZERO flag was not set correctly", LINE_INFO());
   //         Assert::AreEqual(test_case.sign, state->cc[SIGN], L"SIGN flag was not set correctly", LINE_INFO());
   //         Assert::AreEqual(test_case.parity, state->cc[PARITY], L"PARITY flag was not set correctly", LINE_INFO());
   //     }
    public:

        Cpu8080* cpu;
        State8080* state;
        const std::vector<std::wstring> reg_names = { L"B", L"C", L"D", L"E", L"H", L"L", L"HL", L"A" };
        TEST_METHOD_INITIALIZE(Init)
        {
            const size_t memSize = 0x10000; // 64KB
            const size_t dummyCodeSize = 2;
            uint8_t dummyCode[dummyCodeSize] = { 0x00, 0x00 }; // NOP

            cpu = init_cpu_state(dummyCodeSize, dummyCode, memSize);
            Assert::IsNotNull(cpu, L"CPU allocation failed");
            Assert::IsNotNull(cpu->state, L"State allocation failed");
            Assert::IsNotNull(cpu->state->memory, L"Memory allocation failed");

            state = cpu->state;

        }

        TEST_METHOD_CLEANUP(Cleanup)
        {
            free_cpu(cpu);
        }


        TEST_METHOD(MOV_ShouldCopyTargetToSource)
        {	
            for (GENERAL_REGISTER target = B; target < NUMBER_OF_OPCODE_REGISTERS; ++target) {
                if (target == HL_GENERAL_REGISTER) continue; // Handle in a different TEST
                for (GENERAL_REGISTER source = B; source < NUMBER_OF_OPCODE_REGISTERS; ++source) {
                    if (source == HL_GENERAL_REGISTER) continue; // Handle in a different TEST
                        uint8_t expected = (uint8_t)(source * 16 + target + 1); // Just a unique test value
                        state->general_register[source] = expected;

                        uint8_t opcode = MOV + target * NUMBER_OF_CONDITIONS + source;
                        handle_mov(cpu, state, &opcode);
                        wchar_t message[256];
                        swprintf_s(message, L"MOV %s,%s failed. Opcode=0x%02X, Expected=0x%02X, Actual=0x%02X",
                            reg_names[target], reg_names[source], opcode, expected, state->general_register[target]);

                        Assert::AreEqual(expected, state->general_register[target], message, LINE_INFO());

                }
            }
        }
        TEST_METHOD(MOV_ShouldCopySourceToHL)
        {
            for (GENERAL_REGISTER source = B; source < NUMBER_OF_OPCODE_REGISTERS; ++source) {
                if (source == HL_GENERAL_REGISTER) continue;
                uint8_t expected = (uint8_t)(source * 16 + 5);
				state->general_register[source] = expected;
                uint8_t opcode = MOV + HL_GENERAL_REGISTER * NUMBER_OF_CONDITIONS + source;
                handle_mov(cpu, state, &opcode);

                wchar_t message[256];
                uint8_t value_at_hl = read_from_register_pair_address(state, HL);
                swprintf_s(message, L"MOV %s,%s failed. Opcode=0x%02X, Expected=0x%02X, Actual=0x%02X",
                reg_names[HL_GENERAL_REGISTER], reg_names[source], opcode, expected, value_at_hl);
                Assert::AreEqual(expected, value_at_hl, message, LINE_INFO());
            }
        }

        TEST_METHOD(MOV_ShouldCopyHLToTarget)
        {
            for (GENERAL_REGISTER target = B; target < NUMBER_OF_OPCODE_REGISTERS; ++target) {
                if (target == HL_GENERAL_REGISTER) continue;
                uint8_t expected = (uint8_t)(target * 16 + 5);
				set_register_pair(state, HL, 0x0); // Set HL to a known value
				write_to_memory(state, 0x0, expected); // Write to memory at HL address
                uint8_t opcode = MOV + target * NUMBER_OF_CONDITIONS + HL_GENERAL_REGISTER;
                handle_mov(cpu, state, &opcode);

                wchar_t message[256];
                swprintf_s(message, L"MOV %s,%s failed. Opcode=0x%02X, Expected=0x%02X, Actual=0x%02X",
                    reg_names[target], reg_names[HL_GENERAL_REGISTER], opcode, expected, state->general_register[target]);
                Assert::AreEqual(expected, state->general_register[target], message, LINE_INFO());
            }
        }


        TEST_METHOD(ADD_WithoutCarry)
        {
            struct TestCase {
                uint8_t a_value;
                GENERAL_REGISTER src_reg;
                uint8_t src_value;
                uint8_t expected;
                bool carry;
                bool zero;
                bool sign;
                bool parity;
                const wchar_t* description;
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

            for (const auto& test : cases)
            {
                state->general_register[A] = test.a_value;
                state->general_register[test.src_reg] = test.src_value;

                uint8_t opcode = 0x80 + test.src_reg;
                handle_ADD(cpu, state, &opcode);

                std::wstringstream msg;
                msg << L"ADD A, " << reg_names[test.src_reg]
                    << L" | A = 0x" << std::hex << std::uppercase << (int)test.a_value
                    << L", " << reg_names[test.src_reg] << L" = 0x" << (int)test.src_value
                    << L" -> A = 0x" << (int)state->general_register[A];

                Assert::AreEqual(test.expected, state->general_register[A], msg.str().c_str());
                Assert::AreEqual(test.carry, state->cc[CARRY], (msg.str()).c_str());
                Assert::AreEqual(test.zero, state->cc[ZERO], (msg.str()).c_str());
                Assert::AreEqual(test.sign, state->cc[SIGN], (msg.str()).c_str());
                Assert::AreEqual(test.parity, state->cc[PARITY], (msg.str()).c_str());
            }
        }

        TEST_METHOD(ADD_IMMEDITAE) {
            state->general_register[A] = 0x01;
            state->pc = 0;
            uint8_t opcode[2] = { ADD_IMMEDIATE, 0xfe };
            handle_ADD(cpu, state, opcode);

            Assert::AreEqual((uint8_t)0xff, state->general_register[A]);        
            Assert::AreEqual((uint16_t)0x1, state->pc);
        }
        TEST_METHOD(ADD_WithCarry) {
            state->general_register[A] = 0x01;
            state->general_register[B] = 0xfe;
			state->cc[CARRY] = false; // Ensure carry is cleared
			uint8_t opcode = ADC + B; // ADC B
			handle_ADC(cpu, state, &opcode);

			Assert::AreEqual((uint8_t)0xff, state->general_register[A], L"ADC B should add B to A without carry");

            state->general_register[A] = 0x01;
            state->cc[CARRY] = true; // Ensure carry is cleared
            handle_ADC(cpu, state, &opcode);
            Assert::AreEqual((uint8_t)0x00, state->general_register[A], L"ADC B should add B to A without carry");
        }

        TEST_METHOD(SUB_IMMEDITAE) {
            state->general_register[A] = 0x02;
            state->pc = 0;
            uint8_t opcode[2] = { SUB_IMMEDIATE, 0x01 };
            handle_SUB(cpu, state, opcode);

            Assert::AreEqual((uint8_t)0x01, state->general_register[A]);
            Assert::AreEqual((uint16_t)0x1, state->pc);
        }
        TEST_METHOD(SUB_WithoutCarry) {
            state->general_register[A] = 0x02;
            state->general_register[C] = 0x01;
            uint8_t opcode = SUB + C;
            handle_SUB(cpu, state, &opcode);

            Assert::AreEqual((uint8_t)0x01, state->general_register[A]);

            state->general_register[A] = 0x00;
            handle_SUB(cpu, state, &opcode);
            Assert::AreEqual((uint8_t)0xff, state->general_register[A]);
        }

        TEST_METHOD(SUB_WithCarry) {
            state->general_register[A] = 0x02;
            state->general_register[C] = 0x01;
            state->cc[CARRY] = false;
            uint8_t opcode = SBB + C;
            handle_SBB(cpu, state, &opcode);

            Assert::AreEqual((uint8_t)0x01, state->general_register[A]);

            state->general_register[A] = 0x01;
            state->cc[CARRY] = true;
            handle_SBB(cpu, state, &opcode);
            Assert::AreEqual((uint8_t)0xff, state->general_register[A]);
        }

        TEST_METHOD(ANA_TEST) {
            TestCaseBitwiseOperation test1 = {
                .a_value = 0b11110000,
                .source_value = 0b00001111,
                .expected = 0x0,
                .zero = true,
                .sign = false,
                .parity = true,
                .src = D,
                .opcode = (OPCODE)(uint8_t)(ANA + D),
                .opcode_handler = handle_ANA
            };
            TestCaseBitwiseOperation test2 = {
                .a_value = 0b11110001,
                .source_value = 0b00000001,
                .expected = 0x1,
                .zero = false,
                .sign = false,
                .parity = false,
                .src = D,
                .opcode = (OPCODE)(uint8_t)(ANA + D),
                .opcode_handler = handle_ANA
            };

            TestCaseBitwiseOperation test3 = {
                .a_value = 0b11110000,
                .source_value = 0b00001111,
                .expected = 0x0,
                .zero = true,
                .sign = false,
                .parity = true,
                .opcode = ANA_IMMEDIATE,
                .opcode_handler = handle_ANA
            };
            TestCaseBitwiseOperation test4 = {
                .a_value = 0b11110001,
                .source_value = 0b00000001,
                .expected = 0x1,
                .zero = false,
                .sign = false,
                .parity = false,
                .opcode = ANA_IMMEDIATE,
                .opcode_handler = handle_ANA
            };

            test_bitwise(test1);
            test_bitwise(test2);
            test_bitwise(test3);

            test_bitwise(test4);
        }
        TEST_METHOD(XRA_TEST) {
            TestCaseBitwiseOperation test1 = {
                .a_value = 0b11110000,
                .source_value = 0b00001111,
                .expected = 0xff,
                .zero = false,
                .sign = true,
                .parity = true,
                .src = D,
                .opcode = (OPCODE)(uint8_t)(XRA + D),
                .opcode_handler = handle_XRA
            };

            TestCaseBitwiseOperation test2 = {
                .a_value = 0b11110001,
                .source_value = 0b00000001,
                .expected = 0xF0,
                .zero = false,
                .sign = true,
                .parity = true,
                .src = D,
                .opcode = (OPCODE)(uint8_t)(XRA + D),
                .opcode_handler = handle_XRA
            };

            TestCaseBitwiseOperation test3 = {
                .a_value = 0b11110000,
                .source_value = 0b00001111,
                .expected = 0xff,
                .zero = false,
                .sign = true,
                .parity = true,
                .opcode = XRA_IMMEDIATE,
                .opcode_handler = handle_XRA
            };
            TestCaseBitwiseOperation test4 = {
                .a_value = 0b11110001,
                .source_value = 0b00000001,
                .expected = 0xF0,
                .zero = false,
                .sign = true,
                .parity = true,
                .opcode = XRA_IMMEDIATE,
                .opcode_handler = handle_XRA
            };

            test_bitwise(test1);
            test_bitwise(test2);
            test_bitwise(test3);

            test_bitwise(test4);
        }
        
    };
}
