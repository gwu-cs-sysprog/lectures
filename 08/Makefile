all: prog

# compile the program that will link with the library
prog.o: prog.c example.h
	gcc -Wall -Wextra -I. -c -o $@ $<

# compile the library normally
example.o: example.c example.h
	gcc -Wall -Wextra -I. -c -o $@ $<

# make a static library
libexample.a: example.o
	ar -crs libexample.a $<

prog: prog.o libexample.a
	gcc -o $@ $< -L. -lexample

clean:
	rm -rf prog *.o *.a
