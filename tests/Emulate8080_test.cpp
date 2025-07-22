#include "pch.h"
#include "CppUnitTest.h"

#include <bitset>

extern "C"
{
#include "Emulate8080.h"
}


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace CPU8080Emulatortest
{
	TEST_CLASS(CPU8080Emulatortest)
    {
    public:
		Cpu8080* cpu;
        State8080* state;

        // This is called before each test method runs
        TEST_METHOD_INITIALIZE(InitializeTestState)
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
        // This is called after each test method runs
        TEST_METHOD_CLEANUP(CleanupTestState)
        {
            free_cpu(cpu);
        }
        TEST_METHOD(TestByteToFlagsAl)
        {
            // Test case: All flags set (0b11010101)
            uint8_t flags = 0b11010101;
            byte_to_flags(state, flags);
            Assert::AreEqual(true, state->cc[CARRY], L"Carry flag should be set");
            Assert::AreEqual(true, state->cc[PARITY], L"Parity flag should be set");
            Assert::AreEqual(true, state->cc[AC], L"Auxiliary Carry flag should be set");
            Assert::AreEqual(true, state->cc[ZERO], L"Zero flag should be set");
            Assert::AreEqual(true, state->cc[SIGN], L"Sign flag should be set");

            // Test case: All flags cleared (0b00000000)

            flags = 0b00000000;
            byte_to_flags(state, flags);
            Assert::AreEqual(false, state->cc[CARRY], L"Carry flag should be cleared");
            Assert::AreEqual(false, state->cc[PARITY], L"Parity flag should be cleared");
            Assert::AreEqual(false, state->cc[AC], L"Auxiliary Carry flag should be cleared");
            Assert::AreEqual(false, state->cc[ZERO], L"Zero flag should be cleared");
            Assert::AreEqual(false, state->cc[SIGN], L"Sign flag should be cleared");

        }
        TEST_METHOD(TestFlagsToByte)
        {
            // Test case: All flags set
            state->cc[CARRY] = true;
            state->cc[PARITY] = true;
            state->cc[AC] = true;
            state->cc[ZERO] = true;
            state->cc[SIGN] = true;
            uint8_t expectedFlags = 0b11010111;
            uint8_t resultFlags = flags_to_byte(state);
            Assert::AreEqual(expectedFlags, resultFlags, L"Flags byte should match expected value when all flags are set");

            // Test case: All flags cleared
            state->cc[CARRY] = false;
            state->cc[PARITY] = false;
            state->cc[AC] = false;
            state->cc[ZERO] = false;
            state->cc[SIGN] = false;
            expectedFlags = 0b00000010;
            resultFlags = flags_to_byte(state);
            Assert::AreEqual(expectedFlags, resultFlags, L"Flags byte should match expected value when all flags are cleared");

            // Additional test cases can be added as needed
        }

        TEST_METHOD(IsPair_ValidPairs_ShouldReturnTrue)
        {
            Assert::IsTrue(is_pair(BC), L"BC should be a valid register pair");
            Assert::IsTrue(is_pair(DE), L"DE should be a valid register pair");
            Assert::IsTrue(is_pair(HL), L"HL should be a valid register pair");
        }
        TEST_METHOD(IsPair_InvalidPairs_ShouldReturnFalse)
        {
            Assert::IsFalse(is_pair(SP), L"SP should not be a valid register pair");
            Assert::IsFalse(is_pair(PSW), L"PSW should not be a valid register pair");
            Assert::IsFalse(is_pair(PC), L"PC should not be a valid register pair");
        }

        TEST_METHOD(Parity_Validation)
        {
            int values[] = { 0b00000000, 0b00000001, 0b00000011, 0b11111111, 0b00001111, 0b00001111 };
            int sizes[] = { 8,          8,          8,          8,          4,          3 };
            bool expected[] = { true,       false,      true,       true,       true,       false };

            for (int i = 0; i < _countof(values); i++)
            {
                bool result = parity(values[i], sizes[i]);

                std::wstringstream msg;
                msg << L"parity("
                    << std::bitset<8>(values[i])
                    << L", " << sizes[i]
                    << L") should be " << (expected[i] ? L"true" : L"false");

                    Assert::AreEqual(expected[i], result, msg.str().c_str());
            }
        }

        TEST_METHOD(Push_ShouldWriteHighAndLowBytesAndUpdateSP)
        {
            uint16_t original_sp = state->sp;

            uint8_t low = 0x34;
            uint8_t high = 0x12;

            push(state, low, high);

            // After push, SP should decrease by 2
            Assert::AreEqual((uint16_t)(original_sp - 2), state->sp, L"SP should be decremented by 2");

            // Check memory content at new SP
            Assert::AreEqual(low, state->memory[state->sp], L"Low byte should be at new SP");
            Assert::AreEqual(high, state->memory[state->sp + 1], L"High byte should be at SP + 1");
        }
        TEST_METHOD(PopByte_ShouldReturnByteAtSPAndIncrementSP)
        {
            uint16_t initial_sp = state->sp;
            uint8_t test_value = 0xAB;

            // Write test byte at SP
            state->memory[initial_sp] = test_value;

            // Call pop_byte
            uint8_t popped = pop_byte(state);

            // Check returned value
            Assert::AreEqual(test_value, popped, L"pop_byte should return the byte at SP");

            // Check that SP increased by 1
            Assert::AreEqual((uint16_t)(initial_sp + 1), state->sp, L"SP should increment by 1 after pop_byte");
        }
        TEST_METHOD(ReadFromMemory_ShouldReturnCorrectBytes)
        {
            // Write bytes at various memory locations
            state->memory[0x0000] = 0xAA;
            state->memory[0x1234] = 0x55;
            state->memory[0xFFFF] = 0xFF;

            Assert::AreEqual((uint8_t)0xAA, read_from_memory(state, 0x0000), L"Memory read at 0x0000 should be 0xAA");
            Assert::AreEqual((uint8_t)0x55, read_from_memory(state, 0x1234), L"Memory read at 0x1234 should be 0x55");
            Assert::AreEqual((uint8_t)0xFF, read_from_memory(state, 0xFFFF), L"Memory read at 0xFFFF should be 0xFF");
        }

        TEST_METHOD(WriteToMemory_ShouldStoreValueAtOffset)
        {
            uint16_t testOffset = 0x2001;
            uint8_t testValue = 0xAB;

            write_to_memory(state, testOffset, testValue);

            Assert::AreEqual(testValue, state->memory[testOffset], L"Memory at offset should contain the written value");
        }
        TEST_METHOD(BytesToWord_ShouldCombineLowAndHighCorrectly)
        {
            uint16_t low[] = { 0x34, 0xff, 0x00 };
            uint16_t high[] = { 0x12, 0x00, 0xff };
            uint16_t expected[] = { 0x1234, 0x00ff, 0xff00 };
			for (size_t i = 0; i < sizeof(low) / sizeof(low[0]); ++i)
			{
				uint16_t result = bytes_to_word(low[i], high[i]);
				Assert::AreEqual(expected[i], result, L"Should combine low and high bytes into a 16-bit word");
			}
        }

        TEST_METHOD(SetRegisterPair_ShouldSetHighAndLowBytes)
        {
            uint16_t testValue = 0xABCD;

            // Test BC pair
            set_register_pair(state, BC, testValue);
            Assert::AreEqual((uint8_t)0xAB, state->general_register[B], L"High byte of BC should be set to 0xAB");
            Assert::AreEqual((uint8_t)0xCD, state->general_register[C], L"Low byte of BC should be set to 0xCD");

            // Test DE pair
            set_register_pair(state, DE, testValue);
            Assert::AreEqual((uint8_t)0xAB, state->general_register[D], L"High byte of DE should be set to 0xAB");
            Assert::AreEqual((uint8_t)0xCD, state->general_register[E], L"Low byte of DE should be set to 0xCD");

            // Test HL pair
            set_register_pair(state, HL, testValue);
            Assert::AreEqual((uint8_t)0xAB, state->general_register[H], L"High byte of HL should be set to 0xAB");
            Assert::AreEqual((uint8_t)0xCD, state->general_register[L], L"Low byte of HL should be set to 0xCD");
        }
        TEST_METHOD(GetRegisterPair_ShouldReturnCombinedWord)
        {
            // Setup registers for BC pair
            state->general_register[B] = 0x12;
            state->general_register[C] = 0x34;
            uint16_t result = get_register_pair(state, BC);
            Assert::AreEqual((uint16_t)0x1234, result, L"get_register_pair BC should combine B and C correctly");

            // Setup registers for DE pair
            state->general_register[D] = 0xAB;
            state->general_register[E] = 0xCD;
            result = get_register_pair(state, DE);
            Assert::AreEqual((uint16_t)0xABCD, result, L"get_register_pair DE should combine D and E correctly");

            // Setup registers for HL pair
            state->general_register[H] = 0x56;
            state->general_register[L] = 0x78;
            result = get_register_pair(state, HL);
            Assert::AreEqual((uint16_t)0x5678, result, L"get_register_pair HL should combine H and L correctly");
        }

        TEST_METHOD(SetFlags_ShouldSetCarryZeroSignParity)
        {
            // Manually clear carry before test
            state->cc[CARRY] = false;

            // value = 0: ZERO set, SIGN cleared, PARITY even, CARRY cleared
            set_flags(state, 0x00);
            Assert::IsTrue(state->cc[ZERO], L"ZERO flag should be set for zero value");
            Assert::IsFalse(state->cc[SIGN], L"SIGN flag should be cleared for zero value");
            Assert::IsTrue(state->cc[PARITY], L"PARITY flag should be set for even parity");
            Assert::IsFalse(state->cc[CARRY], L"CARRY flag should be cleared for zero value");

            // value = 0x80 (10000000b): SIGN set, ZERO cleared, PARITY odd, CARRY cleared
            set_flags(state, 0x80);
            Assert::IsFalse(state->cc[ZERO], L"ZERO flag should be cleared for non-zero value");
            Assert::IsTrue(state->cc[SIGN], L"SIGN flag should be set for high bit");
            Assert::IsFalse(state->cc[PARITY], L"PARITY flag should be cleared for odd parity");
            Assert::IsFalse(state->cc[CARRY], L"CARRY flag should be cleared");


            // value = 0xFFF (all bits set): SIGN set, ZERO cleared, PARITY even, CARRY cleared
            set_flags(state, 0xFFF);
            Assert::IsFalse(state->cc[ZERO], L"ZERO flag cleared for non-zero");
            Assert::IsTrue(state->cc[SIGN], L"SIGN flag set for high bit");
            Assert::IsTrue(state->cc[PARITY], L"PARITY flag set for even parity");
            Assert::IsTrue(state->cc[CARRY], L"CARRY flag should be set");


            // For carry flag you can explicitly set or check if your function sets it based on some rules.
            // If set_flags does NOT update carry, then maybe skip or assert it stays as is.

			// value = 0x7F (01111111b): SIGN cleared, ZERO cleared, PARITY odd, CARRY cleared
            set_flags(state, 0b100111101);
            Assert::IsFalse(state->cc[ZERO], L"ZERO flag cleared");
            Assert::IsFalse(state->cc[SIGN], L"SIGN flag cleared");
            Assert::IsFalse(state->cc[PARITY], L"PARITY flag cleared for odd parity");
            Assert::IsTrue(state->cc[CARRY], L"CARRY flag should be set");

        }
        TEST_METHOD(ReadFromRegisterPairAddress_ShouldReturnMemoryByteAtRegisterPairAddress)
        {
            // Setup HL to point somewhere in memory
            uint16_t address = 0x1234;
            state->general_register[H] = (address >> 8) & 0xFF;
            state->general_register[L] = address & 0xFF;

            // Write a known byte at that address
            uint8_t expectedValue = 0xAB;
            state->memory[address] = expectedValue;

            // Read using the function
            uint8_t result = read_from_register_pair_address(state, HL);

            Assert::AreEqual(expectedValue, result, L"Value read from memory at HL address should match expected");
        }
        TEST_METHOD(WriteToRegisterPairAddress_ShouldWriteValueToMemoryAtRegisterPairAddress)
        {
            // Setup HL register pair with address 0x4567
            uint16_t address = 0x2001;
            state->general_register[H] = (address >> 8) & 0xFF;
            state->general_register[L] = address & 0xFF;

            uint8_t testValue = 0xCD;

            // Write the value at the address pointed by HL
            write_to_register_pair_address(state, HL, testValue);

            // Verify memory contains the value at that address
            Assert::AreEqual(testValue, state->memory[address], L"Memory at HL address should contain the written value");
        }
        TEST_METHOD(RSTOpcode_ShouldPushPCAndSetPC)
        {
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
            Assert::AreEqual((uint16_t)(0x2400 - 2), state->sp, L"Stack pointer should decrease by 2 after push");

            // Stack memory at new SP + 1 contains low byte, at new SP contains high byte
            Assert::AreEqual(expected_low, state->memory[state->sp], L"Low byte of PC should be pushed onto stack");
            Assert::AreEqual(expected_high, state->memory[state->sp + 1], L"High byte of PC should be pushed onto stack");

            // PC should be set to N * 8
            Assert::AreEqual((uint16_t)(N * 8), state->pc, L"Program counter should be set to N*8");
        }

    };
}
