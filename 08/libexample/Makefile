all: prog_naive prog_static prog2_naive prog2_static

HEADERS = $(wildcard *.h)

# compile the program that will link with the library
prog.o: prog.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

prog2.o: prog2.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

# compile the first library normally
example.o: example.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

# compile the second library normally
example2.o: example2.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

example2b.o: example2b.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

# make a static library for the first library
libexample.a: example.o
	ar -crs libexample.a $<

# make a static library of both .o files for the second library
libexample2.a: example2.o example2b.o
	ar -crs libexample2.a $^

# everything simply compiled together
prog_naive: prog.o example.o example2.o example2b.o
	gcc -o $@ $^
prog2_naive: prog2.o example.o example2.o example2b.o
	gcc -o $@ $^

# compiled together with static libraries
prog_static: prog.o libexample.a libexample2.a
	gcc -o $@ $< -L. -lexample -lexample2
prog2_static: prog2.o libexample.a libexample2.a
	gcc -o $@ $< -L. -lexample -lexample2

clean:
	rm -rf prog_* prog2_* *.o *.a
