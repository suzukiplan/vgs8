// Copyright 2019, SUZUKI PLAN (MIT license)
// VGS Audio Processing Unit emulator
#ifndef INCLUDE_APU_HPP
#define INCLUDE_APU_HPP
#include "vgs8.h"

using namespace VGS8;

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
    short soundBuffer1[367];
    short soundBuffer2[368];
    int soundBufferA;

  public:
    APU(VirtualMachine* vm)
    {
        this->vm = vm;
        vgsdec = vgsdec_create_context();
        soundBufferA = 0;
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
            updateBgmMasterVolume();
            reg.bgmPlaying = 1;
        } else {
            reg.bgmPlaying = 0;
        }
    }

    void pauseBgm()
    {
        reg.bgmPlaying = 0;
    }

    void resumeBgm()
    {
        if (vm->bank->bgm[reg.bgmCursor]) {
            reg.bgmPlaying = 1;
        }
    }

    bool isBgmPlaying()
    {
        return reg.bgmPlaying ? true : false;
    }

    void updateBgmMasterVolume()
    {
        vgsdec_set_value(vgsdec, VGSDEC_REG_VOLUME_RATE, 25500 / (255 - vm->cpu->ram[0x5604]));
    }

    void playEff(unsigned char n)
    {
        if (vm->bank->eff[n]) {
            reg.effPlaying[n] = 1;
            reg.effCursor[n] = 0;
        }
    }

    void stopEff(unsigned char n)
    {
        reg.effPlaying[n] = 0;
    }

    void execute()
    {
        // 1フレーム毎にバッファを367samplesと368samplesで切り替えることで音ズレの発生を防ぐ
        short* soundBuffer;
        size_t soundBufferSize;
        soundBufferA = 1 - soundBufferA;
        if (soundBufferA) {
            soundBuffer = soundBuffer1;
            soundBufferSize = sizeof(soundBuffer1);
        } else {
            soundBuffer = soundBuffer2;
            soundBufferSize = sizeof(soundBuffer2);
        }
        memset(soundBuffer, 0, soundBufferSize);
        // BGMをバッファに書き込む
        if (reg.bgmPlaying) {
            vgsdec_execute(vgsdec, soundBuffer, soundBufferSize);
            reg.bgmCurrentTime = vgsdec_get_value(vgsdec, VGSDEC_REG_TIME);
        }
        // 効果音をバッファに合成
        for (int i = 0; i < 256; i++) {
            if (reg.effPlaying[i]) {
                for (int j = 0; j < soundBufferSize / 2; j++) {
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
                        reg.effCursor[i] = 0;
                        reg.effPlaying[i] = 0;
                        break;
                    }
                }
            }
        }
    }

    short* getBuffer(size_t* size)
    {
        if (soundBufferA) {
            *size = sizeof(soundBuffer1);
            return soundBuffer1;
        } else {
            *size = sizeof(soundBuffer2);
            return soundBuffer2;
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

#endif
