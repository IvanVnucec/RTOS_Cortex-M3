    .syntax unified
    .thumb

    .globl      PendSV_Handler
    .globl      OS_TCBCurrent
    .globl      OS_TCBNext

    .thumb_func
PendSV_Handler:
    //PUSH    {R4-R11}
    //
    ///* SP = taskXsp */
    //LDR     R0, =task1sp
    //LDR     R0, [R0]
    //MOV     SP, R0

    //LDR     R0, =task2sp  // R0 = &task2sp
    //LDR     R1, [R0]      // R1 = task2sp
    //LDR     R2, =task1sp  // R2 = &task1sp
    //LDR     R3, [R2]      // R3 = task1sp
    //STR     R1, [R2]      // task1sp = task2sp
    //STR     R3, [R0]      // task2sp = task1sp
    //
    //POP    {R4-R11}

    //BX      LR

    PUSH      {R4-R11}

    // save current sp
    LDR       R0, =OS_TCBCurrent
    LDR       R0, [R0]
    CBZ       R0, PendSV_noSave

    STR       SP, [R0] 

PendSV_noSave:
    /* SP = OS_taskNext->sp */
    LDR       R0, =OS_TCBNext
    LDR       R0, [R0]
    LDR       SP, [R0]

    POP       {R4-R11}

    BX        LR