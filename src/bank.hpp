// Copyright 2019, SUZUKI PLAN (GPLv3 license)
#include "lz4.h"

class Bank
{
  public:
    void* prg[256];
    void* chr[256];
    void* bgm[256];
    void* eff[256];

    Bank(const void* rom, size_t size)
    {
        clear();
        unsigned char* rp = (unsigned char*)rom;
        if (size < 8) return;
        if (strncmp((const char*)rp, "VGS8", 4)) return;
        int idx = 4;

        // 各バンクサイズを取得
        unsigned char prgN = rp[idx++];
        unsigned char chrN = rp[idx++];
        unsigned char bgmN = rp[idx++];
        unsigned char effN = rp[idx++];

        // extract PRG banks
        for (int i = 0; i < prgN; i++) {
            int sz;
            const size_t max = 0x4000; // 領域は常に16KB固定 (バンク切り替えを高速化することを優先)
            memcpy(&sz, &rp[idx], 4);
            idx += 4;
            if (prg[i] = malloc(max)) {
                memset(prg[i], 0, max);
                ::LZ4_decompress_safe((const char*)prg[i], (char*)&rp[idx], sz, max);
            }
            idx += sz;
        }

        // TODO: extract CHR banks
        // TODO: extract BGM banks
        // TODO: extract EFF banks
    }

    ~Bank()
    {
        int i;
        for (i = 0; i < 256; i++) {
            if (prg[i]) free(prg[i]);
            if (chr[i]) free(chr[i]);
            if (bgm[i]) free(bgm[i]);
            if (eff[i]) free(eff[i]);
        }
        clear();
    }

  private:
    void clear()
    {
        memset(prg, 0, sizeof(prg));
        memset(chr, 0, sizeof(chr));
        memset(bgm, 0, sizeof(bgm));
        memset(eff, 0, sizeof(eff));
    }
};