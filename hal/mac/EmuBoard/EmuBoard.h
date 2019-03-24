//
//  EmuBoard.h
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#include "constants.h"
extern unsigned short emu_vram[VRAM_WIDTH * VRAM_HEIGHT];
void emu_init(void);
void emu_vsync(void);
void emu_destroy(void);
