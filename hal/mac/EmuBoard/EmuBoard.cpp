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
#include "vgs8.h"

/**
 * この内容を書き換えればそれが画面に表示される
 * ※RGB565（16bit color）形式
 */
unsigned short emu_vram[VRAM_WIDTH * VRAM_HEIGHT];

static VGS8::VirtualMachine* vm;

/**
 * 起動時に1回だけ呼び出される
 */
void emu_init(void* rom, size_t size)
{
    puts("emu_init");
    if (vm) return;
    vm = new VGS8::VirtualMachine(rom, size);
}

/**
 * 画面の更新間隔（1秒間で60回）毎にこの関数がコールバックされる
 * この中で以下の処理を実行する想定:
 * 1. エミュレータのCPU処理を1フレーム分実行
 * 2. エミュレータの画面バッファの内容をvramへコピー
 */
void emu_vsync()
{
    if (!vm) return;
    vm->tick();
    size_t size;
    memcpy(emu_vram, vm->getDisplay555(&size), sizeof(emu_vram));
}

/**
 * 終了時に1回だけ呼び出される
 * この中でエミュレータの初期化処理を実行する想定
 */
void emu_destroy()
{
    puts("emu_destroy");
    if (!vm) return;
    delete vm;
    vm = NULL;
}
