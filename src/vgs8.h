// Copyright 2019, SUZUKI PLAN (MIT license)
#ifndef INCLUDE_VGS8_H
#define INCLUDE_VGS8_H
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "lz4.h"

namespace VGS8
{
class Bank;
class CPU;
class APU;
class PPU;

class VirtualMachine
{
  private:
    unsigned char p1key; // 1コンのキー入力状態
    unsigned char p2key; // 2コンのキー入力状態
    unsigned short displayBuffer[240 * 240];
    char saveBuffer[65536]; // max = register + 32KB
    int savePtr;

  public:
    Bank* bank; // Bank
    CPU* cpu;   // Central Processing Unit
    APU* apu;   // Audio Processing Unit
    PPU* ppu;   // Picture Processing Unit

    /**
     * コンストラクタ（VMを新規生成）
     * @param rom [I] このVMで用いるROMを指定
     * @param size [I] romのサイズ
     * @note 1VM/1ROMの制約を課す（※ROMを入れ替える場合VMを再生成すること）
     * @note romの領域はVM生成後に破棄しても良い
     */
    VirtualMachine(const void* rom, size_t size);

    /**
     * デストラクタ（VMを破棄）
     */
    ~VirtualMachine();

    /**
     * ジョイパッド（コントローラ）の入力状態を設定
     * @param number [I] プレイヤ番号（1コン=0, 2コン=1）
     * @param up [I] 上ボタン
     * @param down [I] 下ボタン
     * @param left [I] 左ボタン
     * @param right [I] 右ボタン
     * @param a [I] Aボタン
     * @param b [I] Bボタン
     * @param select [I] セレクトボタン
     * @param start [I] スタートボタン
     */
    void setJoyPad(int number,
                   bool up,
                   bool down,
                   bool left,
                   bool right,
                   bool a,
                   bool b,
                   bool select,
                   bool start);

    /**
     * リセット
     */
    void reset();

    /**
     * CPUを1フレーム回す
     */
    void tick();

    /**
     * ディスプレイ表示内容（RGB565形式）を取得（主にAndroid, Windows用）
     * @param size [O] 出力データサイズ
     * @return 240x240ピクセル分（非可視領域クロップ済み）の画像バッファ
     */
    unsigned short* getDisplay565(size_t* size);

    /**
     * ディスプレイ表示内容（RGB555形式）を取得（主にiOS, macOS用）
     * @param size [O] 出力データサイズ
     * @return 240x240ピクセル分（非可視領域クロップ済み）の画像バッファ
     */
    unsigned short* getDisplay555(size_t* size);

    /**
     * スピーカ出力内容（PCM）を取得
     * @param samples [I] 出力サンプリング周波数（推奨: 22050）
     * @param bits [I] 出力ビットレート（推奨: 16bit）
     * @param ch [I] チャンネル数（推奨: 1/mono）
     * @param buffer [I/O] 出力バッファ
     * @param size [I] バッファサイズ
     */
    void getPCM(int samples, int bits, int ch, void* buffer, size_t size);

    /**
     * クイックセーブ
     * @param size セーブデータのサイズ
     * @return セーブデータ
     */
    void* save(size_t* size);

    /**
     * クイックロード
     * @param state ロードするセーブデータ
     * @param size ロードするセーブデータのサイズ
     * @return true = 成功, false = 失敗
     */
    bool load(void* state, size_t size);

    /* 以下、内部関数 */
    void setChrBank(int cn, unsigned char bn);
    void setChrMap(unsigned char n);
};

#include "bank.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "apu.hpp"

};     // namespace VGS8
#endif // INCLUDE_VGS8_H