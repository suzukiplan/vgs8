// Copyright 2019, SUZUKI PLAN (MIT license)
// MOS6502 emulator
#ifndef INCLUDE_CPU_HPP
#define INCLUDE_CPU_HPP
#define CPU_CLOCKS_PER_FRAME 139810 /* 8MHz / 60 */

using namespace VGS8;

class CPU
{
    struct Register {
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
    struct Register reg;
    bool vramUpdateRequest;
#ifdef DEBUG_OP_DUMP
    char debugLine[32];
#endif

    // nametable全体を縦方向にローテートさせる
    inline void nameTableRotateV(unsigned short base, char value)
    {
        unsigned char work[64];
        if (value & 0x80) {
            // 上スクロール
            memcpy(work, &ram[base], 64);
            memmove(&ram[base], &ram[base + 64], 64 * 63);
            memcpy(&ram[base + 64 * 63], work, 64);
        } else if (value) {
            // 下スクロール
            memcpy(work, &ram[base + 64 * 63], 64);
            memmove(&ram[base + 64], &ram[base], 64 * 63);
            memcpy(&ram[base], work, 64);
        }
    }

    // FGのnametable全体を横方向にシフトさせる
    inline void nameTableRotateH(unsigned short base, char value)
    {
        unsigned char work;
        if (value & 0x80) {
            // 左スクロール
            for (int y = 0; y < 64 * 64; y += 64) {
                work = ram[base + y];
                memmove(&ram[base + y], &ram[base + y + 1], 63);
                ram[base + y + 63] = work;
            }
        } else if (value) {
            // 右スクロール
            for (int y = 0; y < 64 * 64; y += 64) {
                work = ram[base + y + 63];
                memmove(&ram[base + y + 1], &ram[base + y], 63);
                ram[base + y] = work;
            }
        }
    }

    inline unsigned char calc_degree()
    {
        double x1 = ram[0x5A10];
        double y1 = ram[0x5A11];
        double x2 = ram[0x5A12];
        double y2 = ram[0x5A13];
        unsigned char deg = (unsigned char)((M_PI + atan2(y1 - y2, x1 - x2)) / (M_PI * 2) * 255);
        deg -= 191;
        return deg;
    }

    inline void checkLD(unsigned short addr, unsigned char* value)
    {
        switch (addr) {
            case 0x5603: *value = vm->_isBgmPlaying() ? 1 : 0; break;
            case 0x5700: *value = vm->keys[0]; break;
            case 0x5701: *value = vm->keys[1]; break;
            case 0x5800: *value = vm->touching ? 1 : 0; break;
            case 0x5801: *value = vm->touchX; break;
            case 0x5802: *value = vm->touchY; break;
            case 0x5A14: *value = calc_degree(); break;
            case 0x5BFF: vramUpdateRequest = true; return;
            default: return;
        }
        updateNZ(*value);
    }

