// Copyright 2019, SUZUKI PLAN (GPLv3 license)

class CPU
{
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

  private:
    VGS8::VirtualMachine* vm;
    Register reg;
    bool vramUpdateRequest;

    inline void checkLD(unsigned short addr, unsigned char* value)
    {
        switch (addr) {
            case 0x5405: vramUpdateRequest = true; break;
        }
    }

    inline void checkST(unsigned short addr, unsigned char value)
    {
        switch (addr) {
            case 0x5400: changeProgramBank8000(value); break;
            case 0x5401: changeProgramBankC000(value); break;
            case 0x5402: vm->setChrBank(0, value); break;
            case 0x5403: vm->setChrBank(0, value); break;
            case 0x5404: vm->setChrMap(value); break;
            case 0x5405: vramUpdateRequest = true; break;
        }
    }

    inline void updateNZ(unsigned char value)
    {
        reg.p &= 0b01111101;    // clear N, Z
        reg.p |= value & 0x80;  // set negative
        reg.p |= value ? 2 : 0; // set zero
    }

    inline void updateCNZ(int value)
    {
        reg.p &= 0xFE;                       // clear carry
        reg.p |= value & 0xFFFFFF00 ? 1 : 0; // set carry
        reg.a = value & 0xFF;                // set result
        updateNZ(reg.a);                     // update negative and zero
    }

