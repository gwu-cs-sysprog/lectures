all: naive static dynamic

naive: prog_naive  prog2_naive

static: prog_static prog2_static

dynamic: prog_dynamic prog2_dynamic

HEADERS = $(wildcard *.h)

# compile the program that will link with the library
prog.o: prog.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

prog2.o: prog2.c $(HEADERS)
	gcc -Wall -Wextra -I. -c -o $@ $<

# compile the first library normally
example.o: example.c $(HEADERS)
	gcc -Wall -Wextra -fpic -I. -c -o $@ $<
# compile the second library normally
example2.o: example2.c $(HEADERS)
	gcc -Wall -Wextra -fpic -I. -c -o $@ $<
example2b.o: example2b.c $(HEADERS)
	gcc -Wall -Wextra -fpic -I. -c -o $@ $<

# make a static library for the first library
libexample.a: example.o
	ar -crs libexample.a $<
# make a static library of both .o files for the second library
libexample2.a: example2.o example2b.o
	ar -crs libexample2.a $^

# make a dynamic library for the first library
libexample.so: example.o
	gcc -shared -o $@ $<
# make a dynamic library of both .o files for the second library
libexample2.so: example2.o example2b.o
	gcc -shared -o $@ $^

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

# compiled together with static libraries
prog_dynamic: prog.o libexample.so libexample2.so
	gcc -o $@ $< -L. -lexample -lexample2
prog2_dynamic: prog2.o libexample.so libexample2.so
	gcc -o $@ $< -L. -lexample -lexample2

clean:
	rm -rf prog_* prog2_* *.o *.a *.so
