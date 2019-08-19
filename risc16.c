#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "risc16.h"

#define REGISTER_NUM 8
#define ADDR_WIDTH 0xFFFF + 1
#define BIT_WIDTH 8
#define BIT_WIDTH_2 BIT_WIDTH * 2

#define CHAR_SIZE 256

int *label_table;

int OPEN_CODE_FILE(FILE *fp, int argc, char **argv, char *filename){
  int type;
  //char filename[CHAR_SIZE];

  // file name & risc16 type
  if (argc >= 2){
    strcpy(filename,argv[1]);
    if (argc == 2) {
      type = 0; // normal risc16
    }
    else {
      if (!strcmp(argv[2],"p")) {      // risc16p
        type = 1;
      }
      else if (!strcmp(argv[2],"f")) { // risc16f
        type = 2;
      }
      else if (!strcmp(argv[2],"b")) { // risc16b
        type = 3;
      }
      else if (!strcmp(argv[2],"ba")) { // risc16ba
        type = 4;
      }
      else {                           // risc16
        type = 0;
      }
    }
  }
  else {
    printf("input filename: ");
    scanf("%s",&filename);

    do {
      printf("input type(default=0, p=1, f=2, b=3, ba=4): ");
      scanf("%d",&type);
    } while((type < 0) | (3 < type));
  }

  return type;
}

int no_newline(char *word, int word_size) {
  int i;
  int size = 0;

  for (i=0;i<word_size;i++){
    size++;
    if (word[i] == '\n'){
      word[i] = '\0';
      return size;
    }
  }
  return size;
}

