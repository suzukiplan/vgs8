.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    LDA #$01
    STA $01
    LDA #$02
    STA $02
    AND #$00
    LDA $00
    LDX #$01
    LDA $00, X
    LDA $01, X
