// Copyright 2019, SUZUKI PLAN (GPLv3 license)
class VGS8::VirtualMachine;

class Register
{
  public:
    char a;
    unsigned char x;
    unsigned char y;
    unsigned char p;
    unsigned char s;
    unsigned short pc;
};

class CPU
{
  private:
    VGS8::VirtualMachine* vm;

    inline void checkLD(unsigned short addr, unsigned char value)
    {
        // TODO: 以下のようにI/Oポートの場合はPPUへload
        switch (addr) {
            case 0x5400: vm->ppu->load(addr, value); break;
            case 0x5401: vm->ppu->load(addr, value); break;
        }
    }

    inline void lda_immediate()
    {
        reg.a = ram[++reg.pc];  // a = immediate value
        reg.p |= reg.a & 0x80;  // set negative
        reg.p |= reg.a ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

    inline void lda_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];   // calculate address
        reg.a = ram[addr];      // a = zero page value
        reg.p |= reg.a & 0x80;  // set negative
        reg.p |= reg.a ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 3;            // tick the clock
        checkLD(addr, reg.a);   // I/O check
    }

    inline void lda_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x; // calculate address
        reg.a = ram[addr];            // a = zero page value
        reg.p |= reg.a & 0x80;        // set negative
        reg.p |= reg.a ? 2 : 0;       // set zero
        reg.pc++;                     // increment pc
        clocks += 4;                  // tick the clock
        checkLD(addr, reg.a);         // I/O check
    }

    inline void lda_absolute()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (HIGH)
        addr <<= 8;                          // addr *= 256
        addr |= ram[++reg.pc];               // get addr (LOW)
        reg.a = ram[addr];                   // a = any page value
        reg.p |= reg.a & 0x80;               // set negative
        reg.p |= reg.a ? 2 : 0;              // set zero
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkLD(addr, reg.a);                // I/O check
    }

    inline void lda_absolute_x()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (LOW)
        addr |= ram[++reg.pc] * 256;         // get addr (HIGH)
        reg.a = ram[addr + reg.x];           // a = any page value
        reg.p |= reg.a & 0x80;               // set negative
        reg.p |= reg.a ? 2 : 0;              // set zero
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkLD(addr, reg.a);                // I/O check
    }

    inline void lda_absolute_y()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (LOW)
        addr |= ram[++reg.pc] * 256;         // get addr (HIGH)
        reg.a = ram[addr + reg.y];           // a = any page value
        reg.p |= reg.a & 0x80;               // set negative
        reg.p |= reg.a ? 2 : 0;              // set zero
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkLD(addr, reg.a);                // I/O check
    }

    inline void lda_indirect_x()
    {
        unsigned short ptr = ram[++reg.pc] + reg.x;
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        reg.a = ram[addr];                // a = any page value
        reg.p |= reg.a & 0x80;            // set negative
        reg.p |= reg.a ? 2 : 0;           // set zero
        reg.pc++;                         // increment pc
        clocks += 6;                      // tick the clock
        checkLD(addr, reg.a);             // I/O check
    }

    inline void lda_indirect_y()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        reg.a = ram[addr + reg.y];        // a = any page value
        reg.p |= reg.a & 0x80;            // set negative
        reg.p |= reg.a ? 2 : 0;           // set zero
        reg.pc++;                         // increment pc
        clocks += 5;                      // tick the clock
        checkLD(addr, reg.a);             // I/O check
    }

    inline void tax()
    {
        reg.x = reg.a;          // transfer a to x
        reg.p |= reg.x & 0x80;  // set negative
        reg.p |= reg.x ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

    inline void txa()
    {
        reg.a = reg.x;          // transfer x to a
        reg.p |= reg.a & 0x80;  // set negative
        reg.p |= reg.a ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

  public:
    unsigned int clocks;
    unsigned char ram[65536];
    Register reg;

    CPU(VGS8::VirtualMachine* vm, const void* rom, size_t size)
    {
        this->vm = vm;
        memset(ram, 0, sizeof(ram));
        memset(&reg, 0, sizeof(reg));
        // TODO: ROM mapping
        reg.pc = 0x8000;
    }

    inline void execute()
    {
        // TODO: デシマルモードを除く6502の全命令を実装予定
        switch (ram[reg.pc]) {
            case 0x8A: txa(); break;
            case 0xAA: tax(); break;
            case 0xA5: lda_zero(); break;
            case 0xA9: lda_immediate(); break;
            case 0xAD: lda_absolute(); break;
            case 0xB5: lda_zero_x(); break;
            case 0xBD: lda_absolute_x(); break;
            case 0xB9: lda_absolute_y(); break;
            case 0xA1: lda_indirect_x(); break;
            case 0xB1: lda_indirect_y(); break;
        }
    }
};