    inline void checkST(unsigned short addr, unsigned char value)
    {
        switch (addr) {
            case 0x5400: changeProgramBank8000(value); break;
            case 0x5401: changeProgramBankC000(value); break;
            case 0x5402: vm->_setChrBank(0, value); break;
            case 0x5403: vm->_setChrBank(1, value); break;
            case 0x5404: vm->_setChrMap(value); break;
            case 0x5405: vm->_setBgColor(value); break;
            case 0x5406: vm->_setFgX(value); break;
            case 0x5407: vm->_setFgY(value); break;
            case 0x5408: vm->_setBgX(value); break;
            case 0x5409: vm->_setBgY(value); break;
            case 0x540A: nameTableRotateV(0x7000, value); break;
            case 0x540B: nameTableRotateH(0x7000, value); break;
            case 0x540C: nameTableRotateV(0x6000, value); break;
            case 0x540D: nameTableRotateH(0x6000, value); break;
            case 0x5500: vm->_playEff(value); break;
            case 0x5501: vm->_stopEff(value); break;
            case 0x5502: vm->_pauseEff(); break;
            case 0x5503: vm->_resumeEff(); break;
            case 0x5504: vm->_updateEffMasterVolume(); break;
            case 0x5600: vm->_playBgm(value); break;
            case 0x5601: vm->_pauseBgm(); break;
            case 0x5602: vm->_resumeBgm(); break;
            case 0x5604: vm->_updateBgmMasterVolume(); break;
            case 0x5A01: memset(&ram[ram[0x5A00] * 256], value, 256); break;
            case 0x5A02: memcpy(&ram[value * 256], &ram[ram[0x5A00] * 256], 256); break;
            case 0x5BFF: vramUpdateRequest = true; break;
        }
    }

#ifdef DEBUG_OP_DUMP
    inline const char* registerDump()
    {
        static char buf[80];
        static struct Register prv;
        sprintf(buf, "(a=%s$%02X\e[m, x=%s$%02X\e[m, y=%s$%02X\e[m, s=%s$%02X\e[m, p=%s$%02X\e[m)",
                reg.a != prv.a ? "\e[31m" : "",
                (int)((unsigned char)reg.a),
                reg.x != prv.x ? "\e[31m" : "",
                (int)reg.x,
                reg.y != prv.y ? "\e[31m" : "",
                (int)reg.y,
                reg.s != prv.s ? "\e[31m" : "",
                (int)reg.s,
                reg.p != prv.p ? "\e[31m" : "",
                (int)reg.p);
        memcpy(&prv, &reg, sizeof(Register));
        return buf;
    }
#endif

    inline void updateNZ(unsigned char value)
    {
        reg.p &= 0b01111101;    // clear N, Z
        reg.p |= value & 0x80;  // set negative
        reg.p |= value ? 0 : 2; // set zero
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
        unsigned short addr = ram[++reg.pc]; // get addr (LOW)
        addr |= ram[++reg.pc] * 256;         // get addr (HIGH)
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "$%04X", (int)addr);
#endif
        return addr;
    }

    inline unsigned short absoluteX()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (LOW)
        addr |= ram[++reg.pc] * 256;         // get addr (HIGH)
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "$%04X, X", (int)addr);
#endif
        addr += reg.x;
        return addr;
    }

    inline unsigned short absoluteY()
    {
        unsigned short addr = ram[++reg.pc]; // get addr (LOW)
        addr |= ram[++reg.pc] * 256;         // get addr (HIGH)
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "$%04X, Y", (int)addr);
#endif
        addr += reg.y;
        return addr;
    }

    inline unsigned short indirect()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "($%02X) = $%04X", (int)ram[reg.pc], addr);
#endif
        return addr;
    }

    inline unsigned short indirectX()
    {
        unsigned short ptr = ram[++reg.pc] + reg.x;
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "($%02X, X) = $%04X", (int)ram[reg.pc], addr);
#endif
        return addr;
    }

    inline unsigned short indirectY()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++]; // get addr (LOW)
        addr |= ram[ptr] * 256;           // get addr (HIGH)
        addr += reg.y;
#ifdef DEBUG_OP_DUMP
        sprintf(&debugLine[4], "($%02X), Y = $%04X", (int)ram[reg.pc], addr);
#endif
        return addr;
    }

    inline void lda_immediate()
    {
        reg.a = ram[++reg.pc]; // a = immediate value
        updateNZ(reg.a);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDA #$%02X", (int)((unsigned char)reg.a));
#endif
    }

    inline void ldx_immediate()
    {
        reg.x = ram[++reg.pc]; // x = immediate value
        updateNZ(reg.x);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDX #$%02X", (int)reg.x);
#endif
    }

    inline void ldy_immediate()
    {
        reg.y = ram[++reg.pc]; // y = immediate value
        updateNZ(reg.y);       // update p
        reg.pc++;              // increment pc
        clocks += 2;           // tick the clock
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDY #$%02X", (int)reg.y);
#endif
    }

    inline void lda_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDA $%02X", (int)ram[reg.pc]);
#endif
        reg.a = ram[addr];                     // a = zero page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDX $%02X", (int)ram[reg.pc]);
