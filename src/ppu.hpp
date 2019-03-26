// Copyright 2019, SUZUKI PLAN (MIT license)

class PPU
{
    struct OAM {
        unsigned char x;       // X
        unsigned char y;       // Y
        unsigned char pattern; // pattern number of CHR
        unsigned char flags;   // flags: -----vhx
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
        unsigned char window[32 * 32];
        memset(vram, reg.bgC, sizeof(vram));

        // draw BG
        int cno = reg.cmap & 2 ? 1 : 0;
        int bno = reg.cbank[cno];
        data = (unsigned char*)vm->bank->chr[bno];
        if (data) {
            vy = reg.bgY / 8;
            vx = reg.bgX / 8;
            for (i = 0; i < 32; i++) {
                for (j = 0; j < 32; j++) {
                    window[i * 32 + j] = vm->cpu->ram[0x6000 + (vy + i) * 64 + vx + j];
                }
            }
            dy = reg.bgY % 8;
            dx = reg.bgX % 8;
            for (y = 0; y < 32; y++) {
                for (x = 0; x < 32; x++) {
                    int cn = window[y * 32 + x];
                    if (!cn) continue; // do not draw $00
                    cn *= 64;
                    for (i = 0; i < 8; i++) {
                        for (j = 0; j < 8; j++) {
                            dptr = &data[cn + i * 8 + j];
                            if (*dptr) {
                                vx = x * 8 + dx + j;
                                vy = y * 8 + dy + i;
                                vram[vy * 256 + vx] = *dptr;
                            }
                        }
                    }
                }
            }
        }

        // draw sprites
        data = (unsigned char*)vm->bank->chr[reg.cbank[reg.cmap & 1]];
        if (data) {
            for (i = 0x5000; i < 0x5400; i += 4) {
                struct OAM* oam = (struct OAM*)&vm->cpu->ram[i];
                if (oam->flags & 0x01) {
                    // 16x16
                    if (oam->flags & 0x04) {
                        if (oam->flags & 0x02) {
                            // 上下左右反転
                            drawSprite(data, oam, 0x00, 8, 8);
                            drawSprite(data, oam, 0x01, 0, 8);
                            drawSprite(data, oam, 0x10, 8, 0);
                            drawSprite(data, oam, 0x11, 0, 0);
                        } else {
                            // 上下反転
                            drawSprite(data, oam, 0x00, 0, 8);
                            drawSprite(data, oam, 0x01, 8, 8);
                            drawSprite(data, oam, 0x10, 0, 0);
                            drawSprite(data, oam, 0x11, 8, 0);
                        }
                    } else if (oam->flags & 0x02) {
                        // 左右反転
                        drawSprite(data, oam, 0x00, 8, 0);
                        drawSprite(data, oam, 0x01, 0, 0);
                        drawSprite(data, oam, 0x10, 8, 8);
                        drawSprite(data, oam, 0x11, 0, 8);
                    } else {
                        // 反転無し
                        drawSprite(data, oam, 0x00, 0, 0);
                        drawSprite(data, oam, 0x01, 8, 0);
                        drawSprite(data, oam, 0x10, 0, 8);
                        drawSprite(data, oam, 0x11, 8, 8);
                    }
                } else {
                    // 8x8
                    drawSprite(data, oam, 0x00, 0, 0);
                }
            }
        }

        // draw FG
        cno = reg.cmap & 4 ? 1 : 0;
        bno = reg.cbank[cno];
        data = (unsigned char*)vm->bank->chr[bno];
        if (data) {
            vy = reg.fgY / 8;
            vx = reg.fgX / 8;
            for (i = 0; i < 32; i++) {
                for (j = 0; j < 32; j++) {
                    window[i * 32 + j] = vm->cpu->ram[0x7000 + (vy + i) * 64 + vx + j];
                }
            }
            dy = reg.fgY % 8;
            dx = reg.fgX % 8;
            for (y = 0; y < 32; y++) {
                for (x = 0; x < 32; x++) {
                    int cn = window[y * 32 + x];
                    if (!cn) continue; // do not draw $00
                    cn *= 64;
                    for (i = 0; i < 8; i++) {
                        for (j = 0; j < 8; j++) {
                            dptr = &data[cn + i * 8 + j];
                            if (*dptr) {
                                vx = x * 8 + dx + j;
                                vy = y * 8 + dy + i;
                                vram[vy * 256 + vx] = *dptr;
                            }
                        }
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

  private:
    inline void drawSprite(unsigned char* data, struct OAM* oam, unsigned char dp, unsigned char dx, unsigned char dy)
    {
        unsigned char p = oam->pattern + dp;
        unsigned char* dptr;
        unsigned char x, y, c, vx, vy;
        if (p) {
            dptr = &data[64 * p];
            if (oam->flags & 0x04) {
                if (oam->flags & 0x02) {
                    // 上下左右反転
                    for (y = 0; y < 8; y++) {
                        for (x = 0; x < 8; x++) {
                            c = dptr[y * 8 + x];
                            if (c) {
                                vx = oam->x + 7 - x + dx;
                                vy = oam->y + 7 - y + dy;
                                vram[vy * 256 + vx] = c;
                            }
                        }
                    }
                } else {
                    // 上下反転
                    for (y = 0; y < 8; y++) {
                        for (x = 0; x < 8; x++) {
                            c = dptr[y * 8 + x];
                            if (c) {
                                vx = oam->x + x + dx;
                                vy = oam->y + 7 - y + dy;
                                vram[vy * 256 + vx] = c;
                            }
                        }
                    }
                }
            } else if (oam->flags & 0x02) {
                // 左右反転
                for (y = 0; y < 8; y++) {
                    for (x = 0; x < 8; x++) {
                        c = dptr[y * 8 + x];
                        if (c) {
                            vx = oam->x + 7 - x + dx;
                            vy = oam->y + y + dy;
                            vram[vy * 256 + vx] = c;
                        }
                    }
                }
            } else {
                // 反転なし
                for (y = 0; y < 8; y++) {
                    for (x = 0; x < 8; x++) {
                        c = dptr[y * 8 + x];
                        if (c) {
                            vx = oam->x + x + dx;
                            vy = oam->y + y + dy;
                            vram[vy * 256 + vx] = c;
                        }
                    }
                }
            }
        }
    }
};