int Parse_instruction(FILE *fp, int type, int *MEM) {
  int comment_flag = 0;
  int op_flag = 0;
  int line = 0;
  int label_flag = 0;
  int i, word_size, label_val;
  char readline[CHAR_SIZE], filename[CHAR_SIZE], readtemp[CHAR_SIZE];
  char period[] = ":; ";

  char *op;
  char *arg1;
  char *arg2;
  char *label; // jump target label
  char *comment;
  char instruction[CHAR_SIZE], instruction0[CHAR_SIZE], instruction1[CHAR_SIZE];

  label_table = (int *)malloc(sizeof(int)*256);

  // jump label parse {
  printf("jump label parsing ...\n");
  while ( fgets(readline, CHAR_SIZE, fp) != NULL ) {
    no_newline(readline,CHAR_SIZE);
    strcpy(readtemp,readline);
    if (readtemp[0] != '\0') {
      op = strtok(readtemp,period);
      no_newline(op,CHAR_SIZE);
      if ((comment = strstr(op,"//")) == NULL){ // not comment out
        if ((label = strstr(readline,"label:")) != NULL) {
          label = strtok(readline,":");
          label = strtok(NULL,"");

          label_val = strtol(label,NULL,10);
          label_table[label_val] = line;
          printf("  label_table[%d]: %d\n",label_val,line);
        }
        line++;
      }
    }
  }
  printf("jump label parse done!!\n");
  printf("----------------------------------------------------------------\n\n");
  rewind(fp);
  line = 0;
  // } jump label parse

  switch (type) {
    case 0: // normal
      strcpy(filename,"./sim_risc16.mem");
      break;
    case 1: // p
      strcpy(filename,"./sim_risc16p.mem");
      break;
    case 2: // f
      strcpy(filename,"./sim_risc16f.mem");
      break;
    case 3: // b
      strcpy(filename,"./sim_risc16b.mem");
      break;
    case 4: // ba
      strcpy(filename,"./sim_risc16ba.mem");
      break;
    default :
      strcpy(filename,"./sim_risc16.mem");
      break;
  }


  FILE *fp_out = fopen(filename,"w");

  printf("instruction parsing ...\n");
  while ( fgets(readline, CHAR_SIZE, fp) != NULL ) {
    word_size = no_newline(readline,CHAR_SIZE);

    strcpy(readtemp,readline);
    //printf("  %s\n",readtemp);
    if (readtemp[0] != '\0') {
      comment_flag = 0;
      op_flag = 0;
      label_flag = 0;

      // op
      op = NULL;
      op = strtok(readtemp,period);
      no_newline(op,CHAR_SIZE);
      if ((comment = strstr(op,"//")) != NULL){ // comment out
        comment_flag = 1;
        op_flag = 1;
      }
      else if (!strcmp(op,"NOP") | !strcmp(op,"FINISH")) {
        op_flag = 1;
      }

      // arg1
      arg1 = NULL;
      arg1 = strtok(NULL,period);
      if (arg1 != NULL) {
        no_newline(arg1,CHAR_SIZE);
        if ((comment = strstr(arg1,"//")) != NULL){ // comment out
          comment_flag = 1;
        }
        else if (!strcmp(op,"JMP")){
          // JMP
          op_flag = 1;
        }
      }

      // arg2
      arg2 = NULL;
      arg2 = strtok(NULL,period);
      if (arg2 != NULL) {
        no_newline(arg2,CHAR_SIZE);
        if ((comment = strstr(arg2,"//")) != NULL){ // comment out
          comment_flag = 1;
        }
        else {
          op_flag = 1;
        }
      }

      if (!op_flag){
        printf("  syntax error: line %d\n",line);
        break;
      }

      // write file
      if ((comment = strstr(op,"//")) == NULL){ // not comment out
        // instruction word
        if (!strcmp(op,"JMP")){
          instruction_arg1(instruction,op,arg1,line);
        }
        else if (!strcmp(op,"NOP") | !strcmp(op,"FINISH")) {
          bit_char(instruction,0,16);
        }
        else {
          instruction_arg2(instruction,op,arg1,arg2,line);
        }

        // write mem file
        for (i=0;i<BIT_WIDTH;i++) {
          instruction0[i] = instruction[i];
          instruction1[i] = instruction[i+BIT_WIDTH];
        }
        instruction0[BIT_WIDTH] = '\0';
        instruction1[BIT_WIDTH] = '\0';
        if (strcmp(op,"MEM")) {
          fprintf(fp_out,"@%02x %s %s // %s\n",line*2,instruction0,instruction1,readline);
        }
        else {
          fprintf(fp_out,"@%02x %s %s // %s\n",strtol(arg1,NULL,16),instruction0,instruction1,readline);
        }
        line++;
      }
      else { // comment out
        fprintf(fp_out,"%s\n",readline);
      }
    }
    else {
      fprintf(fp_out,"\n");
    }
  }
  printf("instruction parse done!!\n");
  fclose(fp_out);

  free(label_table);

  return 0;
}

void instruction_arg1(char *instruction, char *op, char *arg1, int line){
  int i, diff, a1, hex_flag1, l_flag=0;
  char diff_bit[CHAR_SIZE], func[CHAR_SIZE];
  func_jump func_j;

  if (arg1[0] == 'r' || arg1[0] == 'l'){
    if (arg1[0] == 'l') {
      l_flag = 1;
    }
    for (i=1;i<CHAR_SIZE;i++){
      arg1[i-1] = arg1[i];
      if (arg1[i] == '\0') {
        break;
      }
    }
  }
  else if (arg1[0] == '0' && arg1[1] == 'x') {
    hex_flag1 = 1;
  }

  if (hex_flag1){
    a1 = strtol(arg1,NULL,16);
  }
  else {
    a1 = strtol(arg1,NULL,10);
  }

  diff = (label_table[a1] - (line+1))*2;

  // Jump type
  if (!strcmp(op,"JMP")) {
    func_j = JMP;

    sprintf(instruction,"11\0");
    bit_char(func,func_j,3);
    strcat(instruction,func);
    bit_char(diff_bit,diff,11);
    strcat(instruction,diff_bit);
  }
}