#endif
        reg.x = ram[addr];                     // x = zero page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDY $%02X", (int)ram[reg.pc]);
#endif
        reg.y = ram[addr];                     // y = zero page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STA $%02X", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.a;                   // store a to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STX $%02X", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.x;                   // store x to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc]; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STY $%02X", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.y;                   // store y to zero page
        reg.pc++;                            // increment pc
        clocks += 3;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDA $%02X, X", (int)ram[reg.pc]);
#endif
        reg.a = ram[addr];                     // a = zero page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero_y()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.y; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDX $%02X, Y", (int)ram[reg.pc]);
#endif
        reg.x = ram[addr];                     // x = zero page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LDY $%02X, X", (int)ram[reg.pc]);
#endif
        reg.y = ram[addr];                     // y = zero page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STA $%02X, X ", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.a;                   // store a to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_zero_y()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.y; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STX $%02X, Y", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.x;                   // store x to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x; // calculate address
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "STY $%02X, X", (int)ram[reg.pc]);
#endif
        ram[addr] = reg.y;                   // store y to zero page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDA ");
#endif
        unsigned short addr = absolute();      // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDX ");
#endif
        unsigned short addr = absolute();      // get absolute address
        reg.x = ram[addr];                     // x = any page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDY ");
#endif
        unsigned short addr = absolute();      // get absolute address
        reg.y = ram[addr];                     // y = any page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void sta_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STA ");
#endif
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void stx_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STX ");
#endif
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.x;                   // store x to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.x); // I/O check
    }

    inline void sty_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STY ");
#endif
        unsigned short addr = absolute();    // get absolute address
        ram[addr] = reg.y;                   // store y to any page
        reg.pc++;                            // increment pc
        clocks += 4;                         // tick the clock
        checkST(addr, (unsigned char)reg.y); // I/O check
    }

    inline void lda_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDA ");
#endif
        unsigned short addr = absoluteX();     // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDA ");
#endif
        unsigned short addr = absoluteY();     // get absolute address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void sta_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STA ");
#endif
        unsigned short addr = absoluteX();   // get absolute address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 5;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void sta_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STA ");
#endif
        unsigned short addr = absoluteY();   // get absolute address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 5;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void ldx_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDX ");
#endif
        unsigned short addr = absoluteY();     // get absolute address
        reg.x = ram[addr];                     // x = any page value
        updateNZ(reg.x);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDY ");
#endif
        unsigned short addr = absoluteX();     // get absolute address
        reg.y = ram[addr];                     // y = any page value
        updateNZ(reg.y);                       // update p
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDA ");
#endif
        unsigned short addr = indirectX();     // get indirectX address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 6;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LDA ");
#endif
        unsigned short addr = indirectY();     // get indirectY address
        reg.a = ram[addr];                     // a = any page value
        updateNZ(reg.a);                       // update p
        reg.pc++;                              // increment pc
        clocks += 5;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void sta_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STA ");
#endif
        unsigned short addr = indirectX();   // get indirectX address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 6;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void sta_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "STA ");
#endif
        unsigned short addr = indirectY();   // get indirectY address
        ram[addr] = reg.a;                   // store a to any page
        reg.pc++;                            // increment pc
        clocks += 5;                         // tick the clock
        checkST(addr, (unsigned char)reg.a); // I/O check
    }

    inline void tax()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TAX");
#endif
        reg.x = reg.a;   // transfer a to x
        updateNZ(reg.x); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void txa()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TXA");
#endif
        reg.a = reg.x;   // transfer x to a
        updateNZ(reg.a); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tay()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TAY");
#endif
        reg.y = reg.a;   // transfer a to y
        updateNZ(reg.y); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tya()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TYA");
#endif
        reg.a = reg.y;   // transfer y to a
        updateNZ(reg.a); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void tsx()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TSX");
