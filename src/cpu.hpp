// Copyright 2019, SUZUKI PLAN (GPLv3 license)

class Register
{
  public:
    // 6502標準レジスタ
    char a;
    unsigned char x;
    unsigned char y;
    unsigned char p;
    unsigned char s;
    unsigned short pc;
    // VGS8専用レジスタ
    unsigned char prg8000;
    unsigned char prgC000;
};

class CPU
{
  private:
    VGS8::VirtualMachine* vm;

    inline void checkLDA(unsigned short addr, unsigned char value)
    {
        switch (addr) {
            case 0x5400: reg.a = reg.prg8000; break;
            case 0x5401: reg.a = reg.prgC000; break;
        }
    }

    inline void checkSTA(unsigned short addr, unsigned char value)
    {
        switch (addr) {
            case 0x5400: changeProgramBank8000(reg.a); break;
            case 0x5401: changeProgramBankC000(reg.a); break;
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
        checkLDA(addr, reg.a);  // I/O check
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
        checkLDA(addr, reg.a);        // I/O check
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
        checkLDA(addr, reg.a);               // I/O check
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
        checkLDA(addr, reg.a);               // I/O check
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
        checkLDA(addr, reg.a);               // I/O check
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
        checkLDA(addr, reg.a);            // I/O check
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
        checkLDA(addr, reg.a);            // I/O check
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

    void changeProgramBank8000(unsigned char n)
    {
        reg.prg8000 = n;
        if (vm->bank && vm->bank->prg[n]) {
            memcpy(&ram[0x8000], vm->bank->prg[n], 0x4000);
        } else {
            memset(&ram[0x8000], 0, 0x4000);
        }
    }

    void changeProgramBankC000(unsigned char n)
    {
        reg.prgC000 = n;
        if (vm->bank && vm->bank->prg[n]) {
            memcpy(&ram[0xC000], vm->bank->prg[n], 0x4000);
        } else {
            memset(&ram[0xC000], 0, 0x4000);
        }
    }

  public:
    unsigned int clocks;
    unsigned char ram[65536];
    VGS8::Register reg;

    CPU(VGS8::VirtualMachine* vm)
    {
        this->vm = vm;
        memset(ram, 0, sizeof(ram));
        memset(&reg, 0, sizeof(reg));
        reset();
    }

    void reset()
    {
        reg.pc = 0x8000;
        changeProgramBank8000(0);
        changeProgramBankC000(1);
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
