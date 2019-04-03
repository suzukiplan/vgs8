.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    ; init CMAP register
    lda #%00000000
    sta $5404

    ; draw SUZUKI PLAN (16x16 font)
    ldx #$00
draw_loop1:
    lda tile_suzukiplan, x
    sta $6106, x ; (6, 6)
    clc
    adc #$10
    sta $6146, x
    inx
    cpx #20 ; text length
    bne draw_loop1

    ; draw VGS (32x32 font)
    ldx #$00
draw_loop2:
    lda tile_vgs, x
    sta $620A, x ; (6, 10)
    clc
    adc #$10
    sta $624A, x
    clc
    adc #$10
    sta $628A, x
    clc
    adc #$10
    sta $62CA, x
    inx
    cpx #12 ; text length
    bne draw_loop2

    ; draw copyright
    ldx #$00
draw_loop3:
    lda string_copyright, x
    sta $66C7, x ; (7, 27)
    inx
    cpx #18 ; text length
    bne draw_loop3

main_loop:
    ; draw start
    ldx #$00
draw_loop4:
    lda string_start, x
    sta $6486, x ; (6, 18)
    inx
    cpx #20 ; text length
    bne draw_loop4

    ldx #$20
wait_loop1:
    lda $5BFF ; Wait for VSYNC
    dex
    bne wait_loop1

    ; remove start
    ldx #$00
    lda #$00
draw_loop5:
    sta $6486, x ; (6, 18)
    inx
    cpx #20 ; text length
    bne draw_loop5

    ldx #$10
wait_loop2:
    lda $5BFF ; Wait for VSYNC
    dex
    bne wait_loop2
    jmp main_loop


tile_suzukiplan: .byte $60,$61,$62,$63,$64,$65,$62,$63,$66,$67,$68,$00,$69,$6A,$6B,$6C,$6D,$6E,$80,$81
tile_vgs: .byte $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB
string_copyright: .byte "@2019, SUZUKI PLAN"
string_start: .byte "^+O OR DROP ROM HERE"