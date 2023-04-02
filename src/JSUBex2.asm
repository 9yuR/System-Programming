        JSUB    READ

READ    LDX     #0
        LDT     #100
RLOOP   TD      INDEV
        JEQ     RLOOP
        RD      INDEV
        STCH    RECORD,X
        TIXR    T
        JLT     RLOOP
        RSUB

INDEV   BYTE    0
RECORD  RESB    100