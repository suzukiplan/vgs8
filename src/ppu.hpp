// Copyright 2019, SUZUKI PLAN (MIT license)

class PPU
{
    struct OAM {
        unsigned char x;       // X
        unsigned char y;       // Y
        unsigned char pattern; // pattern number of CHR
        unsigned char attr;    // attribute
    };

    struct Register {
        /* CHR bank number */
        unsigned char cbank[2];

        /**
         * 76543210
         * -----FBS
         * F: chr number of FG
         * B: chr number of BG
         * S: chr number of Sprite
         */
        unsigned char cmap;

        /* background color */
        unsigned char bgC;

        /* window position of BG/FG */
        unsigned char bgX;
        unsigned char bgY;
        unsigned char fgX;
        unsigned char fgY;
    };

  private:
    VirtualMachine* vm;

  public:
    struct Register reg;
    unsigned char vram[65536];

    PPU(VirtualMachine* vm)
    {
        this->vm = vm;
        reset();
    }

    ~PPU()
    {
    }

    void reset()
    {
        memset(&this->reg, 0, sizeof(this->reg));
        memset(vram, 0, sizeof(vram));
        memcpy(&vm->cpu->ram[0x5C00], vm->bank->pal, 1024);
    }

    /**
     * update vram with current register
     */
    void execute()
    {
        int i, j;
        unsigned char y, x, vx, vy, dx, dy;
        unsigned char* data;
        unsigned char* dptr;
        unsigned char c;
        unsigned char window[32 * 32];
        memset(vram, reg.bgC, sizeof(vram));

        // draw BG
        data = (unsigned char*)vm->bank->chr[reg.cbank[reg.cmap & 2 ? 1 : 0]];
        if (data) {
            vy = reg.bgY / 8;
            vx = reg.bgX / 8;
            for (i = 0; i < 32; i++) {
                for (j = 0; j < 32; j++) {
                    window[i * 32 + j] = vm->cpu->ram[0x6000 + (vy + i) * 32 + vx + j];
                }
            }
            dy = reg.bgY % 8;
            dx = reg.bgX % 8;
            for (y = 0; y < 32; y++) {
                for (x = 0; x < 32; x++) {
                    dptr = &data[window[(y * 32 + x) * 64]];
                    if (dptr) {
                        vx = x * 8 + dx;
                        vy = y * 8 + dy;
                        vram[vy * 256 + vx] = *dptr;
                    }
                }
            }
        }

        // draw sprites
        data = (unsigned char*)vm->bank->chr[reg.cbank[reg.cmap & 1]];
        if (data) {
            for (i = 0x5000; i < 0x5400; i += 4) {
                struct OAM* oam = (struct OAM*)&vm->cpu->ram[i];
                dptr = &data[64 * oam->pattern];
                for (y = 0; y < 8; y++) {
                    for (x = 0; x < 8; x++) {
                        c = dptr[y * 8 + x];
                        if (c) {
                            vx = oam->x + x;
                            vy = oam->y + y;
                            vram[vy * 256 + vx] = c;
                        }
                    }
                }
            }
        }

        // draw FG
        data = (unsigned char*)vm->bank->chr[reg.cbank[reg.cmap & 4 ? 1 : 0]];
        if (data) {
            vy = reg.bgY / 8;
            vx = reg.bgX / 8;
            for (i = 0; i < 32; i++) {
                for (j = 0; j < 32; j++) {
                    window[i * 32 + j] = vm->cpu->ram[0x7000 + (vy + i) * 32 + vx + j];
                }
            }
            dy = reg.bgY % 8;
            dx = reg.bgX % 8;
            for (y = 0; y < 32; y++) {
                for (x = 0; x < 32; x++) {
                    dptr = &data[window[(y * 32 + x) * 64]];
                    if (dptr) {
                        vx = x * 8 + dx;
                        vy = y * 8 + dy;
                        vram[vy * 256 + vx] = *dptr;
                    }
                }
            }
        }
    }

    size_t save(char* buffer)
    {
        memcpy(buffer, &reg, sizeof(reg));
        return sizeof(reg);
    }

    size_t load(char* buffer)
    {
        memcpy(&reg, buffer, sizeof(reg));
        return sizeof(reg);
    }
};