#endif
        reg.x = reg.s;   // transfer s to x
        updateNZ(reg.x); // update p
        reg.pc++;        // increment pc
        clocks += 2;     // tick the clock
    }

    inline void txs()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "TXS");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ADC #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void adc_zero()
    {
        adc(ram[ram[++reg.pc]]); // add with carry
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ADC $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void adc_zero_x()
    {
        adc(ram[ram[++reg.pc] + reg.x]); // add with carry
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ADC $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void adc_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ADC ");
#endif
        adc(ram[absolute()]); // add with carry
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void adc_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ADC ");
#endif
        adc(ram[absoluteX()]); // add with carry
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void adc_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ADC ");
#endif
        adc(ram[absoluteY()]); // add with carry
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void adc_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ADC ");
#endif
        adc(ram[indirectX()]); // add with carry
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void adc_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ADC ");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "SBC #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void sbc_zero()
    {
        sbc(ram[ram[++reg.pc]]); // sub with carry
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "SBC $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void sbc_zero_x()
    {
        sbc(ram[ram[++reg.pc] + reg.x]); // sub with carry
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "SBC $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void sbc_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "SBC ");
#endif
        sbc(ram[absolute()]); // sub with carry
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void sbc_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "SBC ");
#endif
        sbc(ram[absoluteX()]); // sub with carry
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void sbc_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "SBC ");
#endif
        sbc(ram[absoluteY()]); // sub with carry
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void sbc_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "SBC ");
#endif
        sbc(ram[indirectX()]); // sub with carry
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void sbc_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "SBC ");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "AND #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void and_zero()
    {
        andA(ram[ram[++reg.pc]]); // and
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "AND $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void and_zero_x()
    {
        andA(ram[ram[++reg.pc] + reg.x]); // and
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "AND $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void and_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "AND ");
#endif
        andA(ram[absolute()]); // and
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void and_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "AND ");
#endif
        andA(absoluteX()); // and
        reg.pc++;          // increment pc
        clocks += 4;       // tick the clock
    }

    inline void and_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "AND ");
#endif
        andA(absoluteY()); // and
        reg.pc++;          // increment pc
        clocks += 4;       // tick the clock
    }

    inline void and_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "AND ");
#endif
        andA(ram[indirectX()]); // and
        reg.pc++;               // increment pc
        clocks += 6;            // tick the clock
    }

    inline void and_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "AND ");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ORA #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void ora_zero()
    {
        ora(ram[ram[++reg.pc]]); // or
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ORA $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void ora_zero_x()
    {
        ora(ram[ram[++reg.pc] + reg.x]); // or
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ORA $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void ora_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ORA ");
#endif
        ora(ram[absolute()]); // or
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void ora_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ORA ");
#endif
        ora(ram[absoluteX()]); // or
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void ora_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ORA ");
#endif
        ora(ram[absoluteY()]); // or
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void ora_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ORA ");
#endif
        ora(ram[indirectX()]); // or
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void ora_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ORA ");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "EOR #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void eor_zero()
    {
        eor(ram[ram[++reg.pc]]); // eor
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "EOR $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void eor_zero_x()
    {
        eor(ram[ram[++reg.pc] + reg.x]); // eor
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "EOR $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void eor_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "EOR ");
#endif
        eor(ram[absolute()]); // eor
        reg.pc++;             // increment pc
        clocks += 4;          // tick the clock
    }

    inline void eor_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "EOR ");
#endif
        eor(ram[absoluteX()]); // eor
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void eor_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "EOR ");
#endif
        eor(ram[absoluteY()]); // eor
        reg.pc++;              // increment pc
        clocks += 4;           // tick the clock
    }

    inline void eor_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "EOR ");
#endif
        eor(ram[indirectX()]); // eor
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void eor_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "EOR ");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CMP #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void cmp_zero()
    {
        compare(reg.a, ram[ram[++reg.pc]]); // comapre
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CMP $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void cmp_zero_x()
    {
        compare(reg.a, ram[ram[++reg.pc] + reg.x]); // compare
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CMP $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 4; // tick the clock
    }

    inline void cmp_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CMP ");
#endif
        compare(reg.a, ram[absolute()]); // compare
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void cmp_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CMP ");
#endif
        compare(reg.a, ram[absoluteX()]); // compare
        reg.pc++;                         // increment pc
        clocks += 4;                      // tick the clock
    }

    inline void cmp_absolute_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CMP ");
