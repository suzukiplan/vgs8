# SUZUKI PLAN - Video Game System - 8bit (WIP)

※まだ開発途中です

- SUZUKI PLAN - Video Game System - 8bit (VGS8) は, 現代向けに再設計された新しい8bitゲーム機です
- このリポジトリでは, VGS8コアシステムのエミュレータ（HALを省略した形）とVGS用ゲーム開発に必要なSDKを提供します
- VGS8の基礎アーキテクチャは, 任天堂のファミリーコンピュータ（ファミコン）から着想を得て設計していますが, ファミコンとの互換性はありません

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

- 上記を実行することで, ROMをリンクするツール（romlink）の生成とVGS8コアのテストが実行されます
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

## CPU memory map (WIP)

|Address|Usage|
|---|---|
|$0000〜$00FF|Zero page|
|$0100〜$01FF|Stack|
|$0200〜$4FFF|RAM|
|$5000〜$53FF|Sprite OAM (4x256)|
|$5400|I/O port (RW): PRG Bank of $8000〜$BFFF|
|$5401|I/O port (RW): PRG Bank of $C000〜$FFFF|
|$5402|I/O port (RW): CHR Bank of 0|
|$5403|I/O port (RW): CHR Bank of 1|
|$5405|I/O port (R): update VRAM request|
|$5402〜$5BFF|other I/O ports (WIP)|
|$5C00〜$5FFF|Palette|
|$6000〜$6FFF|BG nametable (64x64)|
|$7000〜$7FFF|FG nametable (64x64)|
|$8000〜$BFFF|Program 0|
|$C000〜$FFFF|Program 1|

ファミコンの仕様を部分的に流用しつつ, よりプログラミングがしやすいレイアウトになっている筈。

- ファミコンの場合, 任意256バイトをPPUのOAMにDMA転送することでスプライト描画していたが $5000〜$53FF でダイレクトにOAMマップされる
- $5400〜$5FFFで PPUへのI/O, APUへのI/O, ジョイパッド入力, バンク切り替え, DMA等を実現する予定
- BG/FGのnametableはファミコンのnametableと同様8x8単位でキャラクタパターンを指定する（座標系は32x32）

### $5405: update VRAM request

$5405 を load することで __VRAM更新信号__ が VGS8 に送信される。
従って, ゲームのメインループ処理は以下のように実装することになる。

```
mainloop:
    ~~~ 処理 ~~~

    LDA $5405       ; VRAM更新信号 を送信
    JMP mainloop
```

> `VirtualMachine::tick` は __VRAM更新信号__ が発生するまでの間CPUを回し続けるため, VGS8エミュレータは __VRAM更新信号__ を送信しないプログラムを実行した場合にハングアップしてしまうことになるが、この場合 1秒分(8MHz)のクロック消費 または BRK命令 で処理が中断する。


## License

[GPLv3](LICENSE.txt)

