// ROM LINK
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vgs-mml-compiler/src/vgsmml.h"
#include "lz4/lib/lz4.h"

static int compressAndWrite(FILE* fpW, void* data, size_t dataSize)
{
    // LZ4で圧縮
    const int maxCompressedSize = LZ4_compressBound(0x4000);
    int resultSize = 0;
    void* result = malloc(maxCompressedSize);
    if (!result) {
        puts("no memory");
        return -1;
    }
    const int compressedSize = LZ4_compress_default((const char*)data,
                                                    (char*)result,
                                                    (int)dataSize,
                                                    maxCompressedSize);
    if (0 < compressedSize) {
        resultSize = compressedSize;
    } else {
        puts("compress error");
        free(result);
        return -1;
    }
    // 最初にサイズを書く
    if (4 != fwrite(&resultSize, 1, 4, fpW)) {
        printf("file write error (%d)\n", ferror(fpW));
        free(result);
        return -1;
    }
    // 次にデータ本体を書く
    if (resultSize != fwrite(result, 1, resultSize, fpW)) {
        printf("file write error (%d)\n", ferror(fpW));
        free(result);
        return -1;
    }
    free(result);
    return 0;
}

static int linkBIN(FILE* fpW, const char* file)
{
    // ファイルを読み込む
    void* fileData = NULL;
    long fileSize = 0;
    FILE* fpR = fopen(file, "rb");
    if (NULL == fpR) {
        return -1;
    }
    fseek(fpR, 0, SEEK_END);
    fileSize = ftell(fpR);
    if (fileSize < 1 || 0x4000 < fileSize) {
        printf("invalid file size: %ld\n", fileSize);
        fclose(fpR);
        return -1;
    }
    fseek(fpR, 0, SEEK_SET);
    fileData = malloc(fileSize);
    if (!fileData) {
        puts("no memory");
        fclose(fpR);
        return -1;
    }
    if (fileSize != fread(fileData, 1, fileSize, fpR)) {
        printf("file read error (%d)\n", ferror(fpR));
        fclose(fpR);
        free(fileData);
        return -1;
    }
    fclose(fpR);
    // LZ4で圧縮して書き込む
    int ret = compressAndWrite(fpW, fileData, fileSize);
    free(fileData);
    return ret;
}

static int linkCHR(FILE* fpW, const char* file)
{
    return -1; // TODO: not implemented
}

static int linkBGM(FILE* fpW, const char* file)
{
    // MMLをコンパイル
    struct VgsMmlErrorInfo err;
    struct VgsBgmData* data = vgsmml_compile_from_file(file, &err);
    if (!data) {
        printf("mml compile error (code=%d, line=%d, msg=%s)\n", err.code, err.line, err.message);
        return -1;
    }
    // LZ4で圧縮して書き込む
    int ret = compressAndWrite(fpW, data->data, data->size);
    vgsmml_free_bgm_data(data);
    return ret;
}

static int linkEFF(FILE* fpW, const char* file)
{
    return -1; // TODO: not implemented
}

int main(int argc, char* argv[])
{
    char* prg[256];
    char* chr[256];
    char* bgm[256];
    char* eff[256];
    int prgIdx = 0;
    int chrIdx = 0;
    int bgmIdx = 0;
    int effIdx = 0;
    int i;
    unsigned char c;
    FILE* fpW;
    FILE* fpR;
    long size;
    void* data;

    // コマンドライン引数をチェック
    if (argc < 3) {
        puts("usage: romlink output.rom input_file1 [input_file2 [...input_fileN]]");
        puts("input extras:");
        puts("- *.bin: PRG bank data (assembled binary file)");
        puts("- *.bmp: CHR bank data (128x128 of 8bit color bitmap)");
        puts("- *.mml: BGM bank data (VGS mml text)");
        puts("- *.wav: EFF bank data (22050Hz/16bi/1ch wav file)");
        return 1;
    }

    // 入力ファイルのチェック
    for (i = 2; i < argc; i++) {
        char* ext = strrchr(argv[i], '.');
        if (!ext) {
            printf("ERROR: invalid file specified: %s\n", argv[i]);
            return 1;
        }
        if (0 == strcasecmp(ext, ".bin")) {
            if (255 <= prgIdx) {
                printf("ERROR: too meny prg banks");
                return 1;
            }
            prg[prgIdx++] = argv[i];
        }
        if (0 == strcasecmp(ext, ".bmp")) {
            if (255 <= chrIdx) {
                printf("ERROR: too meny CHR banks");
                return 1;
            }
            chr[chrIdx++] = argv[i];
        }
        if (0 == strcasecmp(ext, ".mml")) {
            if (255 <= bgmIdx) {
                printf("too meny BGM banks");
                return 1;
            }
            bgm[bgmIdx++] = argv[i];
        }
        if (0 == strcasecmp(ext, ".wav")) {
            if (255 <= prgIdx) {
                printf("too meny EFF banks");
                return 1;
            }
            eff[effIdx++] = argv[i];
        }
    }

    // ヘッダ情報を書き出す
    fpW = fopen(argv[1], "wb");
    if (NULL == fpW) {
        printf("ERROR: cannot open file: %s\n", argv[1]);
        return 2;
    }
    if (4 != fwrite("VGS8", 1, 4, fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 2;
    }
    c = prgIdx & 0xFF;
    if (1 != fwrite(&c, 1, 1, fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 2;
    }
    c = chrIdx & 0xFF;
    if (1 != fwrite(&c, 1, 1, fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 2;
    }
    c = bgmIdx & 0xFF;
    if (1 != fwrite(&c, 1, 1, fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 2;
    }
    c = effIdx & 0xFF;
    if (1 != fwrite(&c, 1, 1, fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 2;
    }

    // PRGバンクデータをリンク
    for (i = 0; i < prgIdx; i++) {
        if (linkBIN(fpW, prg[i])) {
            printf("ERROR: cannot link PRG: %s\n", prg[i]);
            fclose(fpW);
            return 3;
        }
    }

    // CHRバンクデータをリンク
    for (i = 0; i < chrIdx; i++) {
        if (linkCHR(fpW, chr[i])) {
            printf("ERROR: cannot link CHR: %s\n", chr[i]);
            fclose(fpW);
            return 4;
        }
    }

    // BGMバンクデータをリンク
    for (i = 0; i < bgmIdx; i++) {
        if (linkBGM(fpW, bgm[i])) {
            printf("ERROR: cannot link BGM: %s\n", bgm[i]);
            fclose(fpW);
            return 5;
        }
    }

    // EFFバンクデータをリンク
    for (i = 0; i < effIdx; i++) {
        if (linkEFF(fpW, eff[i])) {
            printf("ERROR: cannot link EFF: %s\n", eff[i]);
            fclose(fpW);
            return 5;
        }
    }

    fclose(fpW);
    return 0;
}