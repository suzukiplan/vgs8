// Copyright 2019, SUZUKI PLAN (MIT license)
// VGS cartridge system (BANK) emulator
#ifndef INCLUDE_BANK_HPP
#define INCLUDE_BANK_HPP

using namespace VGS8;

class Bank
{
  public:
    void* prg[256];
    void* chr[256];
    void* bgm[256];
    short* eff[256];
    int bgmSize[256];
    int effSize[256];
    unsigned char pal[1024];

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
            if (NULL != (prg[i] = malloc(max))) {
                memset(prg[i], 0, max);
                LZ4_decompress_safe((const char*)&rp[idx], (char*)prg[i], sz, max);
            }
            idx += sz;
        }

        // extract CHR banks
        for (int i = 0; i < chrN; i++) {
            int sz;
            const size_t max = 0x4000; // 領域は常に16KB固定
            memcpy(&sz, &rp[idx], 4);
            idx += 4;
            if (NULL != (chr[i] = malloc(max))) {
                memset(chr[i], 0, max);
                LZ4_decompress_safe((const char*)&rp[idx], (char*)chr[i], sz, max);
            }
            idx += sz;
        }

        // extract BGM banks
        for (int i = 0; i < bgmN; i++) {
            memcpy(&bgmSize[i], &rp[idx], 4);
            idx += 4;
            if (NULL != (bgm[i] = malloc(bgmSize[i]))) {
                memcpy(bgm[i], &rp[idx], bgmSize[i]);
            }
            idx += bgmSize[i];
        }

        // extract EFF banks
        for (int i = 0; i < effN; i++) {
            memcpy(&effSize[i], &rp[idx], 4);
            idx += 4;
            if (NULL != (eff[i] = (short*)malloc(effSize[i]))) {
                memcpy(eff[i], &rp[idx], effSize[i]);
            }
            idx += effSize[i];
            effSize[i] /= 2;
        }

        // extract palette data
        memcpy(pal, &rp[idx], 1024);
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

#endif
