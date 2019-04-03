all:
	ca65 init.asm
	ld65 --config mmap.cfg -o init.bin --obj init.o
	../../bin/romlink init.rom init.bin font.bmp

clean:
	rm -rf *.o
	rm -rf *.bin
	rm -rf *.rom
