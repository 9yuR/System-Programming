        JSUB    READ

READ    LDX     ZERO
RLOOP   TD      INDEV
        JEQ     RLOOP
        RD      INDEV
        STCH    RECORD,X
        TIX     K100
        JLT     RLOOP
        RSUB

INDEV   BYTE    0
RECORD  RESB    100

ZERO    WORD    0
K100    WORD    100