//
//  EmuBoard.c
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
#include "EmuBoard.h"
#include "vgs8.h"
#include "vgsspu_al.h"

/**
 * この内容を書き換えればそれが画面に表示される
 * ※RGB565（16bit color）形式
 */
unsigned short emu_vram[VRAM_WIDTH * VRAM_HEIGHT];

static void* spu;
static std::mutex sound_lock;
static unsigned short sound_buffer[65536];
static volatile unsigned short sound_cursor;

int emu_key_up;
int emu_key_down;
int emu_key_left;
int emu_key_right;
int emu_key_a;
int emu_key_b;
int emu_key_select;
int emu_key_start;
int emu_touching;
unsigned char emu_touchX;
unsigned char emu_touchY;

static VGS8::VirtualMachine* vm;


void sound_proc(void* buffer, size_t size)
{
    while (sound_cursor < size / 2) usleep(100);
    std::unique_lock<std::mutex> lock(sound_lock);
    memcpy(buffer, sound_buffer, size);
    if (size <= sound_cursor) sound_cursor -= size;
    else sound_cursor = 0;
}

/**
 * 起動時に1回だけ呼び出される
 */
void emu_init(void* rom, size_t size)
{
    puts("emu_init");
    if (vm) return;
    spu = vgsspu_start2(22050, 16, 1, 5880, sound_proc);
    vm = new VGS8::VirtualMachine(rom, size);
}

void emu_reset()
{
    vm->reset();
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
    vm->setKey(0, emu_key_up, emu_key_down, emu_key_left, emu_key_right, emu_key_a, emu_key_b, emu_key_select, emu_key_start);
    vm->setTouch(emu_touching ? true : false, emu_touchX, emu_touchY);
    vm->tick();
    size_t size;
    memcpy(emu_vram, vm->getDisplay565(&size), sizeof(emu_vram));
    size_t pcmSize;
    void* pcm = vm->getPCM(&pcmSize);
    std::unique_lock<std::mutex> lock(sound_lock);
    if (pcmSize + sound_cursor < 65536) {
        memcpy(&sound_buffer[sound_cursor], pcm, pcmSize);
        sound_cursor += pcmSize / 2;
    }
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
    vgsspu_end(spu);
}
