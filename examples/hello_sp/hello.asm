.setcpu     "6502"
.autoimport on

;-------------------------------------------------------------------------------
; Program entry point
;-------------------------------------------------------------------------------
.segment "STARTUP"
    jsr initialize
mainloop:
    jsr input_joy_pad
    jsr draw_player
    jsr scroll_bg
    lda $5BFF ; Wait for VSYNC
    jmp mainloop

;-------------------------------------------------------------------------------
; Initialize sub routine
;-------------------------------------------------------------------------------
initialize:
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
initialize_draw_loop1:
    lda string_hello_world, x
    sta $73C7, x ; write on (7 + x, 15) = ($007 + x, $3C0 = 15 * 64)
    inx
    cpx #18 ; text length
    bne initialize_draw_loop1

    ; fill $01 on the BG nametable (TIPS: it's very easy if use DMA!)
    ; 1st. fill $01 to $6000 ~ $60FF
    lda #$60
    sta $5A00
    lda #$01
    sta $5A01
    ; 2nd. copy $60xx to $61xx ~ $6Fxx
    ldx #$61
initialize_draw_loop2:
    stx $5A02
    inx
    cpx #$70
    bne initialize_draw_loop2
    ; done!

    ; initialize the player variables
    lda #120
    sta v_playerX
    lda #200
    sta v_playerY
    rts

;-------------------------------------------------------------------------------
; Input Joy-Pad sub routine
;-------------------------------------------------------------------------------
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

;-------------------------------------------------------------------------------
; Draw player sub routine
;-------------------------------------------------------------------------------
draw_player:
    ; set x to OAM
    lda v_playerX
    sta sp_player + 0
    lda v_playerY
    sta sp_player + 1
    lda #$10
    sta sp_player + 2
    lda #%00000001 ; use 16x16 sprite
    sta sp_player + 3
    rts

;-------------------------------------------------------------------------------
; Scroll BG sub routine
;-------------------------------------------------------------------------------
scroll_bg:
    ldx v_scroll
    stx $5409
    inx
    stx v_scroll
    rts

;-------------------------------------------------------------------------------
; String literal definition
;-------------------------------------------------------------------------------
string_hello_world:
    .byte "Hello, VGS8 World!"

;-------------------------------------------------------------------------------
; WRAM (variable labels)
;-------------------------------------------------------------------------------
.org $0200
v_scroll:   .byte $00
v_playerX:  .byte $00
v_playerY:  .byte $00

;-------------------------------------------------------------------------------
; Sprite OAM labels
;-------------------------------------------------------------------------------
.org $5000 
sp_player:  .byte $00, $00, $00, $00
