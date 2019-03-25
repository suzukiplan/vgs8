// Copyright 2019, SUZUKI PLAN (MIT license)
#include <sys/timeb.h>
#include <ctype.h>
#include "../src/vgs8.h"

static void invalidArgument(const char* msg)
{
    if (msg) printf("ERROR: %s\n", msg);
    puts("usage: vgsrun [-f max-execute-frames]");
    puts("              [-b bank addr]");
    puts("              target.rom");
    exit(1);
}

static int atoi65_bin(const char* text)
{
    int result = 0;
    for (; *text; text++) {
        result <<= 1;
        switch (*text) {
            case '0': result += 0; break;
            case '1': result += 1; break;
            default: invalidArgument("invalid binary value");
        }
    }
    return result;
}

static int atoi65_hex(const char* text)
{
    int result = 0;
    for (; *text; text++) {
        result <<= 4;
        switch (toupper(*text)) {
            case '0': result += 0; break;
            case '1': result += 1; break;
            case '2': result += 2; break;
            case '3': result += 3; break;
            case '4': result += 4; break;
            case '5': result += 5; break;
            case '6': result += 6; break;
            case '7': result += 7; break;
            case '8': result += 8; break;
            case '9': result += 9; break;
            case 'A': result += 10; break;
            case 'B': result += 11; break;
            case 'C': result += 12; break;
            case 'D': result += 13; break;
            case 'E': result += 14; break;
            case 'F': result += 15; break;
            default: invalidArgument("invalid hex value");
        }
    }
    return result;
}

static int atoi65(const char* text)
{
    if ('%' == *text) {
        text++;
        return atoi65_bin(text);
    } else if (0 == strncasecmp(text, "0b", 2)) {
        text += 2;
        return atoi65_bin(text);
    } else if ('$' == *text) {
        text++;
        return atoi65_hex(text);
    } else if (0 == strncasecmp(text, "0x", 2)) {
        text += 2;
        return atoi65_hex(text);
    } else {
        return atoi(text);
    }
}

static void breakCallback(VGS8::VirtualMachine* vm)
{
    char buf[1024];
    printf("COMMAND> ");
    while (fgets(buf, sizeof(buf) - 1, stdin)) {
        switch (toupper(buf[0])) {
            case 'C': return;  // continue
            case 'Q': exit(0); // exit
            case 'M': {        // show memory dump
                puts("TODO: memory dump (not implemented)");
                break;
            }
            default: {
                puts("M $ADDR: Memory dump");
                puts("C: Continue");
                puts("Q: Quit");
                printf("COMMAND> ");
            }
        }
    }
}

int main(int argc, char* argv[])
{
    // 引数チェック
    if (argc < 2) invalidArgument(NULL);
    int maxExecuteFrames = 1;
    bool useBreakPoint = false;
    unsigned char breakBank;
    unsigned short breakAddr;
    const char* targetRom = NULL;
    for (int i = 1; i < argc; i++) {
        if ('-' == argv[i][0]) {
            switch (argv[i][1]) {
                case 'f': {
                    if (argc <= ++i) invalidArgument(NULL);
                    maxExecuteFrames = atoi65(argv[i]);
                    break;
                }
                case 'b': {
                    if (useBreakPoint) invalidArgument(NULL);
                    if (argc <= ++i) invalidArgument(NULL);
                    breakBank = (unsigned char)atoi65(argv[i]);
                    if (argc <= ++i) invalidArgument(NULL);
                    breakAddr = (unsigned short)atoi65(argv[i]);
                    useBreakPoint = true;
                    break;
                }
                default: invalidArgument("unknown option");
            }
        } else {
            if (targetRom) invalidArgument("duplicate ROM specified");
            targetRom = argv[i];
        }
    }
    if (!targetRom) invalidArgument("ROM not specified");

    // ROMファイルを読み込む
    void* rom;
    long size;
    FILE* fp = fopen(targetRom, "rb");
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

    if (useBreakPoint) {
        vm.setBreakPoint(breakBank, breakAddr, breakCallback);
    }

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