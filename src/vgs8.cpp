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
    apu = NULL;
    delete ppu;
    ppu = NULL;
    delete cpu;
    cpu = NULL;
    delete bank;
    bank = NULL;
}

void VirtualMachine::reset()
{
    cpu->reset();
    ppu->reset();
    apu->reset();
}

void VirtualMachine::tick()
{
    if (!cpu) return;
    cpu->execute(); // VRAM update request が発生するまでの間CPUを回す
    ppu->execute(); // VRAMを更新
    apu->execute(); // 音声を更新
}

void* VirtualMachine::save(size_t* size)
{
    savePtr = 0;
    savePtr += cpu->save(saveBuffer + savePtr);
    savePtr += ppu->save(saveBuffer + savePtr);
    savePtr += apu->save(saveBuffer + savePtr);
    *size = savePtr;
    return saveBuffer;
}

bool VirtualMachine::load(void* state, size_t size)
{
    if (!state || size < 1 || sizeof(saveBuffer) <= size) return false;
    memset(saveBuffer, 0, sizeof(saveBuffer));
    memcpy(saveBuffer, state, size);
    savePtr = 0;
    savePtr += cpu->load(saveBuffer + savePtr);
    savePtr += ppu->load(saveBuffer + savePtr);
    savePtr += apu->load(saveBuffer + savePtr);
    return true;
}

void VirtualMachine::setChrBank(int cn, unsigned char bn)
{
    ppu->reg.cbank[cn & 1] = bn;
}

void VirtualMachine::setChrMap(unsigned char n)
{
    ppu->reg.cmap = n;
}