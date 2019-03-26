# SUZUKI PLAN - Video Game System - 8bit (WIP)

__※まだ開発途中です（絶賛テスト中なので, まだバグが結構沢山ある筈 & 仕様はモリモリ破壊的変更されます）__

> _後述のWIP statusに全部チェックが付いた頃にようやくテスト完了でstableになります（stableになった後は破壊的変更はしない予定）_

- SUZUKI PLAN - Video Game System - 8bit (VGS8) is a newly designed 8 bit game machine with current technologies.
- This repository provides VGS8 emulator core module, SDK tools, HAL examples and example programs.
- The base architecture design of VGS8 is inspired by NES (;Nintendo Entertaiment System), but it does not have a compatibility.

## WIP status

- [x] [BANK(ROMのメモリマッパ)を実装](src/bank.hpp)
- [x] [CPUを実装](src/cpu.hpp)
- [x] [PPUを実装](src/ppu.hpp)
- [x] [APUを実装](src/apu.hpp)
- [x] [VGS8仮想マシンを実装](src/vgs8.h)
- [x] I/Oポートマップを完成させる (完全には完成していないが一旦最低限揃える)
- [ ] [macOS上で動作するエミュレータ（CocoaApp）](hal/mac)を作成
  - [x] [CPU](src/cpu.hpp)を動作させるHALを実装
  - [x] [PPU](src/ppu.hpp)を動作させるHALを実装
  - [ ] [APU](src/apu.hpp)を動作させるHALを実装
  - [ ] 固定のROMファイル（test.rom）ではなく起動時に任意ROMを選択できるようにする
- [x] コマンドライン・デバッガ ([vgsrun](tools/vgsrun.cpp)) 作成
  - [x] 全命令の動的逆アセンブル & レジスタダンプに対応
  - [x] break point機能の実装
  - [x] メモリダンプの実装
  - [x] キー入力の実装
- [ ] examplesを色々と追加しながら一通りの命令+機能をテスト
  - [x] [Hello, World](examples/hello)
  - [x] [FG+スクロールを使ったサンプル](examples/hello_fg)
  - [x] [スプライト + JoyPadを使ったサンプル](examples/hello_sp)
  - [ ] 音声を使ったサンプル
  - [ ] CHRバンク切り替えのサンプル
  - [ ] PRGバンク切り替えのサンプル
  - [ ] [COSMIC SHOOTER](https://github.com/suzukiplan/stg-for-nes)をVGS8に移植
  - [ ] [ファミコン版Battle Marine](https://github.com/suzukiplan/battle-marine-fc)をVGS8に移植
- [x] リファレンスマニュアル作成（[コレ](https://github.com/suzukiplan/mgp-fc)のリファレンス編のVGS8版）

> _以下, やるかやらないかまだ分からないextra (やるとしても恐らくWIP外した後)_

- [ ] Windows用HALを作成
- [ ] Linux用HALを作成
- [ ] iOS用HALを作成
- [ ] Android用HALを作成
- [ ] 6502マシン語ゲームプログラミング（VGS8版）の本を執筆

## Setup

### Pre requests

- GNU make
- CLANG
- cc65

#### Windows

(略)

#### macOS

- GNU make, CLANG are installed via `XCODE Command Line Tools`
- cc65 is installable with HomeBrew

```
brew install cc65
```

#### Linux

(略)

### Setup

```
git clone https://github.com/suzukiplan/vgs8
cd vgs8
git submodule update --init --recursive
make
```

make command will build the following modules:

- ROM linkage tool（[romlink](tools/romlink.c))
- ROM debugger tool ([vgsrun](tools/vgsrun.cpp))
- example ROMs

Please copy [src/*](src) files to your project, if you make an original VGS8 emulator.

## Specification

- CPU: MOS6502 8MHz
  - program 0: $8000 ~ $BFFF (16KB)
  - program 1: $C000 ~ $FFFF (16KB)
  - switchable 255 banks (PRG bank)
- PPU
  - character data: 8bit color, 128x128px x 2 pages
  - switchable 255 banks (CHR bank)
  - 1 character: 8x8 or 16x16 <sup>*16x16 is sprite only</sup>
  - max sprites: 256 (no holizontal limit)
  - BG (back of sprite) & FG (front of sprite)
  - BG/FG nametable size: 512x512px (64x64 characters)
  - screen size: 256x256px <sup>*the top, bottom, left, and right ends 8px are masks)</sup>
- APU
  - 255 BGM banks & 255 EFF banks
  - BGM: same as VGS2 <sup>*see the [VGS BGM Decoder](https://github.com/suzukiplan/vgs-bgm-decoder), [VGS MML Compiler](https://github.com/suzukiplan/vgs-mml-compiler)</sup>
  - EFF: same as VGS2 <sup>*PCM of 22050Hz, 16bit, 1ch</sup>
- No battery backup features <i>(SAVE/LOAD features are usable as hardware function)</i>

## Memory map

|Address|Usage|
|---|---|
|$0000〜$00FF|Zero page|
|$0100〜$01FF|Stack|
|$0200〜$4FFF|RAM|
|$5000〜$53FF|Sprite OAM (4x256)|
|$5400|CPU I/O port (RW): PRG Bank of $8000〜$BFFF|
|$5401|CPU I/O port (RW): PRG Bank of $C000〜$FFFF|
|$5402|PPU I/O port (RW): CHR Bank of 0|
|$5403|PPU I/O port (RW): CHR Bank of 1|
|$5404|PPU I/O port (RW): CMAP register `-----FBS`|
|$5405|PPU I/O port (RW): Background Color|
|$5406〜$5409|PPU I/O port (RW): FG/BG window positions|
|$540A〜$540D|PPU I/O port (W): FG/BG scroll|
|$5500|APU I/O port (W): play EFF|
|$5501|APU I/O port (W): stop EFF|
|$5600|APU I/O port (W): play BGM|
|$5601|APU I/O port (W): pause BGM|
|$5602|APU I/O port (W): resume BGM|
|$5603|APU I/O port (R): BGM playing status|
|$5700|JoyPad I/O port (R): read 1P JoyPad status|
|$5701|JoyPad I/O port (R): read 2P JoyPad status|
|$5A00|DMA I/O port (RW): Set base page|
|$5A01|DMA I/O port (W): execute memset|
|$5A02|DMA I/O port (W): execute memcpy|
|$5BFF|CPU I/O port (R): update VRAM request|
|$5C00〜$5FFF|Palette|
|$6000〜$6FFF|BG nametable (64x64)|
|$7000〜$7FFF|FG nametable (64x64)|
|$8000〜$BFFF|Program 0|
|$C000〜$FFFF|Program 1|

## Manual

see the [MANUAL.md](MANUAL.md)

## License

[MIT](LICENSE.txt)