#endif
        compare(reg.a, ram[absoluteY()]); // compare
        reg.pc++;                         // increment pc
        clocks += 4;                      // tick the clock
    }

    inline void cmp_indirect_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CMP ");
#endif
        compare(reg.a, ram[indirectX()]); // compare
        reg.pc++;                         // increment pc
        clocks += 6;                      // tick the clock
    }

    inline void cmp_indirect_y()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CMP ");
#endif
        compare(reg.a, ram[indirectY()]); // compare
        reg.pc++;                         // increment pc
        clocks += 5;                      // tick the clock
    }

    inline void cpx_immediate()
    {
        compare(reg.x, ram[++reg.pc]); // compare
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CPX #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void cpx_zero()
    {
        compare(reg.x, ram[ram[++reg.pc]]); // comapre
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CPX $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void cpx_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CPX ");
#endif
        compare(reg.x, ram[absolute()]); // compare
        reg.pc++;                        // increment pc
        clocks += 4;                     // tick the clock
    }

    inline void cpy_immediate()
    {
        compare(reg.y, ram[++reg.pc]); // compare
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CPY #$%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
    }

    inline void cpy_zero()
    {
        compare(reg.y, ram[ram[++reg.pc]]); // comapre
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "CPY $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void cpy_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "CPY ");
#endif
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
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ASL ");
#endif
        shift(true, (unsigned char*)&reg.a); // left shift
        reg.pc++;                            // increment pc
        clocks += 2;                         // tick the clock
    }

    inline void asl_zero()
    {
        shift(true, &ram[ram[++reg.pc]]); // left shift
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ASL $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void asl_zero_x()
    {
        shift(true, &ram[ram[++reg.pc] + reg.x]); // left shift
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ASL $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void asl_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ASL ");
#endif
        shift(true, &ram[absolute()]); // left shift
        reg.pc++;                      // increment pc
        clocks += 6;                   // tick the clock
    }

    inline void asl_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ASL ");
#endif
        shift(true, &ram[absoluteX()]); // left shift
        reg.pc++;                       // increment pc
        clocks += 7;                    // tick the clock
    }

    inline void lsr_a()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LSR ");
#endif
        shift(false, (unsigned char*)&reg.a); // right shift
        reg.pc++;                             // increment pc
        clocks += 2;                          // tick the clock
    }

    inline void lsr_zero()
    {
        shift(false, &ram[ram[++reg.pc]]); // right shift
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LSR $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void lsr_zero_x()
    {
        shift(false, &ram[ram[++reg.pc] + reg.x]); // right shift
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "LSR $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void lsr_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LSR ");
#endif
        shift(false, &ram[absolute()]); // right shift
        reg.pc++;                       // increment pc
        clocks += 6;                    // tick the clock
    }

    inline void lsr_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "LSR ");
#endif
        shift(false, &ram[absoluteX()]); // right shift
        reg.pc++;                        // increment pc
        clocks += 7;                     // tick the clock
    }

    inline void rotate(bool isLeft, unsigned char* v)
    {
        reg.p &= 0b01111100; // clear N, Z, C
        int a = *v;
        unsigned char lsb = a & 1;
        unsigned char msb = a & 0x80;
        if (isLeft) {
            a <<= 1;
            if (msb) {
                a |= 1;
                *v = a & 0xFF;
                reg.p |= 1;
            } else {
                *v = a & 0xFE;
            }
        } else {
            a >>= 1;
            a &= 0x7F;
            if (lsb) {
                a |= 0x80;
                *v = a & 0xFF;
                reg.p |= 1;
            } else {
                *v = a & 0x7F;
            }
        }
        if (a & 0x80) reg.p |= 0x80;
        if (0 == *v) reg.p |= 2;
    }

    inline void rol_a()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROL ");
