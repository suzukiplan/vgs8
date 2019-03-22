// Copyright 2019, SUZUKI PLAN (GPLv3 license)
#include "vgs8.h"
using namespace VGS8;

VGS8::VirtualMachine(const void* rom, size_t size)
{
    this->cpu = new CPU(this, rom, size);
}
