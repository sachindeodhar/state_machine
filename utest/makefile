CC=gcc
CFLAGS=-I. -I../state_machine
DEPS = state_machine.h state_machine_types.h
OBJ = ../state_machine/state_machine.o test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

utest: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o ../state_machine/*.o
	rm utest
