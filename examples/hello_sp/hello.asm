; Hello, World! for VGS8 by SUZUKI PLAN (PUBLIC DOMAIN)
.setcpu     "6502"
.autoimport on

;-------------------------------------------------------------------------------
; Program entry point & main loop
;-------------------------------------------------------------------------------
.segment "STARTUP"
    jsr initialize
mainloop:
    jsr move_player
    jsr move_player_shots
    jsr scroll_bg
    lda $5BFF ; Wait for VSYNC
    jmp mainloop

;-------------------------------------------------------------------------------
; Initialize
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
    lda #120 ; initial position X
    sta sp_player + 0
    lda #200 ; initial position Y
    sta sp_player + 1
    lda #$10 ; tile pattern (fix)
    sta sp_player + 2
    lda #%00000001 ; use 16x16 sprite (fix)
    sta sp_player + 3
    rts

;-------------------------------------------------------------------------------
; Move player
;-------------------------------------------------------------------------------
move_player:
    ldx $5700 ; get JoyPad status
    txa
    and #%10000000
    beq move_player_1
    ; move up
    ldy sp_player + 1
    dey
    sty sp_player + 1
move_player_1:
    txa
    and #%01000000
    beq move_player_2
    ; move down
    ldy sp_player + 1
    iny
    sty sp_player + 1
move_player_2:
    txa
    and #%00100000
    beq move_player_3
    ; move left
    ldy sp_player + 0
    dey
    sty sp_player + 0
move_player_3:
    txa
    and #%00010000
    beq move_player_4
    ; move right
    ldy sp_player + 0
    iny
    sty sp_player + 0
move_player_4:
    txa
    and #%00001100
    beq move_player_5
    lda v_shotW
    bne move_player_6
    jsr fire_player_shot
    rts
move_player_5:
    lda v_shotW
    beq move_player_6
    dec v_shotW ; decrement wait fire flag
    rts
move_player_6:
    rts

;-------------------------------------------------------------------------------
; Fire player shot
;-------------------------------------------------------------------------------
fire_player_shot:
    ; set 1 to v_shotF[v_shotI]
    ldx v_shotI
    lda #1
    sta v_shotF, x
    ; increment v_shotI (and loop 0 ~ 7)
    inc v_shotI
    lda v_shotI
    and #$07
    sta v_shotI
    ; calcurate index of sprite (x4) to x
    txa
    asl
    asl
    tax
    ; set initial X
    lda sp_player + 0
    clc
    adc #4
    sta sp_shot, x
    ; set initial Y
    lda sp_player + 1
    sec
    sbc #8
    sta sp_shot + 1, x
    ; set pattern
    lda #$02
    sta sp_shot + 2, x
    ; set flags
    lda #$00
    sta sp_shot + 3, x
    ; set wait fire flag
    lda #4
    sta v_shotW
    rts

;-------------------------------------------------------------------------------
; Move player shots
;-------------------------------------------------------------------------------
move_player_shots:
    ldx #0
move_player_shots_1:
    lda v_shotF, x
    beq move_player_shots_2 ; branch if flag is unset
    txa
    pha
    asl
    asl
    tax
    jsr move_player_shot
    pla
    tax
move_player_shots_2:
    inx
    cpx #8
    bne move_player_shots_1
    rts

; move a shot (x is index of the shot)
move_player_shot:
    ; move up 4px
    lda sp_shot + 1, x
    sec
    sbc #4
    sta sp_shot + 1, x
    bcs move_player_shot_1
    rts
move_player_shot_1:
    ; hide sprite
    lda #0
    sta sp_shot + 2, x
    ; clear flag
    txa
    lsr
    lsr
    tax
    lda #0
    sta v_shotF, x
    rts

;-------------------------------------------------------------------------------
; Scroll BG
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
v_shotW:    .byte $00 ; wait fire flag
v_shotI:    .byte $00 ; index of the player shot
v_shotF:    .byte $00, $00, $00, $00, $00, $00, $00, $00 ; flags of the player shot

;-------------------------------------------------------------------------------
; Sprite OAM labels (OAM: x, y, pattern, falgs)
;-------------------------------------------------------------------------------
.org $5000 
sp_player:  .byte $00, $00, $00, $00    ; player (16x16)
sp_shot:    .byte $00, $00, $00, $00    ; shot[0]
            .byte $00, $00, $00, $00    ; shot[1]
            .byte $00, $00, $00, $00    ; shot[2]
            .byte $00, $00, $00, $00    ; shot[3]
            .byte $00, $00, $00, $00    ; shot[4]
            .byte $00, $00, $00, $00    ; shot[5]
            .byte $00, $00, $00, $00    ; shot[6]
            .byte $00, $00, $00, $00    ; shot[7]