#endif
        rotate(true, (unsigned char*)&reg.a); // left rotate
        reg.pc++;                             // increment pc
        clocks += 2;                          // tick the clock
    }

    inline void rol_zero()
    {
        rotate(true, &ram[ram[++reg.pc]]); // left rotate
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ROL $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void rol_zero_x()
    {
        rotate(true, &ram[ram[++reg.pc] + reg.x]); // left rotate
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ROL $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void rol_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROL ");
#endif
        rotate(true, &ram[absolute()]); // left rotate
        reg.pc++;                       // increment pc
        clocks += 6;                    // tick the clock
    }

    inline void rol_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROL ");
#endif
        rotate(true, &ram[absoluteX()]); // left rotate
        reg.pc++;                        // increment pc
        clocks += 7;                     // tick the clock
    }

    inline void ror_a()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROR ");
#endif
        rotate(false, (unsigned char*)&reg.a); // right rotate
        reg.pc++;                              // increment pc
        clocks += 2;                           // tick the clock
    }

    inline void ror_zero()
    {
        rotate(false, &ram[ram[++reg.pc]]); // right rotate
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ROR $%02X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void ror_zero_x()
    {
        rotate(false, &ram[ram[++reg.pc] + reg.x]); // right rotate
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "ROR $%02X, X", (int)ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void ror_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROR ");
#endif
        rotate(false, &ram[absolute()]); // right rotate
        reg.pc++;                        // increment pc
        clocks += 6;                     // tick the clock
    }

    inline void ror_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "ROR ");
#endif
        rotate(false, &ram[absoluteX()]); // right rotate
        reg.pc++;                         // increment pc
        clocks += 7;                      // tick the clock
    }

    inline void inc(unsigned char* v)
    {
        (*v)++;
        updateNZ(*v);
    }

    inline void inc_zero()
    {
        inc(&ram[ram[++reg.pc]]); // increment
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "INC $%02X", ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void inc_zero_x()
    {
        inc(&ram[ram[++reg.pc] + reg.x]); // increment
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "INC $%02X, X", ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void inc_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "INC ");
#endif
        inc(&ram[absolute()]); // increment
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void inc_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "INC ");
#endif
        inc(&ram[absoluteX()]); // increment
        reg.pc++;               // increment pc
        clocks += 7;            // tick the clock
    }

    inline void inx()
    {
        inc(&reg.x); // increment
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "INX");
#endif
    }

    inline void iny()
    {
        inc(&reg.y); // increment
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "INY");
#endif
    }

    inline void dec(unsigned char* v)
    {
        (*v)--;
        updateNZ(*v);
    }

    inline void dec_zero()
    {
        dec(&ram[ram[++reg.pc]]); // decrement
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "DEC $%02X", ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 5; // tick the clock
    }

    inline void dec_zero_x()
    {
        dec(&ram[ram[++reg.pc] + reg.x]); // decrement
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "DEC $%02X, X", ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 6; // tick the clock
    }

    inline void dec_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "DEC ");
#endif
        dec(&ram[absolute()]); // decrement
        reg.pc++;              // increment pc
        clocks += 6;           // tick the clock
    }

    inline void dec_absolute_x()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "DEC ");
#endif
        dec(&ram[absoluteX()]); // decrement
        reg.pc++;               // increment pc
        clocks += 7;            // tick the clock
    }

    inline void dex()
    {
        dec(&reg.x); // decrement
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "DEX");
#endif
    }

    inline void dey()
    {
        dec(&reg.y); // decrement
        reg.pc++;    // increment pc
        clocks += 2; // tick the clock
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "DEY");
#endif
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
#ifdef DEBUG_OP_DUMP
        sprintf(debugLine, "BIT $%02X", ram[reg.pc]);
#endif
        reg.pc++;    // increment pc
        clocks += 3; // tick the clock
    }

    inline void bit_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "BIT ");
#endif
        bit(ram[absolute()]); // update p
        reg.pc++;             // increment pc
        clocks += 3;          // tick the clock
    }

    inline void push(unsigned char v)
    {
        ram[0x100 + reg.s] = v;
        reg.s++;
    }

    inline unsigned char pull()
    {
        reg.s--;
        return ram[0x100 + reg.s];
    }

    inline void pha()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "PHA ");