void instruction_arg2(char *instruction, char *op, char *arg1, char *arg2, int line){
  int i, diff, a1, a2, hex_flag1=0, hex_flag2=0, l_flag=0;
  char diff_bit[CHAR_SIZE], func[CHAR_SIZE], reg[CHAR_SIZE];

  func_register func_r;
  func_memory func_m;
  func_immediate func_i;
  func_branch func_b;
  op_type op_t;

  if (arg1[0] == 'r' || arg1[0] == 'l'){
    if (arg1[0] == 'l') {
      l_flag = 1;
    }
    for (i=1;i<CHAR_SIZE;i++){
      arg1[i-1] = arg1[i];
      if (arg1[i] == '\0') {
        break;
      }
    }
  }
  else if (arg1[0] == '0' && arg1[1] == 'x') {
    hex_flag1 = 1;
  }

  if (arg2[0] == 'r' || arg2[0] == 'l'){
    if (arg2[0] == 'l') {
      l_flag = 1;
    }
    for (i=1;i<CHAR_SIZE;i++){
      arg2[i-1] = arg2[i];
      if (arg2[i] == '\0') {
        break;
      }
    }
  }
  else if (arg2[0] == '0' && arg2[1] == 'x') {
    hex_flag2 = 1;
  }

  if (hex_flag1){
    a1 = strtol(arg1,NULL,16);
  }
  else {
    a1 = strtol(arg1,NULL,10);
  }

  if (hex_flag2) {
    a2 = strtol(arg2,NULL,16);
  }
  else {
    a2 = strtol(arg2,NULL,10);
  }

  diff = (label_table[a2] - (line+1))*2;
  //printf("a2:%d diff:%d\n",a2,diff);

  // Register, Memory, Immediate, Branch type
  if (!strcmp(op,"MV")) {
    op_t = Register;
    func_r = MV;
  }
  else if (!strcmp(op,"NOT")) {
    op_t = Register; func_r = NOT;
  }
  else if (!strcmp(op,"XOR")) {
    op_t = Register;
    func_r = XOR;
  }
  else if (!strcmp(op,"ADD")) {
    op_t = Register;
    func_r = ADD;
  }
  else if (!strcmp(op,"SUB")) {
    op_t = Register;
    func_r = SUB;
  }
  else if (!strcmp(op,"SL")) {
    op_t = Register;
    func_r = SL;
  }
  else if (!strcmp(op,"SR")) {
    op_t = Register;
    func_r = SR;
  }
  else if (!strcmp(op,"AND")) {
    op_t = Register;
    func_r = AND;
  }
  else if (!strcmp(op,"OR")) {
    op_t = Register;
    func_r = OR;
  }
  else if (!strcmp(op,"ST")) {
    op_t = Memory;
    func_m = ST;
  }
  else if (!strcmp(op,"LD")) {
    op_t = Memory;
    func_m = LD;
  }
  else if (!strcmp(op,"SBU")) {
    op_t = Memory;
    func_m = SBU;
  }
  else if (!strcmp(op,"LBU")) {
    op_t = Memory;
    func_m = LBU;
  }
  else if (!strcmp(op,"ADDI")) {
    op_t = Immediate;
    func_i = ADDI;
  }
  else if (!strcmp(op,"ANDI")) {
    op_t = Immediate;
    func_i = ANDI;
  }
  else if (!strcmp(op,"ORI")) {
    op_t = Immediate;
    func_i = ORI;
  }
  else if (!strcmp(op,"LLI")) {
    op_t = Immediate;
    func_i = LLI;
  }
  else if (!strcmp(op,"LUI")) {
    op_t = Immediate;
    func_i = LUI;
  }
  else if (!strcmp(op,"BNEZ")) {
    op_t = Branch;
    func_b = BNEZ;
  } else if (!strcmp(op,"BEQZ")) { op_t = Branch;
    func_b = BEQZ;
  }
  else if (!strcmp(op,"BMI")) {
    op_t = Branch;
    func_b = BMI;
  }
  else if (!strcmp(op,"BPL")) {
    op_t = Branch;
    func_b = BPL;
  }
  else if (!strcmp(op,"MEM")) {
    op_t = MEM;
  }
  else { // NOP
    op_t = Register;
    func_r = NOP;
  }

  switch(op_t) {
    case Register:
      sprintf(instruction,"00000\0");
      bit_char(reg,a1,3);
      strcat(instruction,reg);
      bit_char(reg,a2,3);
      strcat(instruction,reg);
      strcat(instruction,"0\0");
      bit_char(func,func_r,4);
      strcat(instruction,func);
      break;
    case Memory:
      sprintf(instruction,"00000\0");
      bit_char(reg,a1,3);
      strcat(instruction,reg);
      bit_char(reg,a2,3);
      strcat(instruction,reg);
      strcat(instruction,"1\0");
      bit_char(func,func_m,4);
      strcat(instruction,func);
      break;
    case Immediate:
      sprintf(instruction,"0\0");
      bit_char(func,func_i,4);
      strcat(instruction,func);
      bit_char(reg,a1,3);
      strcat(instruction,reg);
      bit_char(reg,a2,8);
      strcat(instruction,reg);
      break;
    case Branch:
      sprintf(instruction,"10\0");
      bit_char(func,func_b,3);
      strcat(instruction,func);
      bit_char(reg,a1,3);
      strcat(instruction,reg);
      bit_char(diff_bit,diff,8);
      strcat(instruction,diff_bit);
      break;
    case MEM:
      bit_char(func,a2,16);
      sprintf(instruction,"%s\0",func);
      break;
    default: // NOP
      sprintf(instruction,"0000000000000000\0");
      break;
  }
}

