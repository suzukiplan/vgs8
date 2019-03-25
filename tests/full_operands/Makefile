all:
	ca65 test.asm
	ld65 --config mmap.cfg -o test.bin --obj test.o
	../../bin/romlink test.rom test.bin
	../../bin/vgsrun test.rom > result.txt
	@cat result.txt
