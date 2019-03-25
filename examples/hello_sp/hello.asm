.setcpu     "6502"
.autoimport on

.segment "STARTUP"
    ; init CMAP register
    ;     -----FBS
    lda #%00000011
    sta $5404

    ; set CHR banks
    lda #0
    sta $5402   ; set CHR0 bank (0: bank000.bmp)
    lda #1
    sta $5403   ; set CHR1 bank (1: bank001.bmp)

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

    ; initialize the player variables
    lda #120
    sta v_playerX
    lda #200
    sta v_playerY

mainloop:
    jsr input_joy_pad
    jsr draw_player
    jsr scroll_bg
    lda $5BFF ; Wait for VSYNC
    jmp mainloop

input_joy_pad:
    ldx $5700
    txa
    and #%10000000
    beq input_joy_pad_1
    ; move up
    ldy v_playerY
    dey
    sty v_playerY
input_joy_pad_1:
    txa
    and #%01000000
    beq input_joy_pad_2
    ; move down
    ldy v_playerY
    iny
    sty v_playerY
input_joy_pad_2:
    txa
    and #%00100000
    beq input_joy_pad_3
    ; move left
    ldy v_playerX
    dey
    sty v_playerX
input_joy_pad_3:
    txa
    and #%00010000
    beq input_joy_pad_4
    ; move right
    ldy v_playerX
    iny
    sty v_playerX
input_joy_pad_4:
    rts

draw_player:
    ; set x to OAM
    lda v_playerX
    sta sp_player0 + 0
    sta sp_player2 + 0
    clc
    adc #8
    sta sp_player1 + 0
    sta sp_player3 + 0
    ; set y to OAM
    lda v_playerY
    sta sp_player0 + 1
    sta sp_player1 + 1
    clc
    adc #8
    sta sp_player2 + 1
    sta sp_player3 + 1
    ; set pattern to OAM
    lda #$10
    sta sp_player0 + 2
    lda #$11
    sta sp_player1 + 2
    lda #$20
    sta sp_player2 + 2
    lda #$21
    sta sp_player3 + 2
    rts

scroll_bg:
    ldx v_scroll
    stx $5409
    inx
    stx v_scroll
    rts

string_hello_world:
    .byte "Hello, VGS8 World!"

.org $0200  ; variables
v_scroll:   .byte $00
v_playerX:  .byte $00
v_playerY:  .byte $00

.org $5000  ; Sprite OAM labels
sp_player0: .byte $00, $00, $00, $00    ; left top of the player
sp_player1: .byte $00, $00, $00, $00    ; right top of the player
sp_player2: .byte $00, $00, $00, $00    ; left bottom of the player
sp_player3: .byte $00, $00, $00, $00    ; right bottom of the player