void bit_char(char *bit, int value, int width){
  int i;
  unsigned int b,temp=value;
  char p_char[CHAR_SIZE];

  bit[0] = '\0';
  for (i=0;i<width;i++){
    b = temp % 2;
    sprintf(p_char,"%u\0",b);
    temp /= 2;
    strcat(p_char, bit);
    strcpy(bit, p_char);
  }
}

int simulation(FILE *fp, int type, int *MEM) {
  int REG[REGISTER_NUM], led;
  int reg0, reg1, op, immediate, addr_offset;
  int i;
  int clock_cycles;
  short pc_we, pc_we_next, finish;

  char readline[CHAR_SIZE], readtemp[CHAR_SIZE], temp[CHAR_SIZE], temp1[CHAR_SIZE];
  char bit_data0[CHAR_SIZE], bit_data1[CHAR_SIZE];
  char *pc_char, *i0_char, *i1_char, *comment, *check;
  char period[] = "@ ";
  char filename[] = "./imfiles/set_image.mem";
  FILE *fp_sim;

  int pc, pc_next, instruction, data0, data1;

  // initialize
  pc = 0;
  for (i=0; i<ADDR_WIDTH; i += 8) {
    MEM[i] = 0;
    MEM[i + 1] = 0;
    MEM[i + 2] = 0;
    MEM[i + 3] = 0;
    MEM[i + 4] = 0;
    MEM[i + 5] = 0;
    MEM[i + 6] = 0;
    MEM[i + 7] = 0;
  }

  fp_sim = fopen(filename,"r");
  while ( fgets(readline, CHAR_SIZE, fp_sim) != NULL){
    no_newline(readline,CHAR_SIZE); strcpy(readtemp,readline);
    pc_char = strtok(readline,period);
    pc = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 1] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 2] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 3] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 4] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 5] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 6] = strtol(pc_char,NULL,16);

    pc_char = strtok(NULL,period);
    MEM[pc + 7] = strtol(pc_char,NULL,16);

    pc += 8;
  }
  fclose(fp_sim);

  for (i=0; i<REGISTER_NUM; i++) {
    REG[i] = 0;
  }

  // read mem file
  finish = 0;
  while ( fgets(readline, CHAR_SIZE, fp) != NULL){
    no_newline(readline,CHAR_SIZE);
    strcpy(readtemp,readline);
    if (readtemp[0] != '\0') {
      pc_char = strtok(readtemp,period);
      strcpy(temp,pc_char);
      if (strstr(temp,"//") == NULL) {
        temp[0] = '\0';
        i0_char = strtok(NULL,period);
        i1_char = strtok(NULL,period);
        data0 = strtol(i0_char,NULL,2);
        data1 = strtol(i1_char,NULL,2);
        strcat(temp,i0_char);
        strcat(temp,i1_char);
        pc = strtol(pc_char,NULL,16);
        instruction = strtol(temp,NULL,2);
        MEM[pc] = data0;
        MEM[pc + 1] = data1;
        check = strtok(NULL, period);
        if (!strcmp(check, "//")) {
          comment = strtok(NULL, period);
          if (!strcmp(comment, "FINISH")) {
            finish = pc;
            printf("finish: %x\n", finish);
          }
        }
      }
    }
  }

  // simulation
  i = 0;
  pc_next = 0;
  clock_cycles = 0;
  pc_we_next = 0;
  strcpy(filename,"./emu_risc16.log\0");
  fp_sim = fopen(filename,"w");
  while (pc != finish) { // loop
    pc = pc_next;
    pc_we = pc_we_next;
    addr_offset = immediate;

    bit_char(bit_data0, MEM[pc], 8);
    bit_char(bit_data1, MEM[pc + 1], 8);

    if (bit_data0[0] == '0') { // Register or Memory or Immediate
      pc_we_next = 0;
      if (bit_data0[1] == '0' && bit_data0[2] == '0' && bit_data0[3] == '0'
          && bit_data0[4] == '0') { // Register or Memory
        if (bit_data1[3] == '0') { // Register
          sprintf(temp,"%c%c%c\0",bit_data0[5],bit_data0[6],bit_data0[7]);
          reg0 = strtol(temp,NULL,2);
          sprintf(temp,"%c%c%c\0",bit_data1[0],bit_data1[1],bit_data1[2]);
          reg1 = strtol(temp,NULL,2);
          sprintf(temp,"%c%c%c%c\0",bit_data1[4],bit_data1[5],bit_data1[6],bit_data1[7]);
          op = strtol(temp,NULL,2);;
          Register_sim(REG,reg0,reg1,op);
        }
        else { // Memory
          sprintf(temp,"%c%c%c\0",bit_data0[5],bit_data0[6],bit_data0[7]);
          reg0 = strtol(temp,NULL,2);
          sprintf(temp,"%c%c%c\0",bit_data1[0],bit_data1[1],bit_data1[2]);
          reg1 = strtol(temp,NULL,2);
          sprintf(temp,"%c%c%c%c\0",bit_data1[4],bit_data1[5],bit_data1[6],bit_data1[7]);
          op = strtol(temp,NULL,2);;
          Memory_sim(MEM,REG,reg0,reg1,op);
        }
      }
      else { // Immediate
        sprintf(temp,"%c%c%c%c\0",bit_data0[1],bit_data0[2],bit_data0[3],bit_data0[4]);
        op = strtol(temp,NULL,2);;
        sprintf(temp,"%c%c%c\0",bit_data0[5],bit_data0[6],bit_data0[7]);
        reg0 = strtol(temp,NULL,2);
        immediate = strtol(bit_data1,NULL,2);
        Immediate_sim(REG, reg0, immediate, op);
      }
    }
    else { // Branch or Jump
      sprintf(temp,"%c%c%c\0",bit_data0[2],bit_data0[3],bit_data0[4]);
      op = strtol(temp,NULL,2);
      if (bit_data0[1] == '0') { // Branch
        sprintf(temp,"%c%c%c\0",bit_data0[5],bit_data0[6],bit_data0[7]);
        reg0 = strtol(temp,NULL,2);

        immediate = strtol(bit_data1,NULL,2); // calculate offset
        bit_char(temp,immediate,16);
        if (temp[8] == '1') {
          for (i=0;i<BIT_WIDTH;i++) {
            temp[i] = '1';
          }
        }
        immediate = strtol(temp,NULL,2);
        
        pc_we_next = Branch_sim(REG,reg0,op);
      }
      else { // Jump
        sprintf(temp,"%c%c%c\0",bit_data0[5],bit_data0[6],bit_data0[7]);
        strcat(temp,bit_data1);
        immediate = strtol(temp,NULL,2);
        bit_char(temp,immediate,16);
        if (temp[5] == '1') {
          for (i=0;i<5;i++) {
            temp[i] = '1';
          }
        }
        immediate = strtol(temp,NULL,2);
        
        pc_we_next = Jump_sim(op);
      }
    }

    // program counter
    pc_next = (pc_we) ? pc + addr_offset : pc + 2;
    bit_char(temp,pc_next,16);
    pc_next = strtol(temp,NULL,2);

    // led
    bit_char(temp,MEM[0x0200],8);
    bit_char(temp1,MEM[0x0201],8);
    strcat(temp,temp1);
    led = strtol(temp,NULL,2);

    // --- print --- //
    if (clock_cycles % 1000000 == 0) {
      printf("# ==== clock: %1d ====\n",clock_cycles);
      printf("pc: %04x pc_next: %04x pc_we: %01x led: %04x immediate: %04x\n",pc,pc_next,pc_we,led,immediate);
      printf("# reg:");
      for (i=0; i<REGISTER_NUM; i++){
        printf(" %04x",REG[i]);
      }
      printf("\n");
    }
    fprintf(fp_sim,"# ==== clock: %1d ====\n",clock_cycles);
    fprintf(fp_sim,"# pc: %04x instruction: %s %s\n",pc,bit_data0,bit_data1);
    fprintf(fp_sim,"# pc_next: %04x pc_we: %01x led: %04x\n",pc_next, pc_we, led);
    fprintf(fp_sim,"# reg:");
    for (i=0; i<REGISTER_NUM; i++){
      fprintf(fp_sim," %04x",REG[i]);
    }
    fprintf(fp_sim,"\n");

    fprintf(fp_sim,"# mem:");
    for (i=0xf0;i<0xf9;i++) {
      fprintf(fp_sim," %02x",MEM[i]);
    }
    fprintf(fp_sim," | %02x %02x\n",MEM[0xf9], MEM[0xfa]);

    fprintf(fp_sim,"\n");

    clock_cycles++;
  }
  printf("clock_cycles: %d\n", clock_cycles - 1);
  printf("simulation done !\n");
  fprintf(fp_sim,"clock_cycles: %d\n", clock_cycles - 1);
  fprintf(fp_sim,"simulation done !\n");
  fclose(fp_sim);

  // write simulation dump
  strcpy(filename,"./emu_risc16.dump");

  fp_sim = fopen(filename,"w");
  printf("writing simulation dump...\n\n");
  for (i=0xc000; i<0xffff; i+=8) {
    fprintf(fp_sim, "%02x %02x ",MEM[i],     MEM[i + 1]);
    fprintf(fp_sim, "%02x %02x ",MEM[i + 2], MEM[i + 3]);
    fprintf(fp_sim, "%02x %02x ",MEM[i + 4], MEM[i + 5]);
    fprintf(fp_sim, "%02x %02x",MEM[i + 6], MEM[i + 7]);
    fprintf(fp_sim, "\n");
  }

  fclose(fp_sim);
  printf("done !\n");
  return 0;
}

