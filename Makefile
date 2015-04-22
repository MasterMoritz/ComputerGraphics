CC = gcc
OBJ = RotatingCube.o LoadShader.o Matrix.o
CFLAGS = -g -Wall -Wextra

LDLIBS=-lm -lglut -lGLEW -lGL

RotatingCube: $(OBJ)
	 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

clean:
	rm -f *.o RotatingCube 
.PHONY: all clean
