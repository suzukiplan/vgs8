// Copyright 2019, SUZUKI PLAN (MIT license)
#include <sys/timeb.h>
#include "../src/vgs8.h"

int main(int argc, char* argv[])
{
    // 引数チェック
    if (argc < 2) {
        puts("usage: vgsrun game.rom [max-execute-frames]");
        return 1;
    }
    int maxExecuteFrames = 1;
    if (3 <= argc) {
        maxExecuteFrames = atoi(argv[2]);
        if (maxExecuteFrames < 1) {
            return 0;
        }
    }

    // ROMファイルを読み込む
    void* rom;
    long size;
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        puts("ERROR: cannot open the ROM file");
        return 255;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size < 1) {
        puts("ERROR: invalid file size");
        fclose(fp);
        return 255;
    }
    fseek(fp, 0, SEEK_SET);
    rom = malloc(size);
    if (!rom) {
        puts("ERROR: no memory");
        fclose(fp);
        return 255;
    }
    fread(rom, size, 1, fp);
    fclose(fp);

    // VGS8仮想マシンを作成
    VGS8::VirtualMachine vm(rom, size);

    for (int i = 0; i < maxExecuteFrames; i++) {
        struct timeb ts, te;
        ftime(&ts);
        unsigned int clocks = vm.tick();
        ftime(&te);
        long diff = ((long)(te.time - ts.time)) * 1000L + te.millitm - ts.millitm;
        printf("FRAME #%d END (%ldms, %uHz)\n", i + 1, diff, clocks);
    }
    return 0;
}