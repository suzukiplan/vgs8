; Hello, World! for VGS8 by SUZUKI PLAN (PUBLIC DOMAIN)
.setcpu     "6502"
.autoimport on

;-------------------------------------------------------------------------------
; Program entry point & main loop
;-------------------------------------------------------------------------------
.segment "STARTUP"
    jsr initialize
    lda #$00
    sta $5600 ; play BGM 0
mainloop:
    lda $5BFF ; Wait for VSYNC
    jsr add_new_enemy
    jsr move_enemies
    jsr move_player
    jsr move_player_shots
    jsr scroll_bg
    jsr show_mouse_status
    jsr drag_mouse_to_hscroll
    jsr draw_mouse_cursor
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
    ; increment v_shotI
    txa
    clc
    adc #4
    and #31
    sta v_shotI
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
    ; play eff $01
    lda #$01
    sta $5500
    rts

;-------------------------------------------------------------------------------
; Move player shots
;-------------------------------------------------------------------------------
move_player_shots:
    ldx #0
move_player_shots_1:
    lda v_shotF, x
    beq move_player_shots_2 ; branch if flag is unset
    jsr move_player_shot
move_player_shots_2:
    inx
    inx
    inx
    inx
    cpx #32
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
    lda #0
    sta sp_shot + 2, x
    lda #0
    sta v_shotF, x
    rts

;-------------------------------------------------------------------------------
; Add new enemy
;-------------------------------------------------------------------------------
add_new_enemy:
    ; check timer (enemy emergence will per 16 frames)
    lda v_enemyT
    clc
    adc #1
    sta v_enemyT
    and #$0F
    beq add_new_enemy_1
    rts
add_new_enemy_1:
    ; check still enemy is exist at the current index
    ldx v_enemyI
    lda v_enemy + 0, x
    beq add_new_enemy_2
    rts
add_new_enemy_2:
    ; create an enemy
    inc v_enemy + 0, x
    lda #$01 ; type 1
    sta v_enemy + 1, x
    lda #$00 ; clear v1, v2
    sta v_enemy + 2, x
    sta v_enemy + 3, x
    ; set x (random: 0 ~ 224)
    jsr get_rand_to_a
    cmp #224
    bcc add_new_enemy_3
    sec
    sbc #32
add_new_enemy_3:
    clc
    adc #8
    sta sp_enemy + 0, x
    lda #0
    sta sp_enemy + 1, x
    lda #$12
    sta sp_enemy + 2, x
    lda #%00000001
    sta sp_enemy + 3, x
    ; add 4 the enemy index
    txa
    clc
    adc #4
    and #63
    sta v_enemyI
    rts

;-------------------------------------------------------------------------------
; Move enemies
;-------------------------------------------------------------------------------
move_enemies:
    ldx #0
move_enemies_1:
    ; check flag
    lda v_enemy + 0, x
    bne move_enemies_2
    jmp move_enemies_next
move_enemies_2:
    ; check type
    lda v_enemy + 1, x
    cmp #1
    bne move_enemies_3
    jsr move_enemy_type1
    jmp move_enemies_next
move_enemies_3:
    cmp #2
    bne move_enemies_4
    jsr move_enemy_type2
    jmp move_enemies_next
move_enemies_4:
    cmp #3
    bne move_enemies_5
    jsr move_enemy_type2
    jmp move_enemies_next
move_enemies_5:
    jsr move_enemy_destruct
move_enemies_next:
    jsr enemy_hit_check
    ; add index
    txa
    clc
    adc #4
    and #63
    beq move_enemies_end
    tax
    jmp move_enemies_1 ; check the next enemy
move_enemies_end:
    rts

; Move enemy (type1)
move_enemy_type1:
    lda sp_enemy + 1, x
    clc
    adc #2
    sta sp_enemy + 1, x
    cmp #248
    bcc move_enemy_type1_1
    jsr remove_enemy
move_enemy_type1_1:
    lda v_enemy + 2, x
    clc
    adc #1
    sta v_enemy + 2, x
    and #$07
    lsr
    lsr
    asl
    clc
    adc #$12
    sta sp_enemy + 2, x
    rts

move_enemy_type2:
    rts

move_enemy_type3:
    rts

move_enemy_destruct:
    lda v_enemy + 2, x
    clc
    adc #$01
    and #$0F
    bne move_enemy_destruct_1
    jsr remove_enemy
    rts
