// Copyright 2019, SUZUKI PLAN (GPLv3 license)
#include "vgs8.h"
using namespace VGS8;

VirtualMachine::VirtualMachine(const void* rom, size_t size)
{
    this->bank = new Bank(rom, size);
    this->cpu = new CPU(this);
    this->ppu = new PPU(this);
    this->apu = new APU(this);
}

VirtualMachine::~VirtualMachine()
{
    delete apu;
    delete ppu;
    delete cpu;
    delete bank;
}