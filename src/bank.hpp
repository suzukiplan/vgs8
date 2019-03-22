// Copyright 2019, SUZUKI PLAN (GPLv3 license)

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
        if (size < 8) return;
        if (strncmp((const char*)rom, "VGS8", 4)) return;
        int idx = 4;
        unsigned char prgN = ((unsigned char*)rom)[idx++];
        unsigned char chrN = ((unsigned char*)rom)[idx++];
        unsigned char bgmN = ((unsigned char*)rom)[idx++];
        unsigned char effN = ((unsigned char*)rom)[idx++];
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