.setcpu     "6502"
.autoimport on

;-------------------------------------------------------------------------------
; Program entry point
;-------------------------------------------------------------------------------
.segment "STARTUP"
    jsr initialize
mainloop:
    jsr move_player
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
    lda #120 ; initial position X
    sta sp_playerX
    lda #200 ; initial position Y
    sta sp_playerY
    lda #$10 ; tile pattern (fix)
    sta sp_playerP
    lda #%00000001 ; use 16x16 sprite (fix)
    sta sp_playerF
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
    ldy sp_playerY
    dey
    sty sp_playerY
move_player_1:
    txa
    and #%01000000
    beq move_player_2
    ; move down
    ldy sp_playerY
    iny
    sty sp_playerY
move_player_2:
    txa
    and #%00100000
    beq move_player_3
    ; move left
    ldy sp_playerX
    dey
    sty sp_playerX
move_player_3:
    txa
    and #%00010000
    beq move_player_4
    ; move right
    ldy sp_playerX
    iny
    sty sp_playerX
move_player_4:
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

;-------------------------------------------------------------------------------
; Sprite OAM labels (OAM: x, y, pattern, falgs)
;-------------------------------------------------------------------------------
.org $5000 
sp_playerX: .byte $00
sp_playerY: .byte $00
sp_playerP: .byte $00
sp_playerF: .byte $00