move_enemy_destruct_1:
    sta v_enemy + 2, x
    lsr
    lsr
    asl
    clc
    adc #$30
    sta sp_enemy + 2, x
    dec sp_enemy + 1, x
    rts

remove_enemy:
    lda #0
    sta v_enemy + 0, x ; clear flag
    sta sp_enemy + 2, x ; clear sprite
    rts

destruct_enemy:
    lda #$FF
    sta v_enemy + 0, x ; change flag (no hit check)
    sta v_enemy + 1, x ; change type (bomb)
    lda #$00
    sta v_enemy + 2, x ; clear v1
    lda #$30
    sta sp_enemy + 2, x ; change sprite
    ; play eff $00
    lda #$00
    sta $5500
    rts

enemy_hit_check:
    lda v_enemy + 0, x
    bne enemy_hit_check_1
    rts
enemy_hit_check_1:
    cmp #$FF
    bne enemy_hit_check_1_1
    rts
enemy_hit_check_1_1:
    lda sp_enemy + 3, x
    and #1
    beq enemy_hit_check_2
    lda #8
    sta v_esize
    jmp enemy_hit_check_3
enemy_hit_check_2:
    lda #16
    sta v_esize
enemy_hit_check_3:
    ldy #0
enemy_hit_check_4:
    lda v_shotF, y
    beq enemy_hit_check_next
    lda sp_enemy + 0, x
    sec
    sbc #8
    cmp sp_shot + 0, y
    bcs enemy_hit_check_next
    adc #8
    adc v_esize
    cmp sp_shot + 0, y
    bcc enemy_hit_check_next
    lda sp_enemy + 1, x
    sec
    sbc #8
    cmp sp_shot + 1, y
    bcs enemy_hit_check_next
    adc #8
    adc v_esize
    cmp sp_shot + 1, y
    bcc enemy_hit_check_next
    jsr destruct_enemy
    lda #0
    sta v_shotF, y
    sta sp_shot + 2, y
enemy_hit_check_next:
    iny
    iny
    iny
    iny
    cpy #32
    beq enemy_hit_check_end
    jmp enemy_hit_check_4
enemy_hit_check_end:
    rts

;-------------------------------------------------------------------------------
; Scroll BG
;-------------------------------------------------------------------------------
scroll_bg:
    ldx v_scrollX
    stx $5408
    ldx v_scroll
    stx $5409
    inx
    stx v_scroll
    rts

;-------------------------------------------------------------------------------
; Get random value to a (no change all registers)
; NOTE: use $00 for work area
;-------------------------------------------------------------------------------
get_rand_to_a:
    php
    stx $00
    ldx v_randI
    lda rand_table, x
    inx
    stx v_randI
    ldx $00
    plp
    rts

