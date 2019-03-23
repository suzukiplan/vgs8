// Copyright 2019, SUZUKI PLAN (GPLv3 license)

class APU
{
    struct Register {
        unsigned char bgmPlaying;
        unsigned char bgmCursor;
        unsigned char reserved1;
        unsigned char reserved2;
        unsigned int bgmCurrentTime;
        unsigned char effPlaying[256];
        unsigned int effCursor[256];
    };

  private:
    VirtualMachine* vm;
    void* vgsdec;
    struct Register reg;
    short soundBuffer[22050 / 60];

  public:
    APU(VirtualMachine* vm)
    {
        this->vm = vm;
        vgsdec = vgsdec_create_context();
        reset();
    }

    ~APU()
    {
        vgsdec_release_context(vgsdec);
        vgsdec = NULL;
    }

    void reset()
    {
        memset(&reg, 0, sizeof(reg));
    }

    void playBgm(unsigned char n)
    {
        reg.bgmCursor = n;
        void* data = vm->bank->bgm[n];
        if (data) {
            vgsdec_load_bgm_from_memory(vgsdec, data, vm->bank->bgmSize[n]);
            reg.bgmPlaying = 1;
        } else {
            reg.bgmPlaying = 0;
        }
    }

    void playEff(unsigned char n)
    {
        if (vm->bank->eff[n]) {
            reg.effPlaying[n] = 1;
            reg.effCursor[n] = 0;
        }
    }

    void execute()
    {
        // BGMをバッファに書き込む
        if (reg.bgmPlaying) {
            vgsdec_execute(vgsdec, soundBuffer, sizeof(soundBuffer));
            reg.bgmCurrentTime = vgsdec_get_value(vgsdec, VGSDEC_REG_TIME);
        }
        // 効果音をバッファに合成
        for (int i = 0; i < 256; i++) {
            if (reg.effPlaying[i]) {
                for (int j = 0; j < sizeof(soundBuffer) / 2; j++) {
                    if (reg.effCursor[i] < vm->bank->effSize[i]) {
                        int w = vm->bank->eff[i][reg.effCursor[i]];
                        w += soundBuffer[j];
                        if (32767 < w) {
                            w = 32767;
                        } else if (w < -32768) {
                            w = -32768;
                        }
                        soundBuffer[j] = (short)w;
                        reg.effCursor[i]++;
                    } else {
                        reg.effPlaying[i] = 0;
                        break;
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
        if (reg.bgmPlaying) {
            playBgm(reg.bgmCursor);
            if (reg.bgmPlaying) {
                vgsdec_set_value(vgsdec, VGSDEC_REG_TIME, reg.bgmCurrentTime);
            }
        }
        return sizeof(reg);
    }
};