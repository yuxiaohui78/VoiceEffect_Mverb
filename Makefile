CC=g++
CFLAGS=-I.
DEPS = MVerb.h
OBJ = main.o

%.o: %.c $(DEPS)
	$(CC) -c -g -o $@ $< $(CFLAGS)
	
main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm main.exe main.o