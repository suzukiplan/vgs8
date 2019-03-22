# SUZUKI PLAN - Video Game System - 8bit (WIP)

※まだ開発途中です

SUZUKI PLAN - Video Game System - 8bit (VGS8) は, 現代向けに再設計された新しい8bitゲーム機です。
このリポジトリでは, VGS8コアシステムのエミュレータをプラットフォーム非依存な形式（HALを省略した形）で提供します。
VGS8の基礎アーキテクチャは, 任天堂のファミリーコンピュータ（ファミコン）から着想を得て設計していますが, ファミコンとの互換性はありません。

## Features

- CPU: MOS6502 8MHz
  - プログラムは, 最大16KB x 256バンク
  - ROMのPRGバンクエリアから, RAMの $8000 or $C000 にロードする
  - ファミコンのNxROM(iNES mapper2)と同様, 動的なバンク切り替えが可能
- PPU
  - キャラクタデータ: 8bitカラー (256色) 128x128ピクセル x 2枚 (256個のCHR bankから割り当て)
  - キャラクタサイズ: 8x8 or 16x16ピクセル
  - スプライト表示数: 256個 (水平上限無し)
  - BG4画面 + FG4画面 (BGはスプライトの背面, FGはスプライトの前面に表示される & ミラーは無し)
  - 画面サイズ: 240x240ピクセル (座標系は256x256で, 上下左右の端8pxがmask)
  - パレットは256色パレットひとつのみ（ここはファミコンではなくVGS仕様に準拠）
- APU (ココはVGS2と同じ仕様)
  - 256個のBGM, 256個のSE
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
|$5400|I/O port: Program Bank of $8000〜$BFFF|
|$5401|I/O port: Program Bank of $8000〜$BFFF|
|$5402〜$5FFF|other I/O ports (WIP)|
|$6000〜$63FF|BG nametable 0 (LT; 左上)|
|$6400〜$67FF|BG nametable 1 (RT; 右上)|
|$6800〜$6BFF|BG nametable 2 (LB; 左下)|
|$6C00〜$6FFF|BG nametable 3 (RB; 右下)|
|$7000〜$73FF|FG nametable 0 (LT; 左上)|
|$7400〜$77FF|FG nametable 1 (RT; 右上)|
|$7800〜$7BFF|FG nametable 2 (LB; 左下)|
|$7C00〜$7FFF|FG nametable 3 (RB; 右下)|
|$8000〜$BFFF|Program1|
|$C000〜$FFFF|Program2|

ファミコンの仕様を部分的に流用しつつ, よりプログラミングがしやすいレイアウトになっている筈。

- ファミコンの場合, 任意256バイトをPPUのOAMにDMA転送することでスプライト描画していたが $5000〜$53FF でダイレクトにOAMマップされる
- $5400〜$5FFFで PPUへのI/O, APUへのI/O, ジョイパッド入力, バンク切り替え, DMA等を実現する予定
- BG/FGのnametableはファミコンのnametableと同様8x8単位でキャラクタパターンを指定する（座標系は32x32）
