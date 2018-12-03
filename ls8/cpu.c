#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define DATA_LEN 6

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *filename)
{
  // char data[DATA_LEN] = {
  //   // From print8.ls8
  //   0b10000010, // LDI R0,8
  //   0b00000000,
  //   0b00001000,
  //   0b01000111, // PRN R0
  //   0b00000000,
  //   0b00000001  // HLT
  // };

  // int address = 0;

  // for (int i = 0; i < DATA_LEN; i++) {
  //   cpu->ram[address++] = data[i];
  // }
  // TODO: Replace this with something less hard-coded
  // FILE pointer
  // char array to hold each line as a string
  // a counter var starting at zero
  // while loop fgets to char array

  // mine
  FILE *fp;
  // printf("argv -> %s\n", filename);
  char lineOf[500];
  int address = 0;
  fp = fopen(filename, "r");
  while(fgets(lineOf, sizeof(lineOf), fp)){
    // https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm
    char *endptr;
    unsigned char lineToBin = strtol(lineOf, &endptr, 2);
    if(lineOf == endptr){
      continue;
    }
    // printf("what is endptr:%s", endptr);
    // could use endptr to check if start of line is string and skip
    // printf("does end == line: %d\n", lineOf == endptr);
    cpu->ram[address] = lineToBin;
    address += 1;
  }

  fclose(fp);
}
unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address){
  return cpu->ram[address];
}

void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value){
  cpu->ram[address] = value;
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      cpu->reg[regA] *= cpu->reg[regB];
      break;
    case ALU_ADD:
      cpu->reg[regA] += cpu->reg[regB];
    case ALU_CMP:
      if(cpu->reg[regA] == cpu->reg[regB]){
        cpu->FL |= 0b00000001;
      }else{
        cpu->FL &= ~0b00000001;
      }

    // TODO: implement more ALU ops
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  while (running) {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    // 2. switch() over it to decide on a course of action.
    // 3. Do whatever the instruction should do according to the spec.
    // 4. Move the PC to the next instruction.
    unsigned char IR = cpu_ram_read(cpu, cpu->PC);
    unsigned char operandA = cpu_ram_read(cpu, cpu->PC + 1);
    unsigned char operandB = cpu_ram_read(cpu, cpu->PC + 2);
    unsigned char fromSP;
    printf("this is line| %d\n", cpu->PC);
    switch(IR){
      case LDI:
        cpu->reg[operandA] = operandB;
        break;
      case HLT:
        running = 0;
        break;
      case PRN:
        printf("Printing in PRN: %d\n", cpu->reg[operandA]);
        break;
      case MUL:
        alu(cpu, ALU_MUL, operandA, operandB);
        break;
      case ADD:
        alu(cpu, ALU_ADD, operandA, operandB);
        break;
      case PUSH:
        cpu->reg[SP]--;
        cpu_ram_write(cpu, cpu->reg[SP], cpu->reg[operandA]);
        break;
      case POP:
        fromSP = cpu_ram_read(cpu, cpu->reg[SP]);
        cpu->reg[operandA] = fromSP;
        cpu->reg[SP]++;
        break;
      case CALL:
        cpu->reg[SP]--;
        cpu_ram_write(cpu, cpu->reg[SP], cpu->PC + 1 + (IR >> 6));
        cpu->PC = cpu->reg[operandA];
        continue;
      case RET:
        fromSP = cpu_ram_read(cpu, cpu->reg[SP]);
        cpu->PC = fromSP;
        continue;
      case CMP:
        alu(cpu, ALU_CMP, operandA, operandB);
        break;
      case JMP:
        cpu->PC = cpu->reg[operandA];
        continue;
      case JEQ:
        if(cpu->FL & 0b00000001){
          cpu->PC = cpu->reg[operandA];
          continue;
        }
        break;
      case JNE:
        if(!(cpu->FL & 0b00000001)){
          cpu->PC = cpu->reg[operandA];
          continue;
        }
        break;
    }
    cpu->PC += 1 + (IR >> 6);
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->PC = 0;
  cpu->reg[SP] = 0xF4;
  // TODO: Zero registers and RAM
  memset(cpu->reg, 0, sizeof(cpu->reg));
  memset(cpu->ram, 0, sizeof(cpu->ram));
}
