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

    inline void lda_immediate()
    {
        reg.a = ram[++reg.pc];  // a = immediate value
        reg.p |= reg.a & 0x80;  // set negative
        reg.p |= reg.a ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

    inline void ldx_immediate()
    {
        reg.x = ram[++reg.pc];  // x = immediate value
        reg.p |= reg.x & 0x80;  // set negative
        reg.p |= reg.x ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

    inline void ldy_immediate()
    {
        reg.y = ram[++reg.pc];  // y = immediate value
        reg.p |= reg.y & 0x80;  // set negative
        reg.p |= reg.y ? 2 : 0; // set zero
        reg.pc++;               // increment pc
        clocks += 2;            // tick the clock
    }

    inline void lda_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.a = ram[addr];                     // a = zero page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.x = ram[addr];                     // x = zero page value
        reg.p |= reg.x & 0x80;                 // set negative
        reg.p |= reg.x ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero()
    {
        unsigned short addr;
        addr = ram[++reg.pc];                  // calculate address
        reg.y = ram[addr];                     // y = zero page value
        reg.p |= reg.y & 0x80;                 // set negative
        reg.p |= reg.y ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 3;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;          // calculate address
        reg.a = ram[addr];                     // a = zero page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_zero_y()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.y;          // calculate address
        reg.x = ram[addr];                     // x = zero page value
        reg.p |= reg.x & 0x80;                 // set negative
        reg.p |= reg.x ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_zero_x()
    {
        unsigned short addr;
        addr = ram[++reg.pc] + reg.x;          // calculate address
        reg.y = ram[addr];                     // y = zero page value
        reg.p |= reg.y & 0x80;                 // set negative
        reg.p |= reg.y ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_absolute()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (HIGH)
        addr <<= 8;                            // addr *= 256
        addr |= ram[++reg.pc];                 // get addr (LOW)
        reg.a = ram[addr];                     // a = any page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_absolute()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (HIGH)
        addr <<= 8;                            // addr *= 256
        addr |= ram[++reg.pc];                 // get addr (LOW)
        reg.x = ram[addr];                     // x = any page value
        reg.p |= reg.x & 0x80;                 // set negative
        reg.p |= reg.x ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (HIGH)
        addr <<= 8;                            // addr *= 256
        addr |= ram[++reg.pc];                 // get addr (LOW)
        reg.y = ram[addr];                     // y = any page value
        reg.p |= reg.y & 0x80;                 // set negative
        reg.p |= reg.y ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_absolute_x()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (LOW)
        addr |= ram[++reg.pc] * 256;           // get addr (HIGH)
        reg.a = ram[addr + reg.x];             // a = any page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_absolute_y()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (LOW)
        addr |= ram[++reg.pc] * 256;           // get addr (HIGH)
        reg.a = ram[addr + reg.y];             // a = any page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void ldx_absolute_y()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (LOW)
        addr |= ram[++reg.pc] * 256;           // get addr (HIGH)
        reg.x = ram[addr + reg.y];             // x = any page value
        reg.p |= reg.x & 0x80;                 // set negative
        reg.p |= reg.x ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.x); // I/O check
    }

    inline void ldy_absolute_x()
    {
        unsigned short addr = ram[++reg.pc];   // get addr (LOW)
        addr |= ram[++reg.pc] * 256;           // get addr (HIGH)
        reg.y = ram[addr + reg.x];             // y = any page value
        reg.p |= reg.y & 0x80;                 // set negative
        reg.p |= reg.y ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 4;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.y); // I/O check
    }

    inline void lda_indirect_x()
    {
        unsigned short ptr = ram[++reg.pc] + reg.x;
        unsigned short addr = ram[ptr++];      // get addr (LOW)
        addr |= ram[ptr] * 256;                // get addr (HIGH)
        reg.a = ram[addr];                     // a = any page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 6;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
    }

    inline void lda_indirect_y()
    {
        unsigned short ptr = ram[++reg.pc];
        unsigned short addr = ram[ptr++];      // get addr (LOW)
        addr |= ram[ptr] * 256;                // get addr (HIGH)
        reg.a = ram[addr + reg.y];             // a = any page value
        reg.p |= reg.a & 0x80;                 // set negative
        reg.p |= reg.a ? 2 : 0;                // set zero
        reg.pc++;                              // increment pc
        clocks += 5;                           // tick the clock
        checkLD(addr, (unsigned char*)&reg.a); // I/O check
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

    void execute()
    {
        // TODO: デシマルモードを除く6502の全命令を実装予定
        vramUpdateRequest = false;
        while (!vramUpdateRequest) {
            switch (ram[reg.pc]) {
                // TRANSFER
                case 0x8A: txa(); break;
                case 0xAA: tax(); break;
                // LDA
                case 0xA5: lda_zero(); break;
                case 0xA9: lda_immediate(); break;
                case 0xAD: lda_absolute(); break;
                case 0xB5: lda_zero_x(); break;
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
