//
//  EmuBoard.h
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#include "constants.h"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned short emu_vram[VRAM_WIDTH * VRAM_HEIGHT];
void emu_init(void* rom, size_t size);
void emu_reset(void);
void emu_vsync(void);
void emu_destroy(void);

#ifdef __cplusplus
};
#endif