int Register_sim(int *reg, int reg0, int reg1, func_register op) {
  int i;
  char temp[CHAR_SIZE];

  switch (op) {
    case MV:
      reg[reg0] = reg[reg1];
      break;
    case NOT:
      reg[reg0] = ~reg[reg1];
      break;
    case XOR:
      reg[reg0] = reg[reg0] ^ reg[reg1];
      break;
    case ADD:
      reg[reg0] = reg[reg0] + reg[reg1];
      bit_char(temp,reg[reg0],BIT_WIDTH_2);
      reg[reg0] = strtol(temp,NULL,2);
      break;
    case SUB:
      reg[reg0] = reg[reg0] - reg[reg1];
      bit_char(temp,reg[reg0],BIT_WIDTH_2);
      reg[reg0] = strtol(temp,NULL,2);
      break;
    case SL:
      bit_char(temp,reg[reg1],BIT_WIDTH_2);
      for (i=0;i<BIT_WIDTH_2-1;i++) {
        temp[i] = temp[i+1];
      }
      temp[BIT_WIDTH_2-1] = '0';
      break;
    case SR: break;
      bit_char(temp,reg[reg1],BIT_WIDTH_2);
      for (i=BIT_WIDTH_2-1;i>0;i++) {
        temp[i] = temp[i-1];
      }
      temp[0] = '0';
    case AND:
      reg[reg0] = reg[reg0] & reg[reg1];
      break;
    case OR:
      reg[reg0] = reg[reg0] | reg[reg1];
      break;
    default:
      break;
  }

  return 0;
}

