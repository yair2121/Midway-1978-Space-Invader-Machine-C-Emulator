
//#include <inttypes.h>
//#include <stdbool.h>
//#include <Emulate8080.h>
//

//typedef bool (*should_handle) (uint8_t opcode);
//
//typedef void (*handle_opcode) (Cpu8080* cpu, State8080* state, uint8_t* opcode);
//
//
//
//typedef enum OPCODE {
//	MOV = 0x40,
//	MOV_END_RANGE = MOV + NUMBER_OF_GENERAL_REGISTERS * NUMBER_OF_GENERAL_REGISTERS - 1,
//	ADD = 0x80,
//	ADD_END_RANGE = ADD + NUMBER_OF_GENERAL_REGISTERS - 1,
//	SUB = 0x90,
//	SUB_END_RANGE = SUB + NUMBER_OF_GENERAL_REGISTERS - 1,
//} OPCODE;
//
//#define SINGLE_VALUE_SHOULD_HANDLE(VALUE) bool is_value_##VALUE(uint8_t opcode) { return opcode == VALUE;}
//#define RANGE_VALUE_SHOULD_HANDLE(MIN, MAX) bool is_between_##MIN##MAX(uint8_t opcode) { return opcode >= MIN && opcode <= MAX;}
//
//
//
//typedef struct {
//	const should_handle shouldHandle;
//	const handle_opcode handle;
//} OpcodeHandler;
//
//void handle_mov(Cpu8080* cpu, State8080* state, uint8_t* opcode) {
//	GENERAL_REGISTER source = (*opcode - MOV) % NUMBER_OF_GENERAL_REGISTERS; // Get the register index
//	GENERAL_REGISTER target = (*opcode - MOV) / NUMBER_OF_GENERAL_REGISTERS; // Get the target register index
//
//	if (target == HL) {
//		state->general_register[target] = read_from_register_pair_address(state, HL);
//	}
//	else if (source == HL) {
//		// Write to memory at HL address
//	}
//	else {
//		state->general_register[target] = state->general_register[source];
//	}
//}
//
//SINGLE_VALUE_SHOULD_HANDLE(0x00);
//SINGLE_VALUE_SHOULD_HANDLE(0x01);
//RANGE_VALUE_SHOULD_HANDLE(MOV,MOV_END_RANGE); // MOV
//
//bool should_handle_nop(uint8_t opcode) {
//	switch (opcode)
//	{
//	case 0x00:
//	case 0x08:
//	case 0x10:
//	case 0x18:
//	case 0x20:
//	case 0x28:
//	case 0x38:
//	case 0xcb:
//	case 0xd9:
//	case 0xdd:
//	case 0xed:
//	case 0xfd:
//		return true;
//	default:
//		return false;
//	}
//}
//
//void handle_nop(Cpu8080* cpu, State8080* state, uint8_t* opcode) {}
//
//const OpcodeHandler handlers[] = {
//	{ should_handle_nop, handle_nop }, // NOP
//	{ is_value_0x01, NULL }, // LXI B,D16
//	{ is_between_MOVMOV_END_RANGE, handle_mov }, // MOV}
//	// ... Add more handlers as needed
//};



//template<int value>
//bool is_value(uint8_t opcode) {
//	return opcode == value;
//}

