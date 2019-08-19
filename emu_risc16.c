#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "risc16.h"

#define ADDR_WIDTH 0xFFFF + 1
#define CHAR_SIZE 256

int main(int argc, char *argv[]) {
  int type;
  int MEM[ADDR_WIDTH];
  FILE *fp;
  char filename[CHAR_SIZE];

  // code file
  type = OPEN_CODE_FILE(fp,argc,argv,filename);
  if ((fp=fopen(filename,"r")) == NULL) {
    printf("file open error: \"%s\"\n",filename); return 0;
  }
  else {
    printf("\"%s\" open done\n",filename);
  }

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

  Parse_instruction(fp, type, MEM); // syntax check & translation from code to mem
  fclose(fp);

  if ((fp=fopen(filename,"r")) == NULL) {
    printf("file open error: \"%s\"\n",filename);
    return 0;
  }
  else {
    printf("\"%s\" open done\n",filename);
  }

  simulation(fp, type, MEM);
  fclose(fp);

  return 0;
}
