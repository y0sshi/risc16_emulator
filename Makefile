CODE = code_file/risc16ba.code
OBJECT = risc16
C_SOURCE = emu_risc16
TYPE = ba

default: all

object: $(OBJECT).c
	gcc -c $(OBJECT).c

all: $(C_SOURCE).c $(OBJECT).o
	gcc $(C_SOURCE).c $(OBJECT).o -o $(C_SOURCE)

sim:
	./$(C_SOURCE) $(CODE) $(TYPE) 

check:
		diff imfiles/median.dump $(C_SOURCE).dump

clean:
	rm -f $(C_SOURCE) *.o *.dump *.log *.mem
