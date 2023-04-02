        LDX     #0
        STX     STK
        STX     BALL
        STX     JDX
LOOP    
        STX     IDX
        LDCH    ANSWER, X
        STA     ALPHA
        LDCH    GUESS, X
        COMP    ALPHA
        JEQ     STRIKE

BALL_CHECK
        LDS     IDX
        LDX     JDX
        COMPR   X, S
        JEQ     JDXINCR
        LDCH    GUESS, X
        COMP    ALPHA
        JEQ     ISBALL
        TIX     STRLEN
        STX     JDX
        JLT     BALL_CHECK
        J       IDXINCR

STRIKE    
        LDA     STK 
        ADD     #1
        STA     STK
        LDA     OUT
        SUB     #1
        STA     OUT
        J       IDXINCR

ISBALL  
        LDA     BALL
        ADD     #1
        STA     BALL
        LDA     OUT
        SUB     #1
        STA     OUT
        J       IDXINCR

IDXINCR 
        LDS     #0
        STS     JDX
        LDX     IDX
        TIX     STRLEN
        JLT     LOOP

JDXINCR
        LDA     JDX
        ADD     #1
        STA     JDX
        J       BALL_CHECK


ANSWER  BYTE    C'734'
GUESS   BYTE    C'123'

STRLEN  WORD    3

ALPHA   RESW    1   
IDX     RESW    1   
JDX     RESW    1   

STK     RESW    1
BALL    RESW    1
OUT     WORD    3





