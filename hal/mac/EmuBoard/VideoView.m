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
extern int emu_touching;
extern unsigned char emu_touchX;
extern unsigned char emu_touchY;

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *context);

@interface VideoView()
@property (nonatomic) VideoLayer* videoLayer;
@property CVDisplayLinkRef displayLink;
@property (nonatomic) NSTrackingArea* trankingArea;
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
        [self registerForDraggedTypes:@[NSFilenamesPboardType]];
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

- (void)updateTrackingAreas
{
    if (_trankingArea) [self removeTrackingArea:_trankingArea];
    NSTrackingAreaOptions opts = NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingEnabledDuringMouseDrag;
    _trankingArea = [[NSTrackingArea alloc] initWithRect:self.frame options:opts owner:self userInfo:nil];
    [self addTrackingArea:_trankingArea];
}

- (void)_updateTouchingPoint:(NSPoint)curPoint
{
    if (curPoint.x < 0) curPoint.x = 0;
    if (curPoint.y < 0) curPoint.y = 0;
    curPoint.y = self.frame.size.height - curPoint.y;
    curPoint.x *= 240.0 / self.frame.size.width;
    curPoint.y *= 240.0 / self.frame.size.height;
    if (239 <= curPoint.x) curPoint.x = 239;
    if (239 <= curPoint.y) curPoint.y = 239;
    emu_touchX = (unsigned char)(curPoint.x + 8);
    emu_touchY = (unsigned char)(curPoint.y + 8);
}

- (void)mouseDown:(NSEvent *)event
{
    emu_touching = 1;
    [self _updateTouchingPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
}

- (void)mouseUp:(NSEvent *)event
{
    emu_touching = 0;
    [self _updateTouchingPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
}

- (void)mouseMoved:(NSEvent *)event
{
    [self _updateTouchingPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
}

- (void)mouseDragged:(NSEvent *)event
{
    [self _updateTouchingPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
}

- (void)mouseExited:(NSEvent *)event
{
    [self _updateTouchingPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
}

- (void)keyDown:(NSEvent *)event
{
    unichar c = [event.charactersIgnoringModifiers characterAtIndex:0];
    switch (tolower(c)) {
        case 0xF700: emu_key_up = 1; break;
        case 0xF701: emu_key_down = 1; break;
        case 0xF702: emu_key_left = 1; break;
        case 0xF703: emu_key_right = 1; break;
        case 0x000d: emu_key_start = 1; break; // Enter
        case 0x0020: emu_key_select = 1; break; // Space
        case 0x0078: emu_key_b = 1; break; // X
        case 0x007A: emu_key_a = 1; break; // Z
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
        case 0x000d: emu_key_start = 0; break; // Enter
        case 0x0020: emu_key_select = 0; break; // Space
        case 0x0078: emu_key_b = 0; break; // X
        case 0x007A: emu_key_a = 0; break; // Z
        case 0x0072: emu_reset(); break;
    }
    // NSLog(@"keycode: %04X", (int)tolower(c));
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationCopy;
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender
{
    NSArray<NSString*>* files = [sender.draggingPasteboard propertyListForType:NSFilenamesPboardType];
    return files.count == 1; // 1ファイルのみ許可
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    if (_delegate) {
        for (NSString* file in [sender.draggingPasteboard propertyListForType:NSFilenamesPboardType]) {
            [_delegate videoView:self didDropFile:file];
        }
    }
    return YES;
}
    
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *context)
{
    [(__bridge VideoLayer *)context performSelectorOnMainThread:@selector(vsync) withObject:nil waitUntilDone:NO];
    return kCVReturnSuccess;
}


@end
