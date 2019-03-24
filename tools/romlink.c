// Copyright 2019, SUZUKI PLAN (MIT license)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vgs-mml-compiler/src/vgsmml.h"
#include "lz4/lib/lz4.h"

/* Bitmap情報ヘッダ */
struct BmpHead {
    int isize;             /* 情報ヘッダサイズ */
    int width;             /* 幅 */
    int height;            /* 高さ */
    unsigned short planes; /* プレーン数 */
    unsigned short bits;   /* 色ビット数 */
    unsigned int ctype;    /* 圧縮形式 */
    unsigned int gsize;    /* 画像データサイズ */
    int xppm;              /* X方向解像度 */
    int yppm;              /* Y方向解像度 */
    unsigned int cnum;     /* 使用色数 */
    unsigned int inum;     /* 重要色数 */
};

/* Wav情報ヘッダ */
struct WavHead {
    char riff[4];
    unsigned int fsize;
    char wave[4];
    char fmt[4];
    unsigned int bnum;
    unsigned short fid;
    unsigned short ch;
    unsigned int sample;
    unsigned int bps;
    unsigned short bsize;
    unsigned short bits;
    char data[4];
    unsigned int dsize;
};

unsigned char palette_data[1024];
int palette_loaded;

static int writeRom(FILE* fpW, void* data, int dataSize)
{
    // 最初にサイズを書く
    if (4 != fwrite(&dataSize, 1, 4, fpW)) {
        printf("file write error (%d)\n", ferror(fpW));
        return -1;
    }
    // 次にデータ本体を書く
    if (dataSize != fwrite(data, 1, dataSize, fpW)) {
        printf("file write error (%d)\n", ferror(fpW));
        return -1;
    }
    return 0;
}

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
    int ret = writeRom(fpW, result, resultSize);
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
    int rc = 0;
    FILE* fpR = NULL;
    char fh[14];
    unsigned char pal[1024];
    struct BmpHead dh;
    int i, j, n, y, x, a;
    char bmp[16384];
    char chr[16384];

    /* 読み込みファイルをオープン */
    rc++;
    if (NULL == (fpR = fopen(file, "rb"))) {
        fprintf(stderr, "I/O error");
        goto ENDPROC;
    }

    /* ファイルヘッダを読み込む */
    rc++;
    if (sizeof(fh) != fread(fh, 1, sizeof(fh), fpR)) {
        fprintf(stderr, "ERROR: invalid file header.\n");
        goto ENDPROC;
    }

    /* 先頭2バイトだけ読む */
    rc++;
    if (strncmp(fh, "BM", 2)) {
        fprintf(stderr, "ERROR: inuput file is not bitmap.\n");
        goto ENDPROC;
    }

    /* 情報ヘッダを読み込む */
    rc++;
    if (sizeof(dh) != fread(&dh, 1, sizeof(dh), fpR)) {
        fprintf(stderr, "ERROR: invalid bitmap file header.\n");
        goto ENDPROC;
    }

    /* 128x128でなければエラー扱い */
    rc++;
    if (128 != dh.width || 128 != dh.height) {
        fprintf(stderr, "ERROR: invalid input bitmap size. (128x128 only)");
        goto ENDPROC;
    }

    /* 8ビットカラー以外は弾く */
    rc++;
    if (8 != dh.bits) {
        fprintf(stderr, "ERROR: invalid color format. (8bit color only)\n");
        goto ENDPROC;
    }

    /* 無圧縮以外は弾く */
    rc++;
    if (dh.ctype) {
        fprintf(stderr, "ERROR: invalid compress type.\n");
        goto ENDPROC;
    }

    /* パレットを読み込む */
    rc++;
    if (sizeof(pal) != fread(pal, 1, sizeof(pal), fpR)) {
        fprintf(stderr, "ERROR: Could not read palette data.\n");
        goto ENDPROC;
    }
    if (!palette_loaded) {
        palette_loaded = 1;
        memcpy(palette_data, pal, sizeof(pal));
    }

    /* 画像データを上下反転しながら読み込む */
    rc++;
    for (i = 127; 0 <= i; i--) {
        if (128 != fread(&bmp[i * 128], 1, 128, fpR)) {
            fprintf(stderr, "ERROR: Could not read graphic data.\n");
            goto ENDPROC;
        }
    }

    /* Bitmap を CHR に変換 */
    for (n = 0; n < 256; n++) {
        x = n % 16 * 8;
        y = (n / 16) * 8;
        a = y * 128 + x;
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                chr[n * 64 + i * 8 + j] = bmp[a + i * 128 + j];
            }
        }
    }

    // LZ4で圧縮して書き込む
    rc = compressAndWrite(fpW, chr, sizeof(chr));