#endif
        push(reg.a);
        reg.pc++;
        clocks += 3;
    }

    inline void pla()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "PLA ");
#endif
        reg.a = pull();
        reg.pc++;
        clocks += 4;
    }

    inline void php()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "PHP ");
#endif
        push(reg.p);
        reg.pc++;
        clocks += 3;
    }

    inline void plp()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "PLP ");
#endif
        reg.p = pull();
        reg.pc++;
        clocks += 4;
    }

    inline void jmp_absolute()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "JMP ");
#endif
        reg.pc = absolute();
        clocks += 3;
    }

    inline void jmp_indirect()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "JMP ");
#endif
        reg.pc = indirect();
        clocks += 5;
    }

    inline void jsr()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "JSR ");
#endif
        unsigned short to = absolute();     // get jump to address
        push(((unsigned char*)&reg.pc)[0]); // push return address minus 1 of H
        push(((unsigned char*)&reg.pc)[1]); // push return address minus 1 of L
        reg.pc = to;                        // change program counter (jump to)
        clocks += 6;                        // tick the clock
    }

    inline void rts()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "RTS ");
#endif
        ((unsigned char*)&reg.pc)[1] = pull(); // pull return address of L
        ((unsigned char*)&reg.pc)[0] = pull(); // pull return address of H
        reg.pc++;                              // increment program counter
        clocks += 6;                           // tick the clock
    }

    inline void rti()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "RTI ");
#endif
        reg.p = pull();                        // pull p
        ((unsigned char*)&reg.pc)[1] = pull(); // pull return address of L
        ((unsigned char*)&reg.pc)[0] = pull(); // pull return address of H
        clocks += 6;                           // tick the clock
    }

    inline void branch(unsigned char s, bool isSet)
    {
        clocks += 2;
        short relative = (char)ram[++reg.pc];
#ifdef DEBUG_OP_DUMP
        if (isSet) {
            switch (s) {
                case 0x01: sprintf(debugLine, "BCS $%02X", ram[reg.pc]); break;
                case 0x02: sprintf(debugLine, "BEQ $%02X", ram[reg.pc]); break;
                case 0x40: sprintf(debugLine, "BVS $%02X", ram[reg.pc]); break;
                case 0x80: sprintf(debugLine, "BMI $%02X", ram[reg.pc]); break;
            }
        } else {
            switch (s) {
                case 0x01: sprintf(debugLine, "BCC $%02X", ram[reg.pc]); break;
                case 0x02: sprintf(debugLine, "BNE $%02X", ram[reg.pc]); break;
                case 0x40: sprintf(debugLine, "BVC $%02X", ram[reg.pc]); break;
                case 0x80: sprintf(debugLine, "BPL $%02X", ram[reg.pc]); break;
            }
        }
#endif
        reg.pc++;
        if (reg.p & s) {
            if (isSet) {
                reg.pc += relative;
                return;
            }
        } else {
            if (!isSet) {
                reg.pc += relative;
                return;
            }
        }
    }

    inline void status(unsigned char s, bool isSet)
    {
        if (isSet) {
#ifdef DEBUG_OP_DUMP
            switch (s) {
                case 0x01: strcpy(debugLine, "SEC"); break;
                case 0x04: strcpy(debugLine, "SEI"); break;
                case 0x08: strcpy(debugLine, "SED"); break;
            }
#endif
            reg.p |= s;
        } else {
#ifdef DEBUG_OP_DUMP
            switch (s) {
                case 0x01: strcpy(debugLine, "CLC"); break;
                case 0x04: strcpy(debugLine, "CLI"); break;
                case 0x08: strcpy(debugLine, "CLD"); break;
                case 0x40: strcpy(debugLine, "CLV"); break;
            }
#endif
            s ^= 0xFF;
            reg.p &= s;
        }
        reg.pc += 1;
        clocks += 2;
    }

    inline void brk()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "BRK ");
