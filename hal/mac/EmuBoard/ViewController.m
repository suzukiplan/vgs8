//
//  ViewController.m
//  EmuBoard
//
//  Created by 鈴木　洋司　 on 2018/12/30.
//  Copyright © 2018年 SUZUKIPLAN. All rights reserved.
//

#import "ViewController.h"
#import "VideoView.h"
#import "constants.h"
#import "EmuBoard.h"

@interface ViewController() <NSWindowDelegate, VideoViewDelegate>
@property (nonatomic) VideoView* video;
@property (nonatomic) BOOL isFullScreen;
@property (nonatomic) CGFloat currentWindowWidth;
@property (nonatomic) CGFloat currentWindowHeight;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    NSString* romFile = [[NSBundle mainBundle] pathForResource:@"init" ofType:@"rom"];
    NSData* rom = [NSData dataWithContentsOfFile:romFile];
    emu_init((void*)rom.bytes, rom.length);
    [self _setupDefaultWindowSize];
    CALayer *layer = [CALayer layer];
    [layer setBackgroundColor:CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0)];
    [self.view setWantsLayer:YES];
    [self.view setLayer:layer];
    _video = [[VideoView alloc] initWithFrame:[self calcVramRect]];
    _video.delegate = self;
    [self.view addSubview:_video];
    [self.view.window makeFirstResponder:_video];
}

- (void)_setupDefaultWindowSize
{
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    CGFloat defaultViewWidth = [(NSNumber*)[defaults valueForKey:@"defaultViewWidth"] floatValue];
    if (!defaultViewWidth) defaultViewWidth = VRAM_VIEW_WIDTH * 1.5;
    CGFloat defaultViewHeight = [(NSNumber*)[defaults valueForKey:@"defaultViewHeight"] floatValue];
    if (!defaultViewHeight) defaultViewHeight = VRAM_VIEW_HEIGHT * 1.5;
    _currentWindowWidth = defaultViewWidth;
    _currentWindowHeight = defaultViewHeight;
    [self.view.window setContentSize:NSMakeSize(defaultViewWidth, defaultViewHeight)];
    self.view.frame = CGRectMake(0, 0, defaultViewWidth, defaultViewHeight);
}

- (void)viewWillAppear
{
    self.view.window.delegate = self;
}

- (void)windowDidResize:(NSNotification *)notification
{
    _video.frame = [self calcVramRect];
    if (_isFullScreen) return;
    if (_currentWindowWidth != self.view.frame.size.width || _currentWindowHeight != self.view.frame.size.height) {
        _currentWindowWidth = self.view.frame.size.width;
        _currentWindowHeight = self.view.frame.size.height;
        NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
        [defaults setValue:@(_currentWindowWidth) forKey:@"defaultViewWidth"];
        [defaults setValue:@(_currentWindowHeight) forKey:@"defaultViewHeight"];
    }
}

- (NSRect)calcVramRect
{
    // 幅を16とした時の高さのアスペクト比を計算
    CGFloat aspectY = VRAM_VIEW_HEIGHT / (VRAM_VIEW_WIDTH / 16.0);
    // window中央にVRAMをaspect-fitで描画
    if (self.view.frame.size.height < self.view.frame.size.width) {
        CGFloat height = self.view.frame.size.height;
        CGFloat width = height / aspectY * 16;
        CGFloat x = (self.view.frame.size.width - width) / 2;
        return NSRectFromCGRect(CGRectMake(x, 0, width, height));
    } else {
        CGFloat width = self.view.frame.size.width;
        CGFloat height = width / 16 * aspectY;
        CGFloat y = (self.view.frame.size.height - height) / 2;
        return NSRectFromCGRect(CGRectMake(0, y, width, height));
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    // Update the view, if already loaded.
}

- (void)dealloc
{
    emu_destroy();
}

-(void)menuOpenRomFile:(id)sender
{
    __weak NSOpenPanel* panel = [NSOpenPanel openPanel];
    panel.allowsMultipleSelection = NO;
    panel.canChooseDirectories = NO;
    panel.canCreateDirectories = YES;
    panel.canChooseFiles = YES;
    panel.allowedFileTypes = @[@"rom", @"vgs8"];
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        if (!result) return;
        NSData* data = [NSData dataWithContentsOfURL:panel.URL];
        if (!data) return;
        emu_reload(data.bytes, data.length);
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:panel.URL];
    }];
}

-(void)menuReset:(id)sender
{
    emu_reset();
}

- (void)menuViewSize1x:(id)sender
{
    if (_isFullScreen) return;
    [self.view.window setContentSize:NSMakeSize(VRAM_VIEW_WIDTH * 1, VRAM_VIEW_HEIGHT * 1)];
}

- (void)menuViewSize2x:(id)sender
{
    if (_isFullScreen) return;
    [self.view.window setContentSize:NSMakeSize(VRAM_VIEW_WIDTH * 2, VRAM_VIEW_HEIGHT * 2)];
}

- (void)menuViewSize3x:(id)sender
{
    if (_isFullScreen) return;
    [self.view.window setContentSize:NSMakeSize(VRAM_VIEW_WIDTH * 3, VRAM_VIEW_HEIGHT * 3)];
}

- (void)menuViewSize4x:(id)sender
{
    if (_isFullScreen) return;
    [self.view.window setContentSize:NSMakeSize(VRAM_VIEW_WIDTH * 4, VRAM_VIEW_HEIGHT * 4)];
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    _isFullScreen = YES;
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    _isFullScreen = NO;
}

- (void)videoView:(VideoView *)view didDropFile:(NSString *)file
{
    NSData* data = [NSData dataWithContentsOfFile:file];
    if (!data) return;
    emu_reload(data.bytes, data.length);
}

@end
