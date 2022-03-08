all: main.bin libmalloclog.so
	@:

run: all
	$(shell LD_PRELOAD=./libmalloclog.so ./main.bin)

%.o: %.c
	gcc -Wall -Wextra -fpic -I. -c -o $@ $<

libmalloclog.so: ml.o
	gcc -shared -o $@ $< -ldl

%.bin: %.o
	gcc -o $@ $<

clean:
	rm -rf main.bin libmalloclog.so *.o
