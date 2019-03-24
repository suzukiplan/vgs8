.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    ; init CMAP register
    lda #%00000000
    sta $5404

    ; draw "Hello, VGS8 World!" on the BG nametable
    ldx #$00
draw_loop:
    lda string_hello_world, x
    sta $63C7, x ; write on (7 + x, 15) = ($007 + x, $3C0 = 15 * 64)
    inx
    cpx #18 ; text length
    beq draw_loop

mainloop:
    lda $5BFF ; Wait for VSYNC
    jmp mainloop

string_hello_world:
    .byte "Hello, VGS8 World!"
