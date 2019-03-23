all: vgs8.o vgs2tone.o vgsdec.o bin bin/romlink

clean:
	rm -rf *.o
	rm -rf bin

vgs8.o: src/vgs8.cpp src/vgs8.h src/apu.hpp src/bank.hpp src/cpu.hpp src/ppu.hpp
	clang -c src/vgs8.cpp

vgs2tone.o: src/vgs2tone.c
	clang -c src/vgs2tone.c

vgsdec.o: src/vgsdec.c
	clang -c src/vgsdec.c

bin:
	mkdir bin

bin/romlink: romlink.o vgsmml.o lz4.o miniz.o
	clang -o bin/romlink romlink.o vgsmml.o lz4.o miniz.o

romlink.o: tools/romlink.c
	clang -c tools/romlink.c

vgsmml.o: tools/vgs-mml-compiler/src/vgsmml.c
	clang -c tools/vgs-mml-compiler/src/vgsmml.c -I tools/vgs-mml-compiler/vgs-bgm-decoder/src

miniz.o: tools/vgs-mml-compiler/vgs-bgm-decoder/src/miniz.c
	clang -c tools/vgs-mml-compiler/vgs-bgm-decoder/src/miniz.c

lz4.o: tools/lz4/lib/lz4.c
	clang -c tools/lz4/lib/lz4.c
