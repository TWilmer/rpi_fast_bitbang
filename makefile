all: blinker02.bin mailbox
blinker02.bin: blinker02.s build.c ./tinyasm/asm.c
	gcc build.c -o build
	./build

mailbox: mailbox.c
	gcc -O2 -o mailbox mailbox.c
clean:
	rm blinker02.bin build mailbox
