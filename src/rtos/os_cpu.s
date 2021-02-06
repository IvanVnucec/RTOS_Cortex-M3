/**
 * \file            os_cpu.s
 * \brief           OS CPU assembly file
 */

/* ------------- assembler directives -----------------------------*/
    .syntax unified
    .thumb


/* ------------- public functions definitions ---------------------*/
    .globl      pend_sv_handler


/* ------------- public variable definitions ----------------------*/
    .globl      OS_TCBCurrent
    .globl      OS_TCBNext


/* ------------- public functions declarations --------------------*/

/**
  * @brief  		Will do the context switching between tasks.
  */
    .thumb_func
pend_sv_handler:
    PUSH      {R4-R11}

    /* save current sp */
    LDR       R0, =OS_TCBCurrent
    LDR       R1, [R0]
    CBZ       R1, PendSV_noSave

    STR       SP, [R1]
PendSV_noSave:
    /* restore next sp */
    LDR       R2, =OS_TCBNext
    LDR       R3, [R2]
    LDR       SP, [R3]

    POP       {R4-R11}

    /* OS_TCBCurrent = OS_TCBNext */
    STR       R3, [R0]

    BX        LR
