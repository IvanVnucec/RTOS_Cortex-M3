/* ------------- assembler directives -----------------------------*/
    .syntax unified
    .thumb


/* ------------- public functions definitions ---------------------*/
    .globl      PendSV_Handler


/* ------------- public variable definitions ----------------------*/
    .globl      OS_TCBCurrent
    .globl      OS_TCBNext


/* ------------- public functions declarations --------------------*/
    .thumb_func
PendSV_Handler:
    PUSH      {R4-R11}

    /* save current sp */
    LDR       R0, =OS_TCBCurrent
    LDR       R0, [R0]
    CBZ       R0, PendSV_noSave

    STR       SP, [R0]
PendSV_noSave:
    /* restore next sp */
    LDR       R0, =OS_TCBNext
    LDR       R0, [R0]
    LDR       SP, [R0]

    POP       {R4-R11}

    BX        LR
