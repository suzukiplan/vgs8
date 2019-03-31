// Copyright 2019, SUZUKI PLAN (MIT license)
// VGS8 emulator virtual machine
#ifndef INCLUDE_VGS8_H
#define INCLUDE_VGS8_H
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "lz4.h"
#include "vgsdec.h"

namespace VGS8
{
class Bank;
class CPU;
class APU;
class PPU;

class VirtualMachine
{
  private:
    unsigned short displayBuffer[240 * 240];
    char saveBuffer[65536]; // max = register + 32KB
    int savePtr;

  public:
    unsigned char keys[2];  // キー入力状態
    unsigned char touching; // タッチ状態 ($00: not touching, $01: touching)
    unsigned char touchX;   // 最後にタッチしていたX座標
    unsigned char touchY;   // 最後にタッチしていたY座標
    Bank* bank;             // Bank
    CPU* cpu;               // Central Processing Unit
    APU* apu;               // Audio Processing Unit
    PPU* ppu;               // Picture Processing Unit

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
    void setKey(int number,
                bool up,
                bool down,
                bool left,
                bool right,
                bool a,
                bool b,
                bool select,
                bool start);

    /**
     * タッチパネルを入力状態を設定
     * @param isTouching [I] on / off
     * @param x X座標 (8〜247)
     * @param y Y座標 (8〜247)
     */
    void setTouch(bool isTouching, unsigned char x, unsigned char y);

    /**
     * リセット
     */
    void reset();

    /**
     * CPUを1フレーム回す
     * @return 実行クロック数
     */
    unsigned int tick();

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
     * @param size [O] 出力データサイズ
     * @return PCMデータバッファ (波形は22050Hz/16bit/1ch固定)
     */
    short* getPCM(size_t* size);

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

    /**
     * 特定のバンクの特定のアドレスの命令を実行する直前にコールバックを呼び出す
     * @param bank バンク番号
     * @param addr アドレス（absolute指定）
     * @param callback コールバック
     * @note ブレイクポイントはひとつだけ設定可能（最後に設定したもののみ有効）
     */
    void setBreakPoint(unsigned short bank, unsigned short addr, void (*callback)(VGS8::VirtualMachine*));

    /**
     * ブレイクポイントを解除
     */
    void resetBreakPoint();

    /**
     * RAMの先頭ポインタを取得
     */
    const unsigned char* getRAM();

    /* 以下、内部関数 (CPUからPPUへアクセスするためのもの) */
    void _setChrBank(int cn, unsigned char bn);
    void _setChrMap(unsigned char n);
    void _setBgColor(unsigned char n);
    void _setBgX(unsigned char n);
    void _setBgY(unsigned char n);
    void _setFgX(unsigned char n);
    void _setFgY(unsigned char n);
    void _playEff(unsigned char n);
    void _pauseEff();
    void _resumeEff();
    void _updateEffMasterVolume();
    void _stopEff(unsigned char n);
    void _playBgm(unsigned char n);
    void _pauseBgm();
    void _resumeBgm();
    bool _isBgmPlaying();
    void _updateBgmMasterVolume();
};

#include "bank.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "apu.hpp"

};     // namespace VGS8
#endif // INCLUDE_VGS8_H