ENDPROC:
    fclose(fpR);
    return rc;
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
    // LZ4で圧縮せずに書き込む（VGSMMLでminizに圧縮済みなので多重圧縮しない）
    int ret = writeRom(fpW, data->data, data->size);
    vgsmml_free_bgm_data(data);
    return ret;
}

static int linkEFF(FILE* fpW, const char* file)
{

    FILE* fpR = NULL;
    int rc = 0;
    struct WavHead dh;
    char* data = NULL;
    char mh[4];

    /* 読み込みファイルをオープン */
    rc++;
    if (NULL == (fpR = fopen(file, "rb"))) {
        fprintf(stderr, "ERROR: Could not open: %s\n", file);
        goto ENDPROC;
    }

    /* 情報ヘッダを読み込む */
    rc++;
    if (sizeof(dh) != fread(&dh, 1, sizeof(dh), fpR)) {
        fprintf(stderr, "ERROR: Invalid file header.\n");
        goto ENDPROC;
    }

    /* 形式チェック */
    rc++;
    if (0 != strncmp(dh.riff, "RIFF", 4)) {
        fprintf(stderr, "ERROR: Not RIFF format.\n");
        goto ENDPROC;
    }
    rc++;
    if (0 != strncmp(dh.wave, "WAVE", 4)) {
        fprintf(stderr, "ERROR: Not WAVE format.\n");
        goto ENDPROC;
    }
    rc++;
    if (0 != strncmp(dh.fmt, "fmt ", 4)) {
        fprintf(stderr, "ERROR: Invalid format.\n");
        goto ENDPROC;
    }
    rc++;
    if (0 != strncmp(dh.data, "data", 4)) {
        fprintf(stderr, "ERROR: Invalid data.\n");
        goto ENDPROC;
    }

#if 0
    printf("Header of %s:\n", file);
    printf(" - Format: %d\n", dh.fid);
    printf(" - Channel: %dch\n", dh.ch);
    printf(" - Sample: %dHz\n", dh.sample);
    printf(" - Transform: %dbps\n", dh.bps);
    printf(" - Block-size: %dbyte\n", (int)dh.bsize);
    printf(" - Bit-rate: %dbit\n", (int)dh.bits);
    printf(" - PCM: %dbyte\n", (int)dh.dsize);
#endif

    rc++;
    if (22050 != dh.sample) {
        fprintf(stderr, "ERROR: Sampling rate is not 22050Hz.\n");
        goto ENDPROC;
    }
    rc++;
    if (1 != dh.ch) {
        fprintf(stderr, "ERROR: Sampling channel is not 1(mono).\n");
        goto ENDPROC;
    }
    rc++;
    if (16 != dh.bits) {
        fprintf(stderr, "ERROR: Sampling bit rate is not 16bit.\n");
        goto ENDPROC;
    }
    rc++;
    if (dh.sample * 2 != dh.bps) {
        fprintf(stderr, "ERROR: Invalid transform-rate(byte/sec).\n");
        goto ENDPROC;
    }

    /* 波形データを読む込む領域を確保する */
    rc++;
    if (NULL == (data = (char*)malloc(dh.dsize))) {
        fprintf(stderr, "ERROR: Memory allocation error.\n");
        goto ENDPROC;
    }

    /* 波形データを読み込む */
    rc++;
    if (dh.dsize != fread(data, 1, dh.dsize, fpR)) {
        fprintf(stderr, "ERROR: Could not read PCM data.\n");
        goto ENDPROC;
    }

    /* 波形データを書き込む（LZ4で圧縮しても小さくならない可能性が高いのでそのまま書き込む） */
    rc = writeRom(fpW, data, dh.dsize);

ENDPROC:
    if (data) {
        free(data);
    }
    if (fpR) {
        fclose(fpR);
    }
    return rc;
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
            return 6;
        }
    }

    // 最後にパレットデータを書き込む
    if (sizeof(palette_data) != fwrite(palette_data, 1, sizeof(palette_data), fpW)) {
        printf("ERROR: cannot write file: %s\n", argv[1]);
        fclose(fpW);
        return 7;
    }

    fclose(fpW);
    return 0;
}