.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    ; テストしやすいようにゼロページの値をアドレスと同値にしておく
    LDX #$00
LOOP1:
    TXA
    STA $00, X
    INX
    BNE LOOP1

    ; 特に意味は無いがゼロページを全部ロード
    LDY #$00
LOOP2:
    LDA $00, Y
    INY
    BNE LOOP2

    ; ついでにDEX DEYをテストしておく
    DEX
    DEY

    ; LDX/LDYのゼロページ参照をテスト
    LDX $05
    LDY $05
    LDX $05, Y
    LDY $05, X

    LDA $00
    LDA $01
    LDA $02
    LDA $03

    ; indirectでのロード・ストア（私はFCゲーム開発で使ったことが無いけど）
    LDA ($01, X)
    LDA ($01), Y

    ; absoluteなload/storeパターンを網羅しておく
    LDA $1000
    LDA $1000, X
    LDA $1000, Y
    LDX $2000
    LDX $2000, Y
    LDY $3000
    LDY $3000, X
    BRK