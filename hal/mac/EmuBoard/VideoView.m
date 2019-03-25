//
//  VideoView.m
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#import "VideoView.h"
#import "VideoLayer.h"
#import "EmuBoard.h"
#import "constants.h"

extern int emu_key_up;
extern int emu_key_down;
extern int emu_key_left;
extern int emu_key_right;
extern int emu_key_a;
extern int emu_key_b;
extern int emu_key_select;
extern int emu_key_start;

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *context);

@interface VideoView()
@property (nonatomic) VideoLayer* videoLayer;
@property CVDisplayLinkRef displayLink;
@end

@implementation VideoView

+ (Class)layerClass
{
    return [VideoLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]) != nil) {
        [self setWantsLayer:YES];
        _videoLayer = [VideoLayer layer];
        [self setLayer:_videoLayer];
        CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
        CVDisplayLinkSetOutputCallback(_displayLink, MyDisplayLinkCallback, (__bridge void *)self);
        CVDisplayLinkStart(_displayLink);
    }
    return self;
}

- (void)releaseDisplayLink
{
    if (_displayLink) {
        CVDisplayLinkStop(_displayLink);
        CVDisplayLinkRelease(_displayLink);
        _displayLink = nil;
    }
}

- (void)vsync
{
    emu_vsync();
    [self.videoLayer drawVRAM];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent *)event
{
    unichar c = [event.charactersIgnoringModifiers characterAtIndex:0];
    switch (tolower(c)) {
        case 0xF700: emu_key_up = 1; break;
        case 0xF701: emu_key_down = 1; break;
        case 0xF702: emu_key_left = 1; break;
        case 0xF703: emu_key_right = 1; break;
        case 0x000d: emu_key_a = 1; break;
        case 0x0020: emu_key_b = 1; break;
        case 0x0078: emu_key_select = 1; break;
        case 0x007A: emu_key_start = 1; break;
    }
}

- (void)keyUp:(NSEvent *)event
{
    unichar c = [event.charactersIgnoringModifiers characterAtIndex:0];
    switch (tolower(c)) {
        case 0xF700: emu_key_up = 0; break;
        case 0xF701: emu_key_down = 0; break;
        case 0xF702: emu_key_left = 0; break;
        case 0xF703: emu_key_right = 0; break;
        case 0x000d: emu_key_a = 0; break;
        case 0x0020: emu_key_b = 0; break;
        case 0x0078: emu_key_select = 0; break;
        case 0x007A: emu_key_start = 0; break;
    }
}
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *context)
{
    [(__bridge VideoLayer *)context performSelectorOnMainThread:@selector(vsync) withObject:nil waitUntilDone:NO];
    return kCVReturnSuccess;
}


@end
