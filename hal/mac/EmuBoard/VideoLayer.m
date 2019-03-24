//
//  VideoLayer.m
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#import "VideoLayer.h"
#import "constants.h"
#import "EmuBoard.h"

static unsigned short imgbuf[2][VRAM_WIDTH * VRAM_HEIGHT];
static CGContextRef img[2];
static volatile int bno;

@implementation VideoLayer

+ (id)defaultActionForKey:(NSString *)key
{
    return nil;
}

- (id)init
{
    if (self = [super init]) {
        self.opaque = NO;
        img[0] = nil;
        img[1] = nil;
        // create image buffer
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        NSInteger width = VRAM_VIEW_RIGHT - VRAM_VIEW_LEFT;
        NSInteger height = VRAM_VIEW_BOTTOM - VRAM_VIEW_TOP;
        for (int i = 0; i < 2; i++) {
            img[i] = CGBitmapContextCreate(imgbuf[i], width, height, 5, 2 * VRAM_WIDTH, colorSpace, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Little);
        }
        CFRelease(colorSpace);
    }
    return self;
}

- (void)drawVRAM
{
    bno = 1 - bno;
    unsigned short* buf = imgbuf[1 - bno];
    unsigned short c;
    unsigned char r, g ,b;
    int i = 0;
    int d = VRAM_WIDTH - VRAM_VIEW_WIDTH;
    for (int y = VRAM_VIEW_TOP; y < VRAM_VIEW_BOTTOM; y++) {
        int ptr = y * VRAM_WIDTH;
        for (int x = VRAM_VIEW_LEFT; x < VRAM_VIEW_RIGHT; x++) {
            c = emu_vram[ptr++];
            // カラー変換 (RGB565 -> RGB555)
            r = (unsigned char)((c & 0xf800) >> 11);
            g = (unsigned char)((c & 0x07c0) >> 6);
            b = (unsigned char)(c & 0x001f);
            // 表示バッファへコピー
            buf[i] = r;
            buf[i] <<= 5;
            buf[i] |= g;
            buf[i] <<= 5;
            buf[i] |= b;
            i++;
        }
        i += d;
    }
    CGImageRef cgImage = CGBitmapContextCreateImage(img[1-bno]);
    self.contents = (__bridge id)cgImage;
    CFRelease(cgImage);
}

- (void)dealloc
{
    for (int i = 0; i < 2; i++) {
        if (img[i] != nil) {
            CFRelease(img[i]);
            img[i] = nil;
        }
    }
}

@end
