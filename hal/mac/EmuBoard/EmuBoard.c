//
//  EmuBoard.c
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "EmuBoard.h"

/**
 * この内容を書き換えればそれが画面に表示される
 * ※RGB565（16bit color）形式
 */
unsigned short emu_vram[VRAM_WIDTH * VRAM_HEIGHT];

static int emu_initialized = 0;

/**
 * 起動時に1回だけ呼び出される
 */
void emu_init()
{
    puts("emu_init");
    if (emu_initialized) return;
    emu_initialized = 1;
}

/**
 * 画面の更新間隔（1秒間で60回）毎にこの関数がコールバックされる
 * この中で以下の処理を実行する想定:
 * 1. エミュレータのCPU処理を1フレーム分実行
 * 2. エミュレータの画面バッファの内容をvramへコピー
 */
void emu_vsync()
{
    if (!emu_initialized) return;

    // 表示を全クリア
    memset(emu_vram, 0, sizeof(emu_vram));

    // 画面が更新されていることが分かるように適当に点を打っておく
    int i;
    for (i = 0; i < 16384; i++) {
        int ptr = rand() % (VRAM_WIDTH * VRAM_HEIGHT);
        emu_vram[ptr] = (unsigned short)rand();
    }
}

/**
 * 終了時に1回だけ呼び出される
 * この中でエミュレータの初期化処理を実行する想定
 */
void emu_destroy()
{
    puts("emu_destroy");
    if (!emu_initialized) return;
    emu_initialized = 0;
}