int Memory_sim(int *mem, int *reg, int reg0, int reg1, func_memory op) {
  int addr, i;
  char temp[CHAR_SIZE], temp0[CHAR_SIZE], temp1[CHAR_SIZE];

  addr = reg[reg1];

  switch (op) {
    case ST:
      if (addr % 2 == 1) {
        addr = addr - 1;
      }
      bit_char(temp,reg[reg0],BIT_WIDTH_2);
      for (i=0; i<BIT_WIDTH; i++) {
        temp0[i] = temp[i];
        temp1[i] = temp[i + BIT_WIDTH];
      }
      temp0[BIT_WIDTH] = '\0';
      temp1[BIT_WIDTH] = '\0';
      mem[addr] = strtol(temp0,NULL,2);
      mem[addr + 1] = strtol(temp1,NULL,2);
      break;
    case LD:
      if (addr % 2 == 1) {
        addr = addr - 1;
      }
      bit_char(temp0,mem[addr],BIT_WIDTH);
      bit_char(temp1,mem[addr + 1],BIT_WIDTH);
      strcpy(temp,temp0);
      strcat(temp,temp1);
      reg[reg0] = strtol(temp,NULL,2);
      break;
    case SBU:
      bit_char(temp,reg[reg0],BIT_WIDTH);
      mem[addr] = strtol(temp,NULL,2);
      break;
    case LBU:
      bit_char(temp,mem[addr],BIT_WIDTH);
      reg[reg0] = strtol(temp,NULL,2);
      break;
    default:
      break;
  }

  return 0;
}

