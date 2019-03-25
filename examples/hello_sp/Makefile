all:
	ca65 hello.asm
	ld65 --config mmap.cfg -o hello.bin --obj hello.o
	../../bin/romlink hello.rom hello.bin bank000.bmp bank001.bmp

clean:
	rm -rf hello.o
	rm -rf hello.bin
	rm -rf hello.rom
