INLOOP  TD      INDEV
        JEQ     INLOOP
        RD      INDEV
        STCH    DATA

OUTLP   TD      OUTDEV
        JEQ     OUTLP
        LDCH    DATA
        WD      OUTDEV

INDEV   BYTE    0
OUTDEV  BYTE    1
DATA    RESB    1