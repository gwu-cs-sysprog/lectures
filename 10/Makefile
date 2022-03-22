all:
	gcc -c sc.S -o sc.o
	ld -o asm_syscall sc.o
	gcc -o normal_syscall sc.c

clean:
	rm -rf asm_syscall normal_syscall *.o
