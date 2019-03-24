# SUZUKI PLAN - Video Game System - 8bit (WIP)

※まだ開発途中です

- SUZUKI PLAN - Video Game System - 8bit (VGS8) は, 現代向けに再設計された新しい8bitゲーム機です
- このリポジトリでは, VGS8コアシステムのエミュレータ（HALを省略した形）とVGS用ゲーム開発に必要なSDKを提供します
- VGS8の基礎アーキテクチャは, 任天堂のファミリーコンピュータ（ファミコン）から着想を得て設計していますが, ファミコンとの互換性はありません

## WIP status

- [x] [BANK(ROMのメモリマッパ)を実装](src/bank.hpp)
- [x] [CPUを実装](src/cpu.hpp)
- [x] [PPUを実装](src/ppu.hpp)
- [x] [APUを実装](src/apu.hpp)
- [x] [VGS8仮想マシンを実装](src/vgs8.h)
- [ ] I/Oポートマップを完成させる
- [ ] macOSでテストできるCocoaAppを作成
- [ ] examplesを色々と追加しながらテスト
- [ ] [ファミコン版Battle Marine](https://github.com/suzukiplan/battle-marine-fc)をVGS8に移植して最終テスト
- [ ] 開発者向けのマニュアルを纏める（基本[コレ](https://github.com/suzukiplan/mgp-fc)みたいな感じになる予定）

## Setup

### Pre requests

- GNU make
- CLANG
- cc65

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
- バッテリーバックアップ機能は無い（セーブ/ロードはH/W機能としてのみ提供する想定）

## How to make your GAME

1. cc65を使って最大256個のプログラム `(*.bin)` をアセンブル（1つのプログラムの最大サイズ: 16KB）
2. 任意のグラフィックエディタで 128x128 サイズの 256色bitmapファイル `(*.bmp)` を最大256個作成
3. 任意の波形エディタで 22050Hz, 16bit, 1ch の効果音ファイル `(*.wav)` を最大256個作成
4. 任意のテキストエディタで [VGSのMML形式ファイル](https://github.com/suzukiplan/vgs-mml-compiler/blob/master/MML-ja.md) `(*.mml)` を最大256個作成
5. [romlink](tools/romlink.c)で, `*.bin` , `*.bmp` , `*.wav` , `*.mml` をリンク

> 以下のファイルを用いたゲームの[romlink](tools/romlink.c) をする例を示します。
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
> ```
> romlink mygame.rom \
>     program1.bin program2.bin program3.bin \
>     sprite.bmp bg1.bmp bg2.bmp bg3.bmp \
>     eff1.wav eff2.wav eff3.wav \
>     bgm1.mml bgm2.mml bgm3.mml
> ```

## CPU memory map (WIP)

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
|$5406|PPU I/O port (RW): X of FG window position|
|$5407|PPU I/O port (RW): Y of FG window position|
|$5408|PPU I/O port (RW): X of BG window position|
|$5409|PPU I/O port (RW): Y of BG window position|
|$540A|PPU I/O port (W): FG nametable virtical scroll|
|$540B|PPU I/O port (W): FG nametable horizontal scroll|
|$540C|PPU I/O port (W): BG nametable virtical scroll|
|$540D|PPU I/O port (W): BG nametable horizontal scroll|
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

### Switch character banks ($5402, $5403)

- $5402 にCHRバンク番号を store することで キャラクタ0 のバンク切り替えが出来る
- $5403 にCHRバンク番号を store することで キャラクタ1 のバンク切り替えが出来る

> ファミコンと同様, 高速なバンク切り替えアニメーションが可能です

### CMAP register ($5404)

bit配列: `-----FBS`

FG, BG, スプライトのそれぞれに指定するキャラクタ番号を各1bitで指定します。

```
LDA #%00000110
STA $5404
```

上記コードを実行すれば, FGとBGがキャラクタ1, スプライトがキャラクタ0 になります。

### Background color ($5405)

- 全体の背景色に指定するパレット番号（0〜255）を指定します
- FG, BG, スプライト では パレット番号0 が透明色になりますが, 背景色には0を設定することができます
- デフォルトの背景色: 0

### Window positions ($5406〜$5409)

### Scroll nametables ($540A〜$540D)

FG, BG の nametable を 縦方向 or 横方向 にスクロールできます

- $540A: FG を上下方向にシフト (MSBがONなら上スクロール, MSBがOFF & 非0なら下スクロール)
- $540B: FG を左右方向にシフト (MSBがONなら左スクロール, MSBがOFF & 非0なら右スクロール)
- $540C: BG を上下方向にシフト (MSBがONなら上スクロール, MSBがOFF & 非0なら下スクロール)
- $540D: BG を左右方向にシフト (MSBがONなら左スクロール, MSBがOFF & 非0なら右スクロール)

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

[GPLv3](LICENSE.txt)

