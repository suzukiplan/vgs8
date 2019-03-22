all: vgs8.o

vgs8.o: src/vgs8.cpp src/vgs8.h src/apu.hpp src/bank.hpp src/cpu.hpp src/ppu.hpp
	clang -c src/vgs8.cpp
