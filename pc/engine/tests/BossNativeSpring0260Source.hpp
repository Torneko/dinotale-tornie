#pragma once
// Verbatim native queue removal/attachment CPU oracle; no model substitutes.
// funcs_30.c SHA256 fffbaae35022375e23181b0abcd15ed5502098ad6909a6533f4afc03e8fd7a5a; func_80043C94 sliceSHA 89f3a6299163dea16fce9ea3ca2895f6ce843fa0cc9d2c56bc3aa72066b9c4f7
// funcs_30.c SHA256 fffbaae35022375e23181b0abcd15ed5502098ad6909a6533f4afc03e8fd7a5a; func_800444D0 sliceSHA 4cddf9f4423daf4ba26d76b2e9e2064f4fe11c6e32e4191058d973385def5d0e
// funcs_32.c SHA256 3fea1182b280d4ea7958ded62c3435c3db492899197f6eac95b959cc09b17fc8; func_80047F9C sliceSHA e6c69490c102f9819af6c6585a0c5123be58de00111855b36339c43224331067
// funcs_32.c SHA256 3fea1182b280d4ea7958ded62c3435c3db492899197f6eac95b959cc09b17fc8; func_80048114 sliceSHA b71a768417877ea0f17deca2c9667903f804c383fcc62c36543236ff1f04ac78
namespace native_spring_source0260 {
void Fault_HungUp(uint8_t*,recomp_context*);
void func_80043C94(uint8_t*,recomp_context*);
void func_800444D0(uint8_t*,recomp_context*);
void func_80047F9C(uint8_t*,recomp_context*);
void func_80048114(uint8_t*,recomp_context*);
RECOMP_FUNC void func_80043C94(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80043C94: sll         $t6, $a0, 4
    ctx->r14 = S32(ctx->r4 << 4);
    // 0x80043C98: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80043C9C: srl         $t7, $t6, 28
    ctx->r15 = S32(U32(ctx->r14) >> 28);
    // 0x80043CA0: beq         $t7, $zero, L_80043CC0
    if (ctx->r15 == 0) {
        // 0x80043CA4: sw          $ra, 0x14($sp)
        MEM_W(0X14, ctx->r29) = ctx->r31;
            goto L_80043CC0;
    }
    // 0x80043CA4: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80043CA8: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80043CAC: addiu       $a0, $a0, 0x390
    ctx->r4 = ADD32(ctx->r4, 0X390);
    // 0x80043CB0: jal         0x80071380
    // 0x80043CB4: addiu       $a1, $zero, 0x261
    ctx->r5 = ADD32(0, 0X261);
    Fault_HungUp(rdram, ctx);
        goto after_0;
    // 0x80043CB4: addiu       $a1, $zero, 0x261
    ctx->r5 = ADD32(0, 0X261);
    after_0:
    // 0x80043CB8: b           L_80043CFC
    // 0x80043CBC: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
        goto L_80043CFC;
    // 0x80043CBC: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
L_80043CC0:
    // 0x80043CC0: lh          $t8, 0x17E($a0)
    ctx->r24 = MEM_H(ctx->r4, 0X17E);
    // 0x80043CC4: addiu       $v1, $zero, 0x96
    ctx->r3 = ADD32(0, 0X96);
    // 0x80043CC8: addiu       $t9, $zero, 0x1
    ctx->r25 = ADD32(0, 0X1);
    // 0x80043CCC: bne         $v1, $t8, L_80043CEC
    if (ctx->r3 != ctx->r24) {
        // 0x80043CD0: lui         $at, 0x800E
        ctx->r1 = S32(0X800E << 16);
            goto L_80043CEC;
    }
    // 0x80043CD0: lui         $at, 0x800E
    ctx->r1 = S32(0X800E << 16);
    // 0x80043CD4: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80043CD8: addiu       $a0, $a0, 0x3A0
    ctx->r4 = ADD32(ctx->r4, 0X3A0);
    // 0x80043CDC: jal         0x80071380
    // 0x80043CE0: addiu       $a1, $zero, 0x26B
    ctx->r5 = ADD32(0, 0X26B);
    Fault_HungUp(rdram, ctx);
        goto after_1;
    // 0x80043CE0: addiu       $a1, $zero, 0x26B
    ctx->r5 = ADD32(0, 0X26B);
    after_1:
    // 0x80043CE4: b           L_80043CFC
    // 0x80043CE8: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
        goto L_80043CFC;
    // 0x80043CE8: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
L_80043CEC:
    // 0x80043CEC: sh          $v1, 0x17E($a0)
    MEM_H(0X17E, ctx->r4) = ctx->r3;
    // 0x80043CF0: sb          $zero, 0x17D($a0)
    MEM_B(0X17D, ctx->r4) = 0;
    // 0x80043CF4: sb          $t9, -0x7EB0($at)
    MEM_B(-0X7EB0, ctx->r1) = ctx->r25;
    // 0x80043CF8: addiu       $v0, $zero, 0x1
    ctx->r2 = ADD32(0, 0X1);
L_80043CFC:
    // 0x80043CFC: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80043D00: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80043D04: jr          $ra
    // 0x80043D08: nop

    return;
    // 0x80043D08: nop

;}
RECOMP_FUNC void func_800444D0(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x800444D0: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x800444D4: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x800444D8: lh          $t6, 0x17E($a0)
    ctx->r14 = MEM_H(ctx->r4, 0X17E);
    // 0x800444DC: or          $a2, $a0, $zero
    ctx->r6 = ctx->r4 | 0;
    // 0x800444E0: addiu       $at, $zero, 0x96
    ctx->r1 = ADD32(0, 0X96);
    // 0x800444E4: bne         $t6, $at, L_80044500
    if (ctx->r14 != ctx->r1) {
        // 0x800444E8: lui         $a0, 0x800B
        ctx->r4 = S32(0X800B << 16);
            goto L_80044500;
    }
    // 0x800444E8: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x800444EC: addiu       $a0, $a0, 0x480
    ctx->r4 = ADD32(ctx->r4, 0X480);
    // 0x800444F0: jal         0x80071380
    // 0x800444F4: addiu       $a1, $zero, 0x4A2
    ctx->r5 = ADD32(0, 0X4A2);
    Fault_HungUp(rdram, ctx);
        goto after_0;
    // 0x800444F4: addiu       $a1, $zero, 0x4A2
    ctx->r5 = ADD32(0, 0X4A2);
    after_0:
    // 0x800444F8: b           L_80044570
    // 0x800444FC: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
        goto L_80044570;
    // 0x800444FC: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80044500:
    // 0x80044500: lbu         $t7, 0x17D($a2)
    ctx->r15 = MEM_BU(ctx->r6, 0X17D);
    // 0x80044504: addiu       $at, $zero, 0x2
    ctx->r1 = ADD32(0, 0X2);
    // 0x80044508: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x8004450C: beq         $t7, $at, L_80044524
    if (ctx->r15 == ctx->r1) {
        // 0x80044510: addiu       $a0, $a0, 0x490
        ctx->r4 = ADD32(ctx->r4, 0X490);
            goto L_80044524;
    }
    // 0x80044510: addiu       $a0, $a0, 0x490
    ctx->r4 = ADD32(ctx->r4, 0X490);
    // 0x80044514: addiu       $a1, $zero, 0x4AC
    ctx->r5 = ADD32(0, 0X4AC);
    // 0x80044518: jal         0x80071380
    // 0x8004451C: sw          $a2, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r6;
    Fault_HungUp(rdram, ctx);
        goto after_1;
    // 0x8004451C: sw          $a2, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r6;
    after_1:
    // 0x80044520: lw          $a2, 0x18($sp)
    ctx->r6 = MEM_W(ctx->r29, 0X18);
L_80044524:
    // 0x80044524: lw          $t8, 0x180($a2)
    ctx->r24 = MEM_W(ctx->r6, 0X180);
    // 0x80044528: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x8004452C: addiu       $a0, $a0, 0x4A0
    ctx->r4 = ADD32(ctx->r4, 0X4A0);
    // 0x80044530: beq         $t8, $zero, L_80044548
    if (ctx->r24 == 0) {
        // 0x80044534: lui         $t0, 0x8010
        ctx->r8 = S32(0X8010 << 16);
            goto L_80044548;
    }
    // 0x80044534: lui         $t0, 0x8010
    ctx->r8 = S32(0X8010 << 16);
    // 0x80044538: jal         0x80071380
    // 0x8004453C: addiu       $a1, $zero, 0x4B5
    ctx->r5 = ADD32(0, 0X4B5);
    Fault_HungUp(rdram, ctx);
        goto after_2;
    // 0x8004453C: addiu       $a1, $zero, 0x4B5
    ctx->r5 = ADD32(0, 0X4B5);
    after_2:
    // 0x80044540: b           L_80044570
    // 0x80044544: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
        goto L_80044570;
    // 0x80044544: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80044548:
    // 0x80044548: lw          $t0, -0x77F0($t0)
    ctx->r8 = MEM_W(ctx->r8, -0X77F0);
    // 0x8004454C: lh          $t2, 0x17E($a2)
    ctx->r10 = MEM_H(ctx->r6, 0X17E);
    // 0x80044550: addiu       $t9, $zero, 0x1
    ctx->r25 = ADD32(0, 0X1);
    // 0x80044554: lw          $t1, 0x10($t0)
    ctx->r9 = MEM_W(ctx->r8, 0X10);
    // 0x80044558: sll         $t3, $t2, 2
    ctx->r11 = S32(ctx->r10 << 2);
    // 0x8004455C: or          $a0, $a2, $zero
    ctx->r4 = ctx->r6 | 0;
    // 0x80044560: addu        $t4, $t1, $t3
    ctx->r12 = ADD32(ctx->r9, ctx->r11);
    // 0x80044564: jal         0x80043C94
    // 0x80044568: sw          $t9, 0x0($t4)
    MEM_W(0X0, ctx->r12) = ctx->r25;
    func_80043C94(rdram, ctx);
        goto after_3;
    // 0x80044568: sw          $t9, 0x0($t4)
    MEM_W(0X0, ctx->r12) = ctx->r25;
    after_3:
    // 0x8004456C: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80044570:
    // 0x80044570: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80044574: jr          $ra
    // 0x80044578: nop

    return;
    // 0x80044578: nop

;}
RECOMP_FUNC void func_80047F9C(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80047F9C: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80047FA0: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80047FA4: lh          $t6, 0x184($a0)
    ctx->r14 = MEM_H(ctx->r4, 0X184);
    // 0x80047FA8: slti        $at, $t6, 0x14
    ctx->r1 = SIGNED(ctx->r14) < 0X14 ? 1 : 0;
    // 0x80047FAC: bnel        $at, $zero, L_80047FD0
    if (ctx->r1 != 0) {
        // 0x80047FB0: lw          $t7, 0x180($a1)
        ctx->r15 = MEM_W(ctx->r5, 0X180);
            goto L_80047FD0;
    }
    goto skip_0;
    // 0x80047FB0: lw          $t7, 0x180($a1)
    ctx->r15 = MEM_W(ctx->r5, 0X180);
    skip_0:
    // 0x80047FB4: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80047FB8: addiu       $a0, $a0, 0x7A0
    ctx->r4 = ADD32(ctx->r4, 0X7A0);
    // 0x80047FBC: jal         0x80071380
    // 0x80047FC0: addiu       $a1, $zero, 0x966
    ctx->r5 = ADD32(0, 0X966);
    Fault_HungUp(rdram, ctx);
        goto after_0;
    // 0x80047FC0: addiu       $a1, $zero, 0x966
    ctx->r5 = ADD32(0, 0X966);
    after_0:
    // 0x80047FC4: b           L_80048054
    // 0x80047FC8: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
        goto L_80048054;
    // 0x80047FC8: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80047FCC: lw          $t7, 0x180($a1)
    ctx->r15 = MEM_W(ctx->r5, 0X180);
L_80047FD0:
    // 0x80047FD0: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
    // 0x80047FD4: beq         $t7, $zero, L_80047FF4
    if (ctx->r15 == 0) {
        // 0x80047FD8: nop
    
            goto L_80047FF4;
    }
    // 0x80047FD8: nop

    // 0x80047FDC: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80047FE0: addiu       $a0, $a0, 0x7B0
    ctx->r4 = ADD32(ctx->r4, 0X7B0);
    // 0x80047FE4: jal         0x80071380
    // 0x80047FE8: addiu       $a1, $zero, 0x970
    ctx->r5 = ADD32(0, 0X970);
    Fault_HungUp(rdram, ctx);
        goto after_1;
    // 0x80047FE8: addiu       $a1, $zero, 0x970
    ctx->r5 = ADD32(0, 0X970);
    after_1:
    // 0x80047FEC: b           L_80048054
    // 0x80047FF0: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
        goto L_80048054;
    // 0x80047FF0: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80047FF4:
    // 0x80047FF4: sll         $t8, $v0, 2
    ctx->r24 = S32(ctx->r2 << 2);
L_80047FF8:
    // 0x80047FF8: addu        $v1, $a0, $t8
    ctx->r3 = ADD32(ctx->r4, ctx->r24);
    // 0x80047FFC: lw          $t9, 0x188($v1)
    ctx->r25 = MEM_W(ctx->r3, 0X188);
    // 0x80048000: bnel        $t9, $zero, L_8004802C
    if (ctx->r25 != 0) {
        // 0x80048004: addiu       $v0, $v0, 0x1
        ctx->r2 = ADD32(ctx->r2, 0X1);
            goto L_8004802C;
    }
    goto skip_1;
    // 0x80048004: addiu       $v0, $v0, 0x1
    ctx->r2 = ADD32(ctx->r2, 0X1);
    skip_1:
    // 0x80048008: sw          $a1, 0x188($v1)
    MEM_W(0X188, ctx->r3) = ctx->r5;
    // 0x8004800C: sb          $zero, 0x17C($a1)
    MEM_B(0X17C, ctx->r5) = 0;
    // 0x80048010: sh          $v0, 0x17E($a1)
    MEM_H(0X17E, ctx->r5) = ctx->r2;
    // 0x80048014: sw          $a0, 0x180($a1)
    MEM_W(0X180, ctx->r5) = ctx->r4;
    // 0x80048018: lh          $t0, 0x184($a0)
    ctx->r8 = MEM_H(ctx->r4, 0X184);
    // 0x8004801C: addiu       $t1, $t0, 0x1
    ctx->r9 = ADD32(ctx->r8, 0X1);
    // 0x80048020: b           L_80048050
    // 0x80048024: sh          $t1, 0x184($a0)
    MEM_H(0X184, ctx->r4) = ctx->r9;
        goto L_80048050;
    // 0x80048024: sh          $t1, 0x184($a0)
    MEM_H(0X184, ctx->r4) = ctx->r9;
    // 0x80048028: addiu       $v0, $v0, 0x1
    ctx->r2 = ADD32(ctx->r2, 0X1);
L_8004802C:
    // 0x8004802C: sll         $v0, $v0, 16
    ctx->r2 = S32(ctx->r2 << 16);
    // 0x80048030: sra         $v0, $v0, 16
    ctx->r2 = S32(SIGNED(ctx->r2) >> 16);
    // 0x80048034: slti        $at, $v0, 0x14
    ctx->r1 = SIGNED(ctx->r2) < 0X14 ? 1 : 0;
    // 0x80048038: bnel        $at, $zero, L_80047FF8
    if (ctx->r1 != 0) {
        // 0x8004803C: sll         $t8, $v0, 2
        ctx->r24 = S32(ctx->r2 << 2);
            goto L_80047FF8;
    }
    goto skip_2;
    // 0x8004803C: sll         $t8, $v0, 2
    ctx->r24 = S32(ctx->r2 << 2);
    skip_2:
    // 0x80048040: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80048044: addiu       $a0, $a0, 0x7C0
    ctx->r4 = ADD32(ctx->r4, 0X7C0);
    // 0x80048048: jal         0x80071380
    // 0x8004804C: addiu       $a1, $zero, 0x980
    ctx->r5 = ADD32(0, 0X980);
    Fault_HungUp(rdram, ctx);
        goto after_2;
    // 0x8004804C: addiu       $a1, $zero, 0x980
    ctx->r5 = ADD32(0, 0X980);
    after_2:
L_80048050:
    // 0x80048050: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80048054:
    // 0x80048054: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80048058: jr          $ra
    // 0x8004805C: nop

    return;
    // 0x8004805C: nop

;}
RECOMP_FUNC void func_80048114(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80048114: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80048118: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x8004811C: sw          $a0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r4;
    // 0x80048120: lw          $t6, 0x180($a1)
    ctx->r14 = MEM_W(ctx->r5, 0X180);
    // 0x80048124: or          $a0, $a1, $zero
    ctx->r4 = ctx->r5 | 0;
    // 0x80048128: beq         $t6, $zero, L_80048148
    if (ctx->r14 == 0) {
        // 0x8004812C: nop
    
            goto L_80048148;
    }
    // 0x8004812C: nop

    // 0x80048130: lui         $a0, 0x800B
    ctx->r4 = S32(0X800B << 16);
    // 0x80048134: addiu       $a0, $a0, 0x7E0
    ctx->r4 = ADD32(ctx->r4, 0X7E0);
    // 0x80048138: jal         0x80071380
    // 0x8004813C: addiu       $a1, $zero, 0x9C2
    ctx->r5 = ADD32(0, 0X9C2);
    Fault_HungUp(rdram, ctx);
        goto after_0;
    // 0x8004813C: addiu       $a1, $zero, 0x9C2
    ctx->r5 = ADD32(0, 0X9C2);
    after_0:
    // 0x80048140: b           L_80048160
    // 0x80048144: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
        goto L_80048160;
    // 0x80048144: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80048148:
    // 0x80048148: jal         0x800444D0
    // 0x8004814C: sw          $a1, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r5;
    func_800444D0(rdram, ctx);
        goto after_1;
    // 0x8004814C: sw          $a1, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r5;
    after_1:
    // 0x80048150: lw          $a1, 0x1C($sp)
    ctx->r5 = MEM_W(ctx->r29, 0X1C);
    // 0x80048154: jal         0x80047F9C
    // 0x80048158: lw          $a0, 0x18($sp)
    ctx->r4 = MEM_W(ctx->r29, 0X18);
    func_80047F9C(rdram, ctx);
        goto after_2;
    // 0x80048158: lw          $a0, 0x18($sp)
    ctx->r4 = MEM_W(ctx->r29, 0X18);
    after_2:
    // 0x8004815C: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
L_80048160:
    // 0x80048160: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80048164: jr          $ra
    // 0x80048168: nop

    return;
    // 0x80048168: nop

;}
}
