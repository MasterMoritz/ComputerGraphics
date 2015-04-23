CC = gcc
OBJ = MerryGoRound.o LoadShader.o Matrix.o
CFLAGS = -g -std=c99 -Wall -Wextra

LDLIBS=-lm -lglut -lGLEW -lGL

MerryGoRound: $(OBJ)
	 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

clean:
	rm -f *.o MerryGoRound 
.PHONY: all clean