#endif
        // VGSの場合, vramUpdateRequestを発生させる (非推奨のやり方)
        vramUpdateRequest = true;
        reg.pc += 1;
        clocks += 7;
    }

    inline void nop()
    {
#ifdef DEBUG_OP_DUMP
        strcpy(debugLine, "NOP ");
#endif
        reg.pc += 1;
        clocks += 2;
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
    unsigned char brkBank;
    unsigned short brkAddr;
    void (*brkCB)(VGS8::VirtualMachine*);

    CPU(VGS8::VirtualMachine* vm)
    {
        this->vm = vm;
        this->brkBank = 0;
        this->brkAddr = 0;
        this->brkCB = NULL;
        reset();
    }

    void reset()
    {
        memset(ram, 0, sizeof(ram));
        memset(&reg, 0, sizeof(reg));
        reg.pc = 0x8000;
        changeProgramBank8000(0);
        changeProgramBankC000(1);
    }

    void execute()
    {
        vramUpdateRequest = false;
        clocks = 0;
        while (!vramUpdateRequest) {
            if (brkCB) {
                if (reg.pc == brkAddr) {
                    if (brkAddr < 0xC000) {
                        if (reg.prg8000 == brkBank) {
                            brkCB(vm);
                        }
                    } else {
                        if (reg.prgC000 == brkBank) {
                            brkCB(vm);
                        }
                    }
                }
            }
#ifdef DEBUG_OP_DUMP
            printf("$%04X: ", reg.pc);
            debugLine[0] = '\0';
#endif
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
                case 0x68: pla(); break;
                case 0x08: php(); break;
                case 0x28: plp(); break;
                // JMP
                case 0x4c: jmp_absolute(); break;
                case 0x6c: jmp_indirect(); break;
                // JSR/RTS
                case 0x20: jsr(); break;
                case 0x60: rts(); break;
                // RTI
                case 0x40: rti(); break;
                // other operands (note: P is [N.V.R.B.D.I.Z.C])
                case 0xB0: branch(0x01, true); break;  // BCS (C; carry)
                case 0x90: branch(0x01, false); break; // BCC (C; carry)
                case 0xF0: branch(0x02, true); break;  // BEQ (Z; zero)
                case 0xD0: branch(0x02, false); break; // BNE (Z; zero)
                case 0x70: branch(0x40, true); break;  // BVS (V; overflow)
                case 0x50: branch(0x40, false); break; // BVC (V; overflow)
                case 0x30: branch(0x80, true); break;  // BMI (N; negative)
                case 0x10: branch(0x80, false); break; // BPL (N; negative)
                case 0x38: status(0x01, true); break;  // SEC (C; carry)
                case 0x18: status(0x01, false); break; // CLC (C; carry)
                case 0x78: status(0x04, true); break;  // SEI (I; interrupt)
                case 0x58: status(0x04, false); break; // CLI (I; interrupt)
                case 0xF8: status(0x08, true); break;  // SED (D; decimal) *VGS8では使えない
                case 0xD8: status(0x08, false); break; // CLD (D; decimal) *VGS8では使えない
                case 0xB8: status(0x40, false); break; // CLV (V; overflow)
                case 0x00: brk(); break;
                case 0xEA: nop(); break;
            }
#ifdef DEBUG_OP_DUMP
            for (size_t i = strlen(debugLine); i < sizeof(debugLine) - 1; i++) debugLine[i] = ' ';
            debugLine[sizeof(debugLine) - 1] = '\0';
            printf("%s%s\n", debugLine, registerDump());
#endif
            // 1フレームで処理可能なクロック数を超えた場合に強制的にVRAM更新リクエストを発生させる
            if (CPU_CLOCKS_PER_FRAME <= clocks) {
                vramUpdateRequest = true;
                break;
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
        LZ4_decompress_safe((const char*)buffer + index, (char*)ram, compressedSize, maxCompressedSize);
        index += compressedSize;
        changeProgramBank8000(reg.prg8000);
        changeProgramBankC000(reg.prgC000);
        return index;
    }
};

#endif
