.entry TARGET
.extern EXT1
.extern EXT2

MAIN:   mov EXT1, r3
        add r3, r1
        bne EXT2
TARGET: prn #-10
        stop
VEC:    .data 10, 20, -5
M2:     .mat[2][2] 7,8,9,10
