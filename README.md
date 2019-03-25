# SUZUKI PLAN - Video Game System - 8bit (WIP)

※まだ開発途中です（絶賛テスト中なので, まだバグが結構沢山ある筈です）

- SUZUKI PLAN - Video Game System - 8bit (VGS8) は, 現代向けに再設計された新しい8bitゲーム機です
- このリポジトリでは, VGS8コアシステムのエミュレータ（HALを省略した形）とVGS用ゲーム開発に必要なSDKを提供します
- VGS8の基礎アーキテクチャは, 任天堂のファミリーコンピュータ（ファミコン）から着想を得て設計していますが, ファミコンとの互換性はありません

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
- [ ] コマンドライン・デバッガ作成
- [ ] examplesを色々と追加しながら一通りの命令+機能をテスト
  - [x] [Hello, World](examples/hello)
  - [ ] FGを使ったサンプル
  - [ ] スクロールのサンプル
  - [ ] スプライト + JoyPadを使ったサンプル
  - [ ] 音声を使ったサンプル
  - [ ] CHRバンク切り替えのサンプル
  - [ ] PRGバンク切り替えのサンプル
  - [ ] [COSMIC SHOOTER](https://github.com/suzukiplan/stg-for-nes)をVGS8に移植
  - [ ] [ファミコン版Battle Marine](https://github.com/suzukiplan/battle-marine-fc)をVGS8に移植
- [x] リファレンスマニュアル作成（[コレ](https://github.com/suzukiplan/mgp-fc)のリファレンス編のVGS8版）

> _以下、やるかやらないかまだ分からないextra (やるとしても恐らくWIP外した後)_

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

- GNU make, CLANG は XCODE Command Line Tools をインストール
- cc65 は HomeBrew でインストールできます

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

- 上記を実行することで, ROMをリンクするツール（[romlink](tools/romlink.c)）の生成とVGS8コアのテストが実行されます
- 各自が実装するHALには [src](src) ディレクトリ以下のモジュールを取り込んで使用します

## Features

- CPU: MOS6502 8MHz
  - プログラムは, 最大16KB x 255バンク
  - ROMのPRGバンクエリアから, RAMの $8000 or $C000 にロードする
  - ファミコンのNxROM(iNES mapper2)と同様, 動的なバンク切り替えが可能
- PPU
  - キャラクタデータ: 8bitカラー (256色) 128x128ピクセル x 2枚 (255個のCHR bankから割り当て)
  - キャラクタサイズ: 8x8 or 16x16ピクセル
  - スプライト表示数: 256個 (水平上限無し)
  - BG4画面分 + FG4画面分 (BGはスプライトの背面, FGはスプライトの前面に表示される & ミラーは無し)
  - 画面サイズ: 240x240ピクセル (座標系は256x256で, 上下左右の端8pxがmask)
  - パレットは256色パレットひとつのみ（ここはファミコンではなくVGS仕様に準拠）
- APU (ココはVGS2と同じ仕様)
  - 255個のBGM, 255個のSE
  - BGMはVGSと同等（波形メモリ音源）
  - SEはVGSと同等（PCM）
- バッテリーバックアップ機能は無い（セーブ/ロードはH/W機能として提供）

## How to make your GAME

1. cc65を使って最大256個のプログラム `(*.bin)` をアセンブル（1つのプログラムの最大サイズ: 16KB）
2. 任意のグラフィックエディタで 128x128 サイズの 256色bitmapファイル `(*.bmp)` を最大256個作成
3. 任意の波形エディタで 22050Hz, 16bit, 1ch の効果音ファイル `(*.wav)` を最大256個作成
4. 任意のテキストエディタで [VGSのMML形式ファイル](https://github.com/suzukiplan/vgs-mml-compiler/blob/master/MML-ja.md) `(*.mml)` を最大256個作成
5. [romlink](tools/romlink.c)で, `*.bin` , `*.bmp` , `*.wav` , `*.mml` をリンク

> 以下のファイルを用いたゲームの [romlink](tools/romlink.c) をする例を示します。
>
> - program1.bin (プログラム1)
> - program2.bin (プログラム2)
> - program3.bin (プログラム3)
> - sprite.bmp (スプライト)
> - bg1.bmp (背景1)
> - bg2.bmp (背景2)
> - bg3.bmp (背景3)
> - eff1.wav (効果音1)
> - eff2.wav (効果音2)
> - eff3.wav (効果音3)
> - bgm1.mml (BGM1)
> - bgm2.mml (BGM2)
> - bgm3.mml (BGM3)
>
> ```
> romlink mygame.rom \
>     program1.bin program2.bin program3.bin \
>     sprite.bmp bg1.bmp bg2.bmp bg3.bmp \
>     eff1.wav eff2.wav eff3.wav \
>     bgm1.mml bgm2.mml bgm3.mml
> ```
>
> バンク番号は [romlink](tools/romlink.c) のコマンドライン引数指定順で決まります。

## How to debug your GAME (WIP)

_WIP: まだ全部の命令の逆アセンブルに対応していません_

[vgsrun](tools/vgsrun.cpp)を用いて, Terminal上でゲームの簡易デバッグが可能です。
[vgsrun](tools/vgsrun.cpp)は, 任意フレーム数ゲームROMを実行時のマシン語を動的に逆アセンブルして表示しつつ, 各命令実行後のレジスタ値を確認できます。

以下、[examplesのHello, World!](examples/hello/hello.asm)をvgsrunで実行した例を示します。

```
$ bin/vgsrun examples/hello/hello.rom 
$8000: LDA #$00                       (a=$00, x=$00, y=$00, s=$00, p=$02)
$8002: STA $5404                      (a=$00, x=$00, y=$00, s=$00, p=$02)
$8005: LDX #$00                       (a=$00, x=$00, y=$00, s=$00, p=$02)
$8007: LDA $8018, X                   (a=$48, x=$00, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$48, x=$00, y=$00, s=$00, p=$00)
$800D: INX                            (a=$48, x=$01, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$48, x=$01, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$48, x=$01, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$65, x=$01, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$65, x=$01, y=$00, s=$00, p=$00)
$800D: INX                            (a=$65, x=$02, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$65, x=$02, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$65, x=$02, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$6C, x=$02, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$6C, x=$02, y=$00, s=$00, p=$00)
$800D: INX                            (a=$6C, x=$03, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$6C, x=$03, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$6C, x=$03, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$6C, x=$03, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$6C, x=$03, y=$00, s=$00, p=$00)
$800D: INX                            (a=$6C, x=$04, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$6C, x=$04, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$6C, x=$04, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$6F, x=$04, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$6F, x=$04, y=$00, s=$00, p=$00)
$800D: INX                            (a=$6F, x=$05, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$6F, x=$05, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$6F, x=$05, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$2C, x=$05, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$2C, x=$05, y=$00, s=$00, p=$00)
$800D: INX                            (a=$2C, x=$06, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$2C, x=$06, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$2C, x=$06, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$20, x=$06, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$20, x=$06, y=$00, s=$00, p=$00)
$800D: INX                            (a=$20, x=$07, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$20, x=$07, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$20, x=$07, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$56, x=$07, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$56, x=$07, y=$00, s=$00, p=$00)
$800D: INX                            (a=$56, x=$08, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$56, x=$08, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$56, x=$08, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$47, x=$08, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$47, x=$08, y=$00, s=$00, p=$00)
$800D: INX                            (a=$47, x=$09, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$47, x=$09, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$47, x=$09, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$53, x=$09, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$53, x=$09, y=$00, s=$00, p=$00)
$800D: INX                            (a=$53, x=$0A, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$53, x=$0A, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$53, x=$0A, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$38, x=$0A, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$38, x=$0A, y=$00, s=$00, p=$00)
$800D: INX                            (a=$38, x=$0B, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$38, x=$0B, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$38, x=$0B, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$20, x=$0B, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$20, x=$0B, y=$00, s=$00, p=$00)
$800D: INX                            (a=$20, x=$0C, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$20, x=$0C, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$20, x=$0C, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$57, x=$0C, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$57, x=$0C, y=$00, s=$00, p=$00)
$800D: INX                            (a=$57, x=$0D, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$57, x=$0D, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$57, x=$0D, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$6F, x=$0D, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$6F, x=$0D, y=$00, s=$00, p=$00)
$800D: INX                            (a=$6F, x=$0E, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$6F, x=$0E, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$6F, x=$0E, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$72, x=$0E, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$72, x=$0E, y=$00, s=$00, p=$00)
$800D: INX                            (a=$72, x=$0F, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$72, x=$0F, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$72, x=$0F, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$6C, x=$0F, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$6C, x=$0F, y=$00, s=$00, p=$00)
$800D: INX                            (a=$6C, x=$10, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$6C, x=$10, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$6C, x=$10, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$64, x=$10, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$64, x=$10, y=$00, s=$00, p=$00)
$800D: INX                            (a=$64, x=$11, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$64, x=$11, y=$00, s=$00, p=$80)
$8010: BNE $F5                        (a=$64, x=$11, y=$00, s=$00, p=$80)
$8007: LDA $8018, X                   (a=$21, x=$11, y=$00, s=$00, p=$00)
$800A: STA $63C7, X                   (a=$21, x=$11, y=$00, s=$00, p=$00)
$800D: INX                            (a=$21, x=$12, y=$00, s=$00, p=$00)
$800E: CPX #$12                       (a=$21, x=$12, y=$00, s=$00, p=$03)
$8010: BNE $F5                        (a=$21, x=$12, y=$00, s=$00, p=$03)
$8012: LDA $5BFF                      (a=$00, x=$12, y=$00, s=$00, p=$03)
FRAME #1 END (3ms, 282Hz)
$
```

WIP status:
- [x] Hello, World! の範囲の動的逆アセンブルに対応
- [ ] 全命令の動的逆アセンブルに対応
- [ ] break point機能の追加（任意アドレスでブレイク後、ステップ実行で動作や任意位置のRAMダンプを確認やメモリの書き換えができる機能）

## How to make 6502 programs

- [6502マシン語ゲームプログラミング](https://github.com/suzukiplan/mgp-fc)の基礎編と実践編を読んでください
- リファレンス編に相当する内容は次章で記します（ただし, VGS8はファミコンのアーキテクチャを参考に開発したゲーム機なので, ファミコンのプログラミングに触れてから触るとより理解し易くなります）

## CPU memory map (WIP)

VGS8ではプログラマが意識する必要があるPPUのメモリマップについてもCPU上でミラーしているため, プログラマはCPUのメモリマップのみ把握すればプログラミング可能なシンプルな構造になっています。

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
|$5BFF|CPU I/O port (R): update VRAM request|
|$5C00〜$5FFF|Palette|
|$6000〜$6FFF|BG nametable (64x64)|
|$7000〜$7FFF|FG nametable (64x64)|
|$8000〜$BFFF|Program 0|
|$C000〜$FFFF|Program 1|

### Sprite OAM ($5000〜$53FF)

256個のスプライトの属性情報

```c
struct OAM {
    unsigned char x;        // X
    unsigned char y;        // Y
    unsigned char pattern;  // pattern number of CHR
    unsigned char reserved; // reserved
} oam[256];
```

- VGS8のスプライトはすべて8x8サイズ固定である
- x, y が 0 の場合, 描画処理自体が省略されるため, 消したい場合は x, y を 0 にすれば良い
- `reserved` は現時点では利用できない（将来的に利用するかもしれないので, 今の所常に 0 をセットすることを推奨）

### Switch program banks ($5400, $5401)

- $5400 にPRGバンク番号を store することで Program 0 のバンク切り替えが出来る
- $5401 にPRGバンク番号を store することで Program 1 のバンク切り替えが出来る

以下のユースケースを想定している。

- Program 0 実行中に Program 1 のバンク切り替え
- Program 1 実行中に Program 0 のバンク切り替え

> 現在のpcレジスタ上のバンク切り替えも一応できる（あまり使わないと思われるが）

```
LDA #2
STA $5400   ; Program 0 を PRGバンク番号2 に変更
LDA #3
STA $5401   ; Program 1 を PRGバンク番号3 に変更
```

### Switch character banks ($5402, $5403)

- $5402 にCHRバンク番号を store することで キャラクタ0 のバンク切り替えが出来る
- $5403 にCHRバンク番号を store することで キャラクタ1 のバンク切り替えが出来る

> ファミコンと同様, 高速なバンク切り替えアニメーションが可能です

```
LDA #4
STA $5402   ; キャラクタ0 を CHRバンク番号4 に変更
LDA #5
STA $5403   ; キャラクタ1 を CHRバンク番号5 に変更
```

### CMAP register ($5404)

bit配列: `-----FBS`

FG, BG, スプライトのそれぞれに指定するキャラクタ番号を各1bitで指定します。

```
LDA #%00000110
STA $5404   ; FG = キャラクタ1, BG = キャラクタ1, スプライト = キャラクタ0 に設定
```

### Background color ($5405)

- 全体の背景色に指定するパレット番号（0〜255）を指定します
- FG, BG, スプライト では パレット番号0 が透明色になりますが, 背景色には0を設定することができます

```
LDA #$FF
STA $5405   ; 背景色をパレット番号255に設定
```

### BG/FG window positions ($5406〜$5409)

FG, BG の表示起点座標をピクセル単位で指定する

- $5406: FGのX座標
- $5407: FGのY座標
- $5408: BGのX座標
- $5409: BGのY座標

```
LDA #100
STA $5406
STA $5407   ; FGの表示範囲の起点を (100, 100) にする
LDA #200
STA $5408
STA $5409   ; BGの表示範囲の起点を (200, 200) にする
```

> FG/BGの nametable は 8x8ピクセル のキャラクタ単位で 64x64 (512x512ピクセル) です。

### BG/FG scroll ($540A〜$540D)

FG, BG の nametable を 縦方向 or 横方向 にスクロールできます

- $540A: FG を上下方向にシフト (MSBがONなら上スクロール, MSBがOFF & 非0なら下スクロール)
- $540B: FG を左右方向にシフト (MSBがONなら左スクロール, MSBがOFF & 非0なら右スクロール)
- $540C: BG を上下方向にシフト (MSBがONなら上スクロール, MSBがOFF & 非0なら下スクロール)
- $540D: BG を左右方向にシフト (MSBがONなら左スクロール, MSBがOFF & 非0なら右スクロール)

```
LDA #$80
STA $540A   ; FGを上スクロール
LDA #$01
STA $540B   ; FGを下スクロール
LDA #$80
STA $540C   ; BGを上スクロール
LDA #$01
STA $540D   ; BGを下スクロール
```

> FG/BGの nametable は 8x8ピクセル のキャラクタ単位で 64x64 (512x512ピクセル) なので, 8ピクセル単位のスクロールを本機能で実現可能です。1ピクセル単位の細かいスクロールには window positions を用います。

### Play/Stop sound effect ($5500, $5501)

- $5500 に EFFバンク番号 を store することで, 指定バンク番号の効果音を再生します
- $5501 に EFFバンク番号 を store することで, 指定バンク番号の効果音を停止します

> 効果音は再生時間が終了すると自動的に停止します


### Play/Stop BGM ($5600〜$5603)

- $5600 に BGMバンク番号 を store することで, 指定バンク番号のBGMを再生します
- $5601 に 任意の値 を store することで, BGMの再生を停止（ポーズ）します
- $5602 に 任意の値 を stroe することで, BGMの再生を再開（レジューム）します
- $5603 を load することで, BGMが再生中がチェックできます ($00: 停止中, $01: 再生中)

### read JoyPad status ($5700, $5701)

- $5700 を load することで, プレイヤ1のジョイパッドの入力状態を取得できます
- $5701 を load することで, プレイヤ2のジョイパッドの入力状態を取得できます

取得した値のbit配列: `UDLRABES`

- `U` : 方向キーの上
- `D` : 方向キーの下
- `L` : 方向キーの左
- `R` : 方向キーの右
- `A` : Aボタン
- `B` : Bボタン
- `E` : SELECTボタン
- `S` : STARTボタン

### update VRAM request ($5BFF)

$5BFF を load することで __VRAM更新信号__ が VGS8 に送信される。
従って, ゲームのメインループ処理は以下のように実装することになる。

```
mainloop:
    ~~~ 処理 ~~~
    LDA $5BFF       ; VRAM更新信号 を送信
    JMP mainloop
```

> `VirtualMachine::tick` は __VRAM更新信号__ が発生するまでの間CPUを回し続けるため, VGS8エミュレータは __VRAM更新信号__ を送信しないプログラムを実行した場合にハングアップしてしまうことになるが、この場合 1フレーム分 (8MHz÷60) のクロック消費で強制的に __VRAM更新信号__ が発生する。


## License

[MIT](LICENSE.txt)