;-------------------------------------------------------------------------------
; Scroll the background horizontally by dragging (I don't know why :)
;-------------------------------------------------------------------------------
drag_mouse_to_hscroll:
    lda v_prevT
    beq drag_mouse_to_hscroll_end
    lda $5800
    beq drag_mouse_to_hscroll_end
    lda $5801
    sec
    sbc sp_mouse + 0
    clc
    adc v_scrollX
    sta v_scrollX
drag_mouse_to_hscroll_end:
    rts

;-------------------------------------------------------------------------------
; Draw mouse cursor
;-------------------------------------------------------------------------------
draw_mouse_cursor:
    lda $5801
    sec
    sbc #8
    sta sp_mouse + 0
    lda $5802
    sec
    sbc #8
    sta sp_mouse + 1
    lda $5800
    sta v_prevT
    asl
    clc
    adc #$16
    sta sp_mouse + 2
    lda #%00000001
    sta sp_mouse + 3
    rts

;-------------------------------------------------------------------------------
; Show mouse status ($FF use for work)
;-------------------------------------------------------------------------------
show_mouse_status:
    lda #$5B ; '['
    sta $7041
    lda #$2C ; ','
    sta $7045
    lda #$5D ; ']'
    sta $7049
    ; draw ?00 of X
    lda $5801
    jsr calc_100s
    clc
    adc #$30
    sta $7042
    ; draw 0?0 of X
    lda $5801
    jsr calc_10s
    clc
    adc #$30
    sta $7043
    ; draw 00? of X
    lda $5801
    jsr calc_1s
    adc #$30
    sta $7044
    ; draw ?00 of Y
    lda $5802
    jsr calc_100s
    clc
    adc #$30
    sta $7046
    ; draw 0?0 of X
    lda $5802
    jsr calc_10s
    clc
    adc #$30
    sta $7047
    ; draw 00? of X
    lda $5802
    jsr calc_1s
    adc #$30
    sta $7048
    ; draw clicking mark
    lda $5800
    beq show_mouse_status_no_clicking
    lda #$43 ; 'C'
    sta $704A
    rts
show_mouse_status_no_clicking:
    lda #$00
    sta $704A
    rts

;-------------------------------------------------------------------------------
; Calculate the hundreds place of register A
;-------------------------------------------------------------------------------
calc_100s:
    cmp #200
    bcc calc_100s_1
    lda #2
    rts
calc_100s_1:
    cmp #100
    bcc calc_100s_2
    lda #1
    rts
calc_100s_2:
    lda #0
    rts

;-------------------------------------------------------------------------------
; Calculate the tens digit of register A (use $FF for work)
; a = (a mod 100) div 10
;-------------------------------------------------------------------------------
calc_10s:
    sta $FF
    txa
    pha
    lda $FF
    cmp #200
    bcc calc_10s_1
    sec
    sbc #200
    jmp calc_10s_2
calc_10s_1:
    cmp #100
    bcc calc_10s_2
    sec
    sbc #100
calc_10s_2:
    ldx #$FF
calc_10s_3:
    inx
    sec
    sbc #10
    bcc calc_10s_3
    stx $FF
    pla
    tax
    lda $FF
    rts

;-------------------------------------------------------------------------------
; Calculate 1's digit of register A
;-------------------------------------------------------------------------------
calc_1s:
    and #$0F
    cmp #10
    bcc calc_1s_2
    sec
    sbc #10
calc_1s_2:
    rts

;-------------------------------------------------------------------------------
; String literal definition
;-------------------------------------------------------------------------------
string_hello_world:
    .byte "Hello, VGS8 World!"

rand_table:; 乱数テーブル
    .byte   $72,$DD,$03,$89,$C9,$86,$DB,$30,$8E,$4F,$DC,$99,$67,$54,$13,$4C
    .byte   $A3,$CA,$D8,$28,$50,$0F,$8B,$87,$6B,$B9,$10,$CF,$EC,$40,$FD,$B6
    .byte   $F3,$AF,$70,$56,$74,$CC,$47,$60,$B4,$0C,$80,$16,$D7,$79,$61,$BC
    .byte   $C8,$11,$B0,$A8,$FE,$5A,$B5,$62,$A6,$6E,$6D,$77,$CE,$32,$4D,$55
    .byte   $9E,$09,$21,$83,$9A,$F8,$E5,$BF,$8A,$BA,$2E,$4B,$EB,$9F,$D3,$36
    .byte   $CB,$07,$44,$EE,$E4,$9D,$73,$5F,$F4,$00,$1E,$78,$3A,$EA,$D1,$BD
    .byte   $A0,$D6,$C0,$3D,$66,$19,$FB,$92,$C7,$53,$7C,$D4,$98,$E7,$C4,$AC
    .byte   $7F,$7A,$93,$B8,$0A,$8C,$A2,$06,$E8,$A5,$29,$88,$BE,$CD,$0E,$DA
    .byte   $5B,$5C,$A9,$02,$7E,$EF,$ED,$D9,$E6,$F1,$3E,$05,$45,$8D,$F6,$23
    .byte   $E3,$FC,$4A,$D2,$91,$F9,$20,$C3,$04,$68,$18,$49,$2F,$94,$2C,$2B
    .byte   $01,$2D,$27,$1D,$6A,$9B,$12,$A1,$A4,$DF,$76,$6C,$64,$69,$1C,$C6
    .byte   $08,$AD,$3F,$96,$65,$C2,$38,$5D,$7B,$E2,$97,$41,$1F,$E0,$9C,$B3
    .byte   $48,$84,$51,$8F,$22,$5E,$46,$57,$31,$37,$4E,$0D,$58,$AE,$AB,$7D
    .byte   $B2,$FF,$AA,$D0,$59,$3C,$35,$26,$34,$24,$A7,$1A,$52,$15,$95,$C1
    .byte   $17,$71,$BB,$25,$82,$75,$3B,$E1,$14,$C5,$0B,$F5,$6F,$E9,$DE,$F2
    .byte   $42,$33,$43,$B1,$F0,$90,$B7,$39,$85,$D5,$81,$1B,$63,$2A,$FA,$F7

;-------------------------------------------------------------------------------
; WRAM (variable labels)
;-------------------------------------------------------------------------------
.org $0200
v_scroll:   .byte $00
v_scrollX:  .byte $00
v_shotW:    .byte $00 ; wait fire flag
v_shotI:    .byte $00 ; index of the player shot
v_shotF:    .byte $00, $00, $00, $00    ; flags of the player shot[0]
            .byte $00, $00, $00, $00    ; flags of the player shot[1]
            .byte $00, $00, $00, $00    ; flags of the player shot[2]
            .byte $00, $00, $00, $00    ; flags of the player shot[3]
            .byte $00, $00, $00, $00    ; flags of the player shot[4]
            .byte $00, $00, $00, $00    ; flags of the player shot[5]
            .byte $00, $00, $00, $00    ; flags of the player shot[6]
            .byte $00, $00, $00, $00    ; flags of the player shot[7]
v_randI:    .byte $00 ; random index
v_enemyT:   .byte $00 ; enemy timer
v_enemyI:   .byte $00 ; enemy index
v_enemy:    .byte $00, $00, $00, $00    ; enemy[0] vars (flag, type, v1, v2)
            .byte $00, $00, $00, $00    ; enemy[1] vars
            .byte $00, $00, $00, $00    ; enemy[2] vars
            .byte $00, $00, $00, $00    ; enemy[3] vars
            .byte $00, $00, $00, $00    ; enemy[4] vars
            .byte $00, $00, $00, $00    ; enemy[5] vars
            .byte $00, $00, $00, $00    ; enemy[6] vars
            .byte $00, $00, $00, $00    ; enemy[7] vars
            .byte $00, $00, $00, $00    ; enemy[8] vars
            .byte $00, $00, $00, $00    ; enemy[9] vars
            .byte $00, $00, $00, $00    ; enemy[10] vars
            .byte $00, $00, $00, $00    ; enemy[11] vars
            .byte $00, $00, $00, $00    ; enemy[12] vars
            .byte $00, $00, $00, $00    ; enemy[13] vars
            .byte $00, $00, $00, $00    ; enemy[14] vars
            .byte $00, $00, $00, $00    ; enemy[15] vars
v_esize:    .byte $00 ; work area for hit check
v_prevT:    .byte $00 ; mouse status of previous frame
v_prevX:    .byte $00 ; mouse X of previous frame

;-------------------------------------------------------------------------------
; Sprite OAM labels
;-------------------------------------------------------------------------------
.org $5000  ;     X    Y    PTN  FLG
sp_player:  .byte $00, $00, $00, $00    ; player
sp_shot:    .byte $00, $00, $00, $00    ; shot[0]
            .byte $00, $00, $00, $00    ; shot[1]
            .byte $00, $00, $00, $00    ; shot[2]
            .byte $00, $00, $00, $00    ; shot[3]
            .byte $00, $00, $00, $00    ; shot[4]
            .byte $00, $00, $00, $00    ; shot[5]
            .byte $00, $00, $00, $00    ; shot[6]
            .byte $00, $00, $00, $00    ; shot[7]
sp_enemy:   .byte $00, $00, $00, $00    ; enemy[0]
            .byte $00, $00, $00, $00    ; enemy[1]
            .byte $00, $00, $00, $00    ; enemy[2]
            .byte $00, $00, $00, $00    ; enemy[3]
            .byte $00, $00, $00, $00    ; enemy[4]
            .byte $00, $00, $00, $00    ; enemy[5]
            .byte $00, $00, $00, $00    ; enemy[6]
            .byte $00, $00, $00, $00    ; enemy[7]
            .byte $00, $00, $00, $00    ; enemy[8]
            .byte $00, $00, $00, $00    ; enemy[9]
            .byte $00, $00, $00, $00    ; enemy[10]
            .byte $00, $00, $00, $00    ; enemy[11]
            .byte $00, $00, $00, $00    ; enemy[12]
            .byte $00, $00, $00, $00    ; enemy[13]
            .byte $00, $00, $00, $00    ; enemy[14]
            .byte $00, $00, $00, $00    ; enemy[15]
sp_mouse:   .byte $00, $00, $00, $00    ; mouse