    inline unsigned short absolute()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (HIGH)
        addr <<= 8;                          // addr *= 256
        addr |= ram[++reg.pc];               // get addr (LOW)
        return addr;
    }

    inline unsigned short absolute(unsigned char i)
    {
        unsigned short addr = absolute(); // get absolute address
        addr += i;                        // plus index
        return addr;
    }

    inline unsigned short indirect()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        return addr;
    }

    inline unsigned short indirectX()
    {
        unsigned short ptr = ram[++reg.pc] + reg.x;
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        return addr;
    }

    inline unsigned short indirectY()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        addr += reg.y;
        return addr;
    }

    inline void lda_immediate()
    {
        reg.a = ram[++reg.pc]; // a = immediate value
        updateNZ(reg.a);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
    }

    inline void ldx_immediate()
    {
        reg.x = ram[++reg.pc]; // x = immediate value
        updateNZ(reg.x);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
    }

    inline void ldy_immediate()
    {
        reg.y = ram[++reg.pc]; // y = immediate value
        updateNZ(reg.y);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
    }

    inline void lda_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.a = ram[addr];                     // a = zero page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.x = ram[addr];                     // x = zero page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.y = ram[addr];                     // y = zero page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                // calculate address
        ram[addr] = reg.a;                   // store a to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                // calculate address
        ram[addr] = reg.x;                   // store x to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                // calculate address
        ram[addr] = reg.y;                   // store y to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;          // calculate address
        reg.a = ram[addr];                     // a = zero page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero_y()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.y;          // calculate address
        reg.x = ram[addr];                     // x = zero page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;          // calculate address
        reg.y = ram[addr];                     // y = zero page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;        // calculate address
        ram[addr] = reg.a;                   // store a to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_zero_y()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.y;        // calculate address
        ram[addr] = reg.x;                   // store x to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;        // calculate address
        ram[addr] = reg.y;                   // store y to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_absolute()
    {
        unsigned short addr = absolute();      // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_absolute()
    {
        unsigned short addr = absolute();      // get absolute address
        reg.x = ram[addr];                     // x = any page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute()
    {
        unsigned short addr = absolute();      // get absolute address
        reg.y = ram[addr];                     // y = any page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_absolute()
    {
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_absolute()
    {
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.x;                   // store x to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_absolute()
    {
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.y;                   // store y to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_absolute_x()
    {
        unsigned short addr = absolute(reg.x); // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_absolute_y()
    {
        unsigned short addr = absolute(reg.y); // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void sta_absolute_x()
    {
        unsigned short addr = absolute(reg.x); // get absolute address
        ram[addr] = reg.a;                     // store a to any page
        reg.pc++;                              // increment pc
        clocks += 5;                           // tick the clock
        checkST(addr, (unsigned char)reg.a);   // I/O check
    }

    inline void sta_absolute_y()
    {
        unsigned short addr = absolute(reg.y); // get absolute address
        ram[addr] = reg.a;                     // store a to any page
        reg.pc++;                              // increment pc
        clocks += 5;                           // tick the clock
        checkST(addr, (unsigned char)reg.a);   // I/O check
    }

    inline void ldx_absolute_y()
    {
        unsigned short addr = absolute(reg.y); // get absolute address
        reg.x = ram[addr];                     // x = any page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute_x()
    {
        unsigned short addr = absolute(reg.x); // get absolute address
        reg.y = ram[addr];                     // y = any page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_indirect_x()
    {
        unsigned short addr = indirectX();     // get indirectX address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 6;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_indirect_y()
    {
        unsigned short addr = indirectY();     // get indirectY address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 5;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void sta_indirect_x()
    {
        unsigned short addr = indirectX();   // get indirectX address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 6;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void sta_indirect_y()
    {
        unsigned short addr = indirectY();   // get indirectY address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 5;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void tax()
    {
        reg.x = reg.a;   // transfer a to x
        updateNZ(reg.x); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void txa()
    {
        reg.a = reg.x;   // transfer x to a
        updateNZ(reg.a); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tay()
    {
        reg.y = reg.a;   // transfer a to y
        updateNZ(reg.y); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tya()
    {
        reg.a = reg.y;   // transfer y to a
        updateNZ(reg.a); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tsx()
    {
        reg.x = reg.s;   // transfer s to x
        updateNZ(reg.x); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void txs()
    {
        reg.s = reg.x;   // transfer x to s
        updateNZ(reg.s); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void adc(unsigned char v)
    {
        int a = (unsigned char)reg.a;
        reg.p &= 0xBF; // clear overflow
        unsigned char prev = v & 0x80;
        if ((a & 0x80) == prev) {
            a += v + (reg.p & 1);
            if ((a & 0x80) != prev) {
                reg.p |= 0x40; // set overflow
            }
        } else {
            a += v + (reg.p & 1);
        }
        updateCNZ(a); // update p and a
    }

    inline void adc_immediate()
    {
        adc(ram[++reg.pc]); // add with carry
        reg.pc++;           // increment pc
        clocks += 2;        // tick the clock
    }

    inline void adc_zero()
    {
        adc(ram[ram[++reg.pc]]); // add with carry
        reg.pc++;                // increment pc
        clocks += 3;             // tick the clock
    }

    inline void adc_zero_x()
    {
        adc(ram[ram[++reg.pc] + reg.x]); // add with carry
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void adc_absolute()
    {
        adc(ram[absolute()]); // add with carry
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void adc_absolute_x()
    {
        adc(ram[absolute(reg.x)]); // add with carry
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void adc_absolute_y()
    {
        adc(ram[absolute(reg.y)]); // add with carry
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void adc_indirect_x()
    {
        adc(ram[indirectX()]); // add with carry
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void adc_indirect_y()
    {
        adc(ram[indirectY()]); // add with carry
        reg.pc++;              // increment pc
        clocks += 5;           // tick the clock
    }

    inline void sbc(unsigned char v)
    {
        int a = (unsigned char)reg.a;
        reg.p &= 0xBF; // clear overflow
        unsigned char prev = v & 0x80;
        if ((a & 0x80) == prev) {
            a -= v + (reg.p & 1 ? 0 : 1);
            if ((a & 0x80) != prev) {
                reg.p |= 0x40; // set overflow
            }
        } else {
            a -= v + (reg.p & 1 ? 0 : 1);
        }
        updateCNZ(a); // update p and a
    }

    inline void sbc_immediate()
    {
        sbc(ram[++reg.pc]); // sub with carry
        reg.pc++;           // increment pc
        clocks += 2;        // tick the clock
    }

    inline void sbc_zero()
    {
        sbc(ram[ram[++reg.pc]]); // sub with carry
        reg.pc++;                // increment pc
        clocks += 3;             // tick the clock
    }

    inline void sbc_zero_x()
    {
        sbc(ram[ram[++reg.pc] + reg.x]); // sub with carry
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void sbc_absolute()
    {
        sbc(ram[absolute()]); // sub with carry
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void sbc_absolute_x()
    {
        sbc(ram[absolute(reg.x)]); // sub with carry
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void sbc_absolute_y()
    {
        sbc(ram[absolute(reg.y)]); // sub with carry
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void sbc_indirect_x()
    {
        sbc(ram[indirectX()]); // sub with carry
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void sbc_indirect_y()
    {
        sbc(ram[indirectY()]); // sub with carry
        reg.pc++;              // increment pc
        clocks += 5;           // tick the clock
    }

    inline void andA(unsigned char v)
    {
        reg.a &= v;
        updateNZ(reg.a);
    }

    inline void and_immediate()
    {
        andA(ram[++reg.pc]); // and
        reg.pc++;            // increment pc
        clocks += 2;         // tick the clock
    }

    inline void and_zero()
    {
        andA(ram[ram[++reg.pc]]); // and
        reg.pc++;                 // increment pc
        clocks += 3;              // tick the clock
    }

    inline void and_zero_x()
    {
        andA(ram[ram[++reg.pc] + reg.x]); // and
        reg.pc++;                         // increment pc
        clocks += 4;                      // tick the clock
    }

    inline void and_absolute()
    {
        andA(ram[absolute()]); // and
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void and_absolute_x()
    {
        andA(absolute(reg.x)); // and
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void and_absolute_y()
    {
        andA(absolute(reg.y)); // and
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void and_indirect_x()
    {
        andA(ram[indirectX()]); // and
        reg.pc++;               // increment pc
        clocks += 6;            // tick the clock
    }

    inline void and_indirect_y()
    {
        andA(ram[indirectY()]); // and
        reg.pc++;               // increment pc
        clocks += 5;            // tick the clock
    }

    inline void ora(unsigned char v)
    {
        reg.a |= v;
        updateNZ(reg.a);
    }

    inline void ora_immediate()
    {
        ora(ram[++reg.pc]); // or
        reg.pc++;           // increment pc
        clocks += 2;        // tick the clock
    }

    inline void ora_zero()
    {
        ora(ram[ram[++reg.pc]]); // or
        reg.pc++;                // increment pc
        clocks += 3;             // tick the clock
    }

    inline void ora_zero_x()
    {
        ora(ram[ram[++reg.pc] + reg.x]); // or
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void ora_absolute()
    {
        ora(ram[absolute()]); // or
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void ora_absolute_x()
    {
        ora(ram[absolute(reg.x)]); // or
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void ora_absolute_y()
    {
        ora(ram[absolute(reg.y)]); // or
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void ora_indirect_x()
    {
        ora(ram[indirectX()]); // or
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void ora_indirect_y()
    {
        ora(ram[indirectY()]); // or
        reg.pc++;              // increment pc
        clocks += 5;           // tick the clock
    }

    inline void eor(unsigned char v)
    {
        reg.a ^= v;
        updateNZ(reg.a);
    }

    inline void eor_immediate()
    {
        eor(ram[++reg.pc]); // eor
        reg.pc++;           // increment pc
        clocks += 2;        // tick the clock
    }

    inline void eor_zero()
    {
        eor(ram[ram[++reg.pc]]); // eor
        reg.pc++;                // increment pc
        clocks += 3;             // tick the clock
    }

    inline void eor_zero_x()
    {
        eor(ram[ram[++reg.pc] + reg.x]); // eor
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void eor_absolute()
    {
        eor(ram[absolute()]); // eor
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void eor_absolute_x()
    {
        eor(ram[absolute(reg.x)]); // eor
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void eor_absolute_y()
    {
        eor(ram[absolute(reg.y)]); // eor
        reg.pc++;                  // increment pc
        clocks += 4;               // tick the clock
    }

    inline void eor_indirect_x()
    {
        eor(ram[indirectX()]); // eor
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void eor_indirect_y()
    {
        eor(ram[indirectY()]); // eor
        reg.pc++;              // increment pc
        clocks += 5;           // tick the clock
    }

    inline void compare(unsigned char t, unsigned char v)
    {
        reg.p &= 0b01111100;      // clear N, Z, C
        int r = t - (int)v;       // calcurate
        if (r < 0) reg.p |= 0x80; // set N if negative
        if (0 == r) reg.p |= 2;   // set Z if zero
        if (t >= v) reg.p |= 1;   // set C
    }

    inline void cmp_immediate()
    {
        compare(reg.a, ram[++reg.pc]); // compare
        reg.pc++;                      // increment pc
        clocks += 2;                   // tick the clock
    }

    inline void cmp_zero()
    {
        compare(reg.a, ram[ram[++reg.pc]]); // comapre
        reg.pc++;                           // increment pc
        clocks += 3;                        // tick the clock
    }

    inline void cmp_zero_x()
    {
        compare(reg.a, ram[ram[++reg.pc] + reg.x]); // compare
        reg.pc++;                                   // increment pc
        clocks += 4;                                // tick the clock
    }

    inline void cmp_absolute()
    {
        compare(reg.a, ram[absolute()]); // compare
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void cmp_absolute_x()
    {
        compare(reg.a, ram[absolute(reg.x)]); // compare
        reg.pc++;                             // increment pc
        clocks += 4;                          // tick the clock
    }

    inline void cmp_absolute_y()
    {
        compare(reg.a, ram[absolute(reg.y)]); // compare
        reg.pc++;                             // increment pc
        clocks += 4;                          // tick the clock
    }

    inline void cmp_indirect_x()
    {
        compare(reg.a, ram[indirectX()]); // compare
        reg.pc++;                         // increment pc
        clocks += 6;                      // tick the clock
    }

    inline void cmp_indirect_y()
    {
        compare(reg.a, ram[indirectY()]); // compare
        reg.pc++;                         // increment pc
        clocks += 5;                      // tick the clock
    }

    inline void cpx_immediate()
    {
        compare(reg.x, ram[++reg.pc]); // compare
        reg.pc++;                      // increment pc
        clocks += 2;                   // tick the clock
    }

    inline void cpx_zero()
    {
        compare(reg.x, ram[ram[++reg.pc]]); // comapre
        reg.pc++;                           // increment pc
        clocks += 3;                        // tick the clock
    }

    inline void cpx_absolute()
    {
        compare(reg.x, ram[absolute()]); // compare
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void cpy_immediate()
    {
        compare(reg.y, ram[++reg.pc]); // compare
        reg.pc++;                      // increment pc
        clocks += 2;                   // tick the clock
    }

    inline void cpy_zero()
    {
        compare(reg.y, ram[ram[++reg.pc]]); // comapre
        reg.pc++;                           // increment pc
        clocks += 3;                        // tick the clock
    }

    inline void cpy_absolute()
    {
        compare(reg.y, ram[absolute()]); // compare
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void shift(bool isLeft, unsigned char* v)
    {
        reg.p &= 0b01111100; // clear N, Z, C
        int a = *v;
        if (isLeft) {
            a <<= 1;
        } else {
            a >>= 1;
        }
        *v = a & 0xFF;                  // store to result
        if (a & 0x80) reg.p |= 0x80;    // set N if negative
        if (0 == *v) reg.p |= 2;        // set Z if zero
        if (a & 0xFFFFFF00) reg.p |= 1; // set carry
    }

    inline void asl_a()
    {
        shift(true, (unsigned char*)&reg.a); // left shift
        reg.pc++;                            // increment pc
        clocks += 2;                         // tick the clock
    }

    inline void asl_zero()
    {
        shift(true, &ram[ram[++reg.pc]]); // left shift
        reg.pc++;                         // increment pc
        clocks += 5;                      // tick the clock
    }

    inline void asl_zero_x()
    {
        shift(true, &ram[ram[++reg.pc] + reg.x]); // left shift
        reg.pc++;                                 // increment pc
        clocks += 6;                              // tick the clock
    }

    inline void asl_absolute()
    {
        shift(true, &ram[absolute()]); // left shift
        reg.pc++;                      // increment pc
        clocks += 6;                   // tick the clock
    }

    inline void asl_absolute_x()
    {
        shift(true, &ram[absolute(reg.x)]); // left shift
        reg.pc++;                           // increment pc
        clocks += 7;                        // tick the clock
    }

    inline void lsr_a()
    {
        shift(false, (unsigned char*)&reg.a); // right shift
        reg.pc++;                             // increment pc
        clocks += 2;                          // tick the clock
    }

    inline void lsr_zero()
    {
        shift(false, &ram[ram[++reg.pc]]); // right shift
        reg.pc++;                          // increment pc
        clocks += 5;                       // tick the clock
    }

    inline void lsr_zero_x()
    {
        shift(false, &ram[ram[++reg.pc] + reg.x]); // right shift
        reg.pc++;                                  // increment pc
        clocks += 6;                               // tick the clock
    }

    inline void lsr_absolute()
    {
        shift(false, &ram[absolute()]); // right shift
        reg.pc++;                       // increment pc
        clocks += 6;                    // tick the clock
    }

    inline void lsr_absolute_x()
    {
        shift(false, &ram[absolute(reg.x)]); // right shift
        reg.pc++;                            // increment pc
        clocks += 7;                         // tick the clock
    }

    inline void rotate(bool isLeft, unsigned char* v)
    {
        reg.p &= 0b01111100; // clear N, Z, C
        int a = *v;
        unsigned char lsb = a & 1;
        unsigned char msb = a & 0x80;
        if (isLeft) {
            a <<= 1;           // left shift
            *v = a & 0xFE;     // store to result without LSB
            *v |= msb ? 1 : 0; // store LSB if previous MSB has set
        } else {
            a >>= 1;              // right shift
            *v = a & 0x7F;        // store to result without MSB
            *v |= lsb ? 0x80 : 0; // store MSB if previous LSB has set
        }
        if (a & 0x80) reg.p |= 0x80;    // set N if negative
        if (0 == *v) reg.p |= 2;        // set Z if zero
        if (a & 0xFFFFFF00) reg.p |= 1; // set carry
    }

    inline void rol_a()
    {
        rotate(true, (unsigned char*)&reg.a); // left rotate
        reg.pc++;                             // increment pc
        clocks += 2;                          // tick the clock
    }

    inline void rol_zero()
    {
        rotate(true, &ram[ram[++reg.pc]]); // left rotate
        reg.pc++;                          // increment pc
        clocks += 5;                       // tick the clock
    }

    inline void rol_zero_x()
    {
        rotate(true, &ram[ram[++reg.pc] + reg.x]); // left rotate
        reg.pc++;                                  // increment pc
        clocks += 6;                               // tick the clock
    }

    inline void rol_absolute()
    {
        rotate(true, &ram[absolute()]); // left rotate
        reg.pc++;                       // increment pc
        clocks += 6;                    // tick the clock
    }

    inline void rol_absolute_x()
    {
        rotate(true, &ram[absolute(reg.x)]); // left rotate
        reg.pc++;                            // increment pc
        clocks += 7;                         // tick the clock
    }

    inline void ror_a()
    {
        rotate(false, (unsigned char*)&reg.a); // right rotate
        reg.pc++;                              // increment pc
        clocks += 2;                           // tick the clock
    }

    inline void ror_zero()
    {
        rotate(false, &ram[ram[++reg.pc]]); // right rotate
        reg.pc++;                           // increment pc
        clocks += 5;                        // tick the clock
    }

    inline void ror_zero_x()
    {
        rotate(false, &ram[ram[++reg.pc] + reg.x]); // right rotate
        reg.pc++;                                   // increment pc
        clocks += 6;                                // tick the clock
    }

    inline void ror_absolute()
    {
        rotate(false, &ram[absolute()]); // right rotate
        reg.pc++;                        // increment pc
        clocks += 6;                     // tick the clock
    }

    inline void ror_absolute_x()
    {
        rotate(false, &ram[absolute(reg.x)]); // right rotate
        reg.pc++;                             // increment pc
        clocks += 7;                          // tick the clock
    }

    inline void inc(unsigned char* v)
    {
        (*v)++;
        updateNZ(*v);
    }

    inline void inc_zero()
    {
        inc(&ram[ram[++reg.pc]]); // increment
        reg.pc++;                 // increment pc
        clocks += 5;              // tick the clock
    }

    inline void inc_zero_x()
    {
        inc(&ram[ram[++reg.pc] + reg.x]); // increment
        reg.pc++;                         // increment pc
        clocks += 6;                      // tick the clock
    }

    inline void inc_absolute()
    {
        inc(&ram[absolute()]); // increment
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void inc_absolute_x()
    {
        inc(&ram[absolute(reg.x)]); // increment
        reg.pc++;                   // increment pc
        clocks += 7;                // tick the clock
    }

    inline void inx()
    {
        inc(&reg.x); // increment
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void iny()
    {
        inc(&reg.y); // increment
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void dec(unsigned char* v)
    {
        (*v)--;
        updateNZ(*v);
    }

    inline void dec_zero()
    {
        dec(&ram[ram[++reg.pc]]); // decrement
        reg.pc++;                 // increment pc
        clocks += 5;              // tick the clock
    }

    inline void dec_zero_x()
    {
        dec(&ram[ram[++reg.pc] + reg.x]); // decrement
        reg.pc++;                         // increment pc
        clocks += 6;                      // tick the clock
    }

    inline void dec_absolute()
    {
        dec(&ram[absolute()]); // decrement
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void dec_absolute_x()
    {
        dec(&ram[absolute(reg.x)]); // decrement
        reg.pc++;                   // increment pc
        clocks += 7;                // tick the clock
    }

    inline void dex()
    {
        dec(&reg.x); // decrement
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void dey()
    {
        dec(&reg.y); // decrement
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void bit(unsigned char v)
    {
        reg.p &= 0b00111101;        // clear N, V, Z
        reg.p |= v & 0xC0;          // set negative and overflow
        reg.p |= v & reg.a ? 2 : 0; // set zero
    }

    inline void bit_zero()
    {
        bit(ram[ram[++reg.pc]]); // update p
        reg.pc++;                // increment pc
        clocks += 3;             // tick the clock
    }

    inline void bit_absolute()
    {
        bit(ram[absolute()]); // update p
        reg.pc++;             // increment pc
        clocks += 3;          // tick the clock
    }

    inline void push(unsigned char v)
    {
        ram[0x100 + reg.s] = v;
        reg.s--;
    }

    inline unsigned char pull()
    {
        reg.s++;
        return ram[0x100 + reg.s];
    }

    inline void pha()
    {
        push(reg.a);
        reg.pc++;
        clocks += 3;
    }

    inline void pla()
    {
        reg.a = pull();
        reg.pc++;
        clocks += 4;
    }

    inline void php()
    {
        push(reg.p);
        reg.pc++;
        clocks += 3;
    }

    inline void plp()
    {
        reg.p = pull();
        reg.pc++;
        clocks += 4;
    }

    inline void jmp_absolute()
    {
        reg.pc = absolute();
        clocks += 3;
    }

    inline void jmp_indirect()
    {
        reg.pc = indirect();
        clocks += 5;
    }

    inline void jsr()
    {
        unsigned short to = absolute();     // get jump to address
        push(((unsigned char*)&reg.pc)[0]); // push return address minus 1 of H
        push(((unsigned char*)&reg.pc)[1]); // push return address minus 1 of L
        reg.pc = to;                        // change program counter (jump to)
        clocks += 6;                        // tick the clock
    }

    inline void rts()
    {
        ((unsigned char*)&reg.pc)[1] = pull(); // pull return address of L
        ((unsigned char*)&reg.pc)[0] = pull(); // pull return address of H
        reg.pc++;                              // increment program counter
        clocks += 6;                           // tick the clock
    }

    inline void rti()
    {
        reg.p = pull();                        // pull p
        ((unsigned char*)&reg.pc)[1] = pull(); // pull return address of L
        ((unsigned char*)&reg.pc)[0] = pull(); // pull return address of H
        clocks += 6;                           // tick the clock
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

    void execute()
    {
        // TODO: デシマルモードを除く6502の全命令を実装予定
        vramUpdateRequest = false;
        while (!vramUpdateRequest) {
            switch (ram[reg.pc]) {
                // TRANSFER
                case 0xAA: tax(); break;
                case 0xA8: tay(); break;
                case 0xBA: tsx(); break;
                case 0x8A: txa(); break;
                case 0x9A: txs(); break;
                case 0x98: tya(); break;
                // LDA
                case 0xA9: lda_immediate(); break;
                case 0xA5: lda_zero(); break;
                case 0xB5: lda_zero_x(); break;
                case 0xAD: lda_absolute(); break;
                case 0xBD: lda_absolute_x(); break;
                case 0xB9: lda_absolute_y(); break;
                case 0xA1: lda_indirect_x(); break;
                case 0xB1: lda_indirect_y(); break;
                // LDX
                case 0xA2: ldx_immediate(); break;
                case 0xA6: ldx_zero(); break;
                case 0xB6: ldx_zero_y(); break;
                case 0xAE: ldx_absolute(); break;
                case 0xBE: ldx_absolute_y(); break;
                // LDY
                case 0xA0: ldy_immediate(); break;
                case 0xA4: ldy_zero(); break;
                case 0xB4: ldy_zero_x(); break;
                case 0xAC: ldy_absolute(); break;
                case 0xBC: ldy_absolute_x(); break;
                // STA
                case 0x85: sta_zero(); break;
                case 0x95: sta_zero_x(); break;
                case 0x8D: sta_absolute(); break;
                case 0x9D: sta_absolute_x(); break;
                case 0x99: sta_absolute_y(); break;
                case 0x81: sta_indirect_x(); break;
                case 0x91: sta_indirect_y(); break;
                // STX
                case 0x86: stx_zero(); break;
                case 0x96: stx_zero_y(); break;
                case 0x8E: stx_absolute(); break;
                // STY
                case 0x84: sty_zero(); break;
                case 0x94: sty_zero_x(); break;
                case 0x8C: sty_absolute(); break;
                // ADC
                case 0x69: adc_immediate(); break;
                case 0x65: adc_zero(); break;
                case 0x75: adc_zero_x(); break;
                case 0x6D: adc_absolute(); break;
                case 0x7D: adc_absolute_x(); break;
                case 0x79: adc_absolute_y(); break;
                case 0x61: adc_indirect_x(); break;
                case 0x71: adc_indirect_y(); break;
                // SBC
                case 0xE9: sbc_immediate(); break;
                case 0xE5: sbc_zero(); break;
                case 0xF5: sbc_zero_x(); break;
                case 0xED: sbc_absolute(); break;
                case 0xFD: sbc_absolute_x(); break;
                case 0xF9: sbc_absolute_y(); break;
                case 0xE1: sbc_indirect_x(); break;
                case 0xF1: sbc_indirect_y(); break;
                // AND
                case 0x29: and_immediate(); break;
                case 0x25: and_zero(); break;
                case 0x35: and_zero_x(); break;
                case 0x2D: and_absolute(); break;
                case 0x3D: and_absolute_x(); break;
                case 0x39: and_absolute_y(); break;
                case 0x21: and_indirect_x(); break;
                case 0x31: and_indirect_y(); break;
                // ORA
                case 0x09: ora_immediate(); break;
                case 0x05: ora_zero(); break;
                case 0x15: ora_zero_x(); break;
                case 0x0D: ora_absolute(); break;
                case 0x1D: ora_absolute_x(); break;
                case 0x19: ora_absolute_y(); break;
                case 0x01: ora_indirect_x(); break;
                case 0x11: ora_indirect_y(); break;
                // EOR
                case 0x49: eor_immediate(); break;
                case 0x45: eor_zero(); break;
                case 0x55: eor_zero_x(); break;
                case 0x4D: eor_absolute(); break;
                case 0x5D: eor_absolute_x(); break;
                case 0x59: eor_absolute_y(); break;
                case 0x41: eor_indirect_x(); break;
                case 0x51: eor_indirect_y(); break;
                // CMP
                case 0xC9: cmp_immediate(); break;
                case 0xC5: cmp_zero(); break;
                case 0xD5: cmp_zero_x(); break;
                case 0xCD: cmp_absolute(); break;
                case 0xDD: cmp_absolute_x(); break;
                case 0xD9: cmp_absolute_y(); break;
                case 0xC1: cmp_indirect_x(); break;
                case 0xD1: cmp_indirect_y(); break;
                // CPX
                case 0xE0: cpx_immediate(); break;
                case 0xE4: cpx_zero(); break;
                case 0xEC: cpx_absolute(); break;
                // CPY
                case 0xC0: cpy_immediate(); break;
                case 0xC4: cpy_zero(); break;
                case 0xCC: cpy_absolute(); break;
                // ASL
                case 0x0A: asl_a(); break;
                case 0x06: asl_zero(); break;
                case 0x16: asl_zero_x(); break;
                case 0x0E: asl_absolute(); break;
                case 0x1E: asl_absolute_x(); break;
                // LSR
                case 0x4A: lsr_a(); break;
                case 0x46: lsr_zero(); break;
                case 0x56: lsr_zero_x(); break;
                case 0x4E: lsr_absolute(); break;
                case 0x5E: lsr_absolute_x(); break;
                // ROL
                case 0x2A: rol_a(); break;
                case 0x26: rol_zero(); break;
                case 0x36: rol_zero_x(); break;
                case 0x2E: rol_absolute(); break;
                case 0x3E: rol_absolute_x(); break;
                // ROR
                case 0x6A: ror_a(); break;
                case 0x66: ror_zero(); break;
                case 0x76: ror_zero_x(); break;
                case 0x6E: ror_absolute(); break;
                case 0x7E: ror_absolute_x(); break;
                // INC/INX/INY
                case 0xE6: inc_zero(); break;
                case 0xF6: inc_zero_x(); break;
                case 0xEE: inc_absolute(); break;
                case 0xFE: inc_absolute_x(); break;
                case 0xE8: inx(); break;
                case 0xC8: iny(); break;
                // DEC/DEX/DEY
                case 0xC6: dec_zero(); break;
                case 0xD6: dec_zero_x(); break;
                case 0xCE: dec_absolute(); break;
                case 0xDE: dec_absolute_x(); break;
                case 0xCA: dex(); break;
                case 0x88: dey(); break;
                // BIT
                case 0x24: bit_zero(); break;
                case 0x2C: bit_absolute(); break;
                // PHA/PLA/PHP/PLP
                case 0x48: pha(); break;
                case 0x08: pla(); break;
                case 0x68: php(); break;
                case 0x28: plp(); break;
                // JMP
                case 0x4c: jmp_absolute(); break;
                case 0x6c: jmp_indirect(); break;
                // JSR/RTS
                case 0x20: jsr(); break;
                case 0x60: rts(); break;
                // RTI
                case 0x40: rti(); break;
            }
        }
    }

    size_t save(char* buffer)
    {
        int index = 0;
        memcpy(buffer, &reg, sizeof(reg));
        index += sizeof(reg);
        const int maxCompressedSize = LZ4_compressBound(0x8000);
        char zram[maxCompressedSize];
        const int compressedSize = LZ4_compress_default((const char*)ram,
                                                        zram,
                                                        0x8000,
                                                        maxCompressedSize);
        memcpy(buffer + index, &compressedSize, 4);
        index += 4;
        memcpy(buffer + index, zram, compressedSize);
        index += compressedSize;
        return index;
    }

    size_t load(char* buffer)
    {
        int index = 0;
        memcpy(&reg, buffer, sizeof(reg));
        index += sizeof(reg);
        const int maxCompressedSize = LZ4_compressBound(0x8000);
        int compressedSize;
        memcpy(&compressedSize, buffer + index, 4);
        index += 4;
        LZ4_decompress_safe((const char*)buffer + index, (char*)ram, compressedSize, 0x8000);
        index += compressedSize;
        changeProgramBank8000(reg.prg8000);
        changeProgramBankC000(reg.prgC000);
        return index;
    }
};
