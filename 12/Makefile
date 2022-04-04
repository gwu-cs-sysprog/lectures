BINS=$(patsubst %.c,%,$(wildcard *.c))

all: $(BINS)

%: %.c
	gcc -o $@ $<

clean:
	rm -rf $(BINS) ./AAAAA* ./cat
