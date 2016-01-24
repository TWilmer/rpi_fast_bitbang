all: blinker02.bin mailbox step1.bin
blinker02.bin: blinker02.s build.c ./tinyasm/asm.c
	gcc build.c -o build
	./build
step1.bin: step1.s build.c ./tinyasm/asm.c
	gcc build.c -o step1
	./step1

mailbox: mailbox.c
	gcc -O2 -o mailbox mailbox.c
clean:
	rm blinker02.bin build mailbox
