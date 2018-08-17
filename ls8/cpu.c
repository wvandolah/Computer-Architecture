#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

// Global branch table

// Declare an array of 256 pointers to functions (each of which accepts a
// pointer to a cpu and two unsigned char parameters and returns void),
// initializing each to NULL:

void (*branchtable[256])(struct cpu *cpu, unsigned char, unsigned char) = {0};

/**
 * Push a value on the CPU stack
 */
void cpu_push(struct cpu *cpu, unsigned char val)
{
  cpu->reg[SP]--;

  cpu->ram[cpu->reg[SP]] = val;
}

/**
 * Pop a value from the CPU stack
 */
unsigned char cpu_pop(struct cpu *cpu)
{
  unsigned char val = cpu->ram[cpu->reg[SP]];

  cpu->reg[SP]++;

  return val;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(char *filename, struct cpu *cpu)
{
  FILE *fp;
  char line[1024];
  int address = ADDR_PROGRAM_ENTRY;

  // Open the source file
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Cannot open file %s\n", filename);
    exit(2);
  }

  // Read all the lines and store them in RAM
  while (fgets(line, sizeof line, fp) != NULL) {

    // Convert string to a number
    char *endchar;
    unsigned char byte = strtol(line, &endchar, 2);;

    // Ignore lines from whicn no numbers were read
    if (endchar == line) {
      continue;
    }

    // Store in ram
    cpu->ram[address++] = byte;
  }
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  unsigned char *reg = cpu->reg;

  unsigned char valB = reg[regB];

  switch (op) {
    case ALU_MUL:
      reg[regA] *= valB;
      break;

    case ALU_ADD:
      reg[regA] += valB;
      break;
  }

}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  unsigned char *ram = cpu->ram;

  while (!cpu->halted) {
    unsigned char IR = ram[cpu->PC];

    unsigned char operandA = ram[(cpu->PC + 1) & 0xff];
    unsigned char operandB = ram[(cpu->PC + 2) & 0xff];

    // Declare "handler", a pointer to a handler function:
    void (*handler)(struct cpu*, unsigned char, unsigned char);

    // Look it up in the branch table:
    handler = branchtable[IR];

    if (handler == NULL) {
      fprintf(stderr, "PC %02x: unknown instruction %02x\n", cpu->PC, IR);
      exit(3);
    }

    // True if this instruction might set the PC
    cpu->inst_set_pc = (IR >> 4) & 1;

    handler(cpu, operandA, operandB);

    if (!cpu->inst_set_pc) {
      cpu->PC += ((IR >> 6) & 0x3) + 1;
    }

  }
}

void handle_LDI(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  cpu->reg[operandA] = operandB;
}

void handle_PRN(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandB; // suppress unused variable warnings

  printf("%d\n", cpu->reg[operandA]);
}

void handle_MUL(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  alu(cpu, ALU_MUL, operandA, operandB);
}

void handle_ADD(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  alu(cpu, ALU_ADD, operandA, operandB);
}

void handle_HLT(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandA; // suppress unused variable warnings
  (void)operandB;

  cpu->halted = 1;
}

void handle_PRA(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandB; // suppress unused variable warnings

  printf("%c\n", cpu->reg[operandA]);
  //printf("%c", cpu->reg[operandA]); fflush(stdout); // Without newline
}

void handle_CALL(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandB; // suppress unused variable warnings

  cpu_push(cpu, cpu->PC + 2);
  cpu->PC = cpu->reg[operandA];
}

void handle_RET(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandA; // suppress unused variable warnings
  (void)operandB;

  cpu->PC = cpu_pop(cpu);
}

void handle_PUSH(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandB; // suppress unused variable warnings

  cpu_push(cpu, cpu->reg[operandA]);
}

void handle_POP(struct cpu *cpu, unsigned char operandA, unsigned char operandB)
{
  (void)operandB; // suppress unused variable warnings

  cpu->reg[operandA] = cpu_pop(cpu);
}

/**
 * Initialize the branch table
 */
void init_branchtable(void)
{
  branchtable[LDI]  = handle_LDI;
  branchtable[PRN]  = handle_PRN;
  branchtable[MUL]  = handle_MUL;
  branchtable[ADD]  = handle_ADD;
  branchtable[HLT]  = handle_HLT;
  branchtable[PRA]  = handle_PRA;
  branchtable[CALL] = handle_CALL;
  branchtable[RET]  = handle_RET;
  branchtable[PUSH] = handle_PUSH;
  branchtable[POP]  = handle_POP;
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  cpu->PC = 0;

  // Zero registers and RAM
  memset(cpu->reg, 0, sizeof cpu->reg);
  memset(cpu->ram, 0, sizeof cpu->ram);

  // Initialize SP
  cpu->reg[SP] = ADDR_EMPTY_STACK;

  cpu->halted = 0;

  init_branchtable();
}
