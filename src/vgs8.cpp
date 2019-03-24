// Copyright 2019, SUZUKI PLAN (MIT license)
#include "vgs8.h"
using namespace VGS8;

VirtualMachine::VirtualMachine(const void* rom, size_t size)
{
    this->bank = new Bank(rom, size);
    this->cpu = new CPU(this);
    this->ppu = new PPU(this);
    this->apu = new APU(this);
    memset(keys, 0, sizeof(keys));
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

void VirtualMachine::setKey(int number,
                            bool up,
                            bool down,
                            bool left,
                            bool right,
                            bool a,
                            bool b,
                            bool select,
                            bool start)
{
    number &= 1;
    keys[number] = up ? 0x80 : 0;
    keys[number] |= down ? 0x40 : 0;
    keys[number] |= left ? 0x20 : 0;
    keys[number] |= right ? 0x10 : 0;
    keys[number] |= a ? 0x08 : 0;
    keys[number] |= b ? 0x04 : 0;
    keys[number] |= select ? 0x02 : 0;
    keys[number] |= start ? 0x01 : 0;
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

unsigned short* VirtualMachine::getDisplay565(size_t* size)
{
    if (!ppu) return NULL;
    int dp = 0;
    for (int y = 8; y < 248; y++) {
        int vp = (y << 8);
        for (int x = 8; x < 248; x++) {
            int cp = 0x5C00 + ppu->vram[vp] * 4;
            unsigned short r = cpu->ram[cp + 1] & 0b11111000;
            unsigned short g = cpu->ram[cp + 2] & 0b11111100;
            unsigned char b = cpu->ram[cp + 3] & 0b11111000;
            displayBuffer[dp] = r << 8;
            displayBuffer[dp] |= g << 3;
            displayBuffer[dp] |= b >> 3;
            vp++;
            dp++;
        }
    }
    *size = sizeof(displayBuffer);
    return displayBuffer;
}

unsigned short* VirtualMachine::getDisplay555(size_t* size)
{
    if (!ppu) return NULL;
    int dp = 0;
    for (int y = 8; y < 248; y++) {
        int vp = (y << 8);
        for (int x = 8; x < 248; x++) {
            int cp = 0x5C00 + ppu->vram[vp] * 4;
            unsigned short r = cpu->ram[cp + 1] & 0b11111000;
            unsigned short g = cpu->ram[cp + 2] & 0b11111000;
            unsigned char b = cpu->ram[cp + 3] & 0b11111000;
            displayBuffer[dp] = r << 7;
            displayBuffer[dp] |= g << 2;
            displayBuffer[dp] |= b >> 3;
            vp++;
            dp++;
        }
    }
    *size = sizeof(displayBuffer);
    return displayBuffer;
}

short* VirtualMachine::getPCM(size_t* size)
{
    if (!apu) return NULL;
    return apu->getBuffer(size);
}

void* VirtualMachine::save(size_t* size)
{
    if (!cpu) return NULL;
    savePtr = 0;
    savePtr += cpu->save(saveBuffer + savePtr);
    savePtr += ppu->save(saveBuffer + savePtr);
    savePtr += apu->save(saveBuffer + savePtr);
    *size = savePtr;
    return saveBuffer;
}

bool VirtualMachine::load(void* state, size_t size)
{
    if (!cpu) return false;
    if (!state || size < 1 || sizeof(saveBuffer) <= size) return false;
    memset(saveBuffer, 0, sizeof(saveBuffer));
    memcpy(saveBuffer, state, size);
    savePtr = 0;
    savePtr += cpu->load(saveBuffer + savePtr);
    savePtr += ppu->load(saveBuffer + savePtr);
    savePtr += apu->load(saveBuffer + savePtr);
    return true;
}

void VirtualMachine::_setChrBank(int cn, unsigned char bn)
{
    ppu->reg.cbank[cn & 1] = bn;
}

void VirtualMachine::_setChrMap(unsigned char n)
{
    ppu->reg.cmap = n;
}

void VirtualMachine::_setBgColor(unsigned char n)
{
    ppu->reg.bgC = n;
}

void VirtualMachine::_setBgX(unsigned char n)
{
    ppu->reg.bgX = n;
}

void VirtualMachine::_setBgY(unsigned char n)
{
    ppu->reg.bgY = n;
}

void VirtualMachine::_setFgX(unsigned char n)
{
    ppu->reg.fgX = n;
}

void VirtualMachine::_setFgY(unsigned char n)
{
    ppu->reg.fgY = n;
}