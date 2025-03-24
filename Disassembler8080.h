#pragma once

/// <summary>
/// Print the next Opcode. 
/// </summary>
/// <param name="codebuffer">Valid pointer to 8080 assembly code</param>
/// <param name="pc">Current offset into the code</param>
/// <returns>Size of op in bytes</returns>
int disassemble_8080_op(unsigned char* codebuffer, int pc);

void disassemble_code(size_t size, unsigned char* codebuffer);