#ifndef RISC16_H
#define RISC16_H

typedef enum op_type {
  Register,
  Memory,
  Immediate,
  Branch,
  Jump,
  MEM
} op_type;

typedef enum func_register {
  NOP,
  MV,
  NOT,
  XOR,
  ADD,
  SUB,
  TMP_r0,
  TMP_r1,
  SL,
  SR,
  AND,
  OR
} func_register;

typedef enum func_memory {
  ST,
  LD,
  SBU,
  LBU
} func_memory;

typedef enum func_immediate {
  TMP_i0,
  LLI,
  TMP_i1,
  TMP_i2,
  ADDI,
  TMP_i3,
  LUI,
  TMP_i4,
  TMP_i5,
  TMP_i6,
  ANDI,
  ORI
} func_immediate;

typedef enum func_branch {
  BNEZ,
  BEQZ,
  BMI,
  BPL
} func_branch;

typedef enum func_jump {
  JMP
} func_jump;

// code file parse
int no_newline(char *word, int word_size);
int OPEN_CODE_FILE(FILE *fp, int argc, char **argv, char *filename);
int Parse_instruction(FILE *fp, int type, int *MEM); // syntax
void instruction_arg1(char *instruction, char *op, char *arg1, int line);
void instruction_arg2(char *instruction, char *op, char *arg1, char *arg2, int line);

// mem file simulation
int simulation(FILE *fp, int type, int *MEM);
int Register_sim(int *reg, int reg0, int reg1, func_register op);
int Memory_sim(int *mem, int *reg, int reg0, int reg1, func_memory op);
int Immediate_sim(int *reg, int reg0, int immediate, func_immediate op);
int Branch_sim(int *reg, int reg0, func_branch op);
int Jump_sim(func_jump op);

void bit_char(char *bit, int value, int width);

#endif