int Immediate_sim(int *reg, int reg0, int immediate, func_immediate op) {
  int i;
  char temp[CHAR_SIZE];

  switch (op) {
    case ADDI:
      bit_char(temp,immediate,BIT_WIDTH_2);
      if (temp[BIT_WIDTH] == '1') {
        for (i=0; i<BIT_WIDTH; i++) {
          temp[i] = '1';
        }
      }
      immediate = strtol(temp,NULL,2);
      reg[reg0] = reg[reg0] + immediate;
      bit_char(temp,reg[reg0],BIT_WIDTH_2);
      reg[reg0] = strtol(temp,NULL,2);
      break;
    case ANDI:
      reg[reg0] = reg[reg0] & immediate;
      break;
    case ORI:
      reg[reg0] = reg[reg0] | immediate;
      break;
    case LLI:
      reg[reg0] = immediate;
      break;
    case LUI:
      bit_char(temp,immediate,16);
      for (i=0; i<8; i++) {
        temp[i] = temp[i + 8];
        temp[i + 8] = '0';
      }
      immediate = strtol(temp,NULL,2);
      reg[reg0] = immediate;
      break;
    default:
      break;
  }

  return 0;
}

int Branch_sim(int *reg, int reg0, func_branch op) {
  int r0, pc_we;
  char temp[CHAR_SIZE];

  bit_char(temp,reg[reg0],16);
  r0 = strtol(temp,NULL,2);

  switch (op) {
    case BNEZ:
      pc_we = (r0 != 0);
      break;
    case BEQZ:
      pc_we = (r0 == 0);
      break;
    case BMI:
      pc_we = (temp[0] == '1');
      break;
    case BPL:
      pc_we = (temp[0] == '0');
      break;
    default:
      pc_we = 0;
      break;
  }

  return pc_we;
}

int Jump_sim (func_jump op) {
  short pc_we;

  switch (op) {
    case JMP:
      pc_we = 1;
      break;
    default:
      pc_we = 0;
      break;
  }

  return pc_we;
}
