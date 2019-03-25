.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    ; init CMAP register
    lda #%00000010
    sta $5404

    ; set CHR banks
    lda #0
    sta $5402   ; set CHR0 bank (0: font.bmp)
    lda #1
    sta $5403   ; set CHR1 bank (1: bg.bmp)

    ; draw "Hello, VGS8 World!" on the FG nametable
    ldx #$00
draw_loop:
    lda string_hello_world, x
    sta $73C7, x ; write on (7 + x, 15) = ($007 + x, $3C0 = 15 * 64)
    inx
    cpx #18 ; text length
    bne draw_loop

    ; fill $01 on the BG nametable (TIPS: it's very easy if use DMA!)
    ; 1st. fill $01 to $6000 ~ $60FF
    lda #$60
    sta $5A00
    lda #$01
    sta $5A01
    ; 2nd. copy $60xx to $61xx ~ $6Fxx
    ldx #$61
draw_loop2:
    stx $5A02
    inx
    cpx #$70
    bne draw_loop2
    ; done!

    ldx #$00
mainloop:
    lda $5BFF ; Wait for VSYNC
    ; scroll BG
    stx $5409
    inx
    jmp mainloop

string_hello_world:
    .byte "Hello, VGS8 World!"
