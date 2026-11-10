#pragma once
// CPU oracle only: verbatim generated functions, not hand-translated models.
// funcs_10.c SHA256 75d0de66fc4dba4d1c7b9787ec8179ff9a0ea3aaa516c1b4e78a885e8c5b3cf1; func_80022A58 normalized-slice SHA256 cebcfd14a91729098f37574025ecf1a6da909e104c7d4325848b17d91d55ed4e
// funcs_10.c SHA256 75d0de66fc4dba4d1c7b9787ec8179ff9a0ea3aaa516c1b4e78a885e8c5b3cf1; func_80022B20 normalized-slice SHA256 8ad069c0b221a738a7bfe15dd4a1c447921d3744fd3688003e046eafdc0e5f1b
// funcs_47.c SHA256 961696f653a51c33b3b33dfd341d0b873d1873a2cedf114b6931f70bc80081c5; func_80065954 normalized-slice SHA256 b10d5fe2f1f461ca072dd613f30d81325e90597dc2640f08d0397b771bb0c2d8
// funcs_47.c SHA256 961696f653a51c33b3b33dfd341d0b873d1873a2cedf114b6931f70bc80081c5; func_80066DE4 normalized-slice SHA256 08d91f8c97c59fff653fd9dd7e508a9a6eeb01ad7b90d079b1972ebc2d82c03e
// funcs_47.c SHA256 961696f653a51c33b3b33dfd341d0b873d1873a2cedf114b6931f70bc80081c5; func_80066E88 normalized-slice SHA256 20b267a7e5178f78fafde728875559dae75701a404a49b3c78e46c053ecbac0a
// funcs_60.c SHA256 3cb85daffbfab22587543e67d41cb0507a6a012a2292c43fa944995eb570579e; Math_FAtanContFracF normalized-slice SHA256 fec3de10a418c610bf3d440dc325de6cf4ebdbe07869e06a492d1f8bdf3bbf9f
// funcs_60.c SHA256 3cb85daffbfab22587543e67d41cb0507a6a012a2292c43fa944995eb570579e; fatan2 normalized-slice SHA256 2c077c701dbd0130dd23f0c3833b97f444325a6d30ab42e76f9df15dfe5a0338
// funcs_60.c SHA256 3cb85daffbfab22587543e67d41cb0507a6a012a2292c43fa944995eb570579e; sins normalized-slice SHA256 5708bcc6972a1d11343852086b9e0cf77fdaac2025e2eac0ef9e7218d4a17412
// funcs_61.c SHA256 98b27faa3e1fd6dbae9fee46f69f83012fe4e28c1c8d362aa6e196bb6cb29458; coss normalized-slice SHA256 78f6d77d0d479de04d66931829f415eb256ee1d286e268cd9880f03a8dd71b88
// ultramodern/src/input.cpp SHA256 c903138cf52682cfeecacb65f4a632c0dff57230955d76c370b7d2ea5b4348d1
namespace native_stick_source0260 {
void func_80022AC0(uint8_t*,recomp_context*);
void func_80022A58(uint8_t*,recomp_context*);
void Math_FAtanContFracF(uint8_t*,recomp_context*);
void fatan2(uint8_t*,recomp_context*);
void func_80065954(uint8_t*,recomp_context*);
void sins(uint8_t*,recomp_context*);
void coss(uint8_t*,recomp_context*);
void func_80066DE4(uint8_t*,recomp_context*);
void func_80066E88(uint8_t*,recomp_context*);
void func_80022B20(uint8_t*,recomp_context*);
void convert_to_n64_range(float x, float y, int8_t& stick_x, int8_t& stick_y) {
    constexpr float PI = 3.14159265358979323846f;
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude > 1.0f) {
        magnitude = 1.0f;
    }

    float angle = atan2f(y, x);
    float angle_modulo = fmodf(angle + 2.0f * PI, PI / 2.0f);
    if (angle_modulo > PI / 4.0f) {
        angle_modulo = PI / 2.0f - angle_modulo;
    }

    constexpr float r0 = 82.0f;
    constexpr float alpha = 1.39414574f;
    float square_radius = 1.0f;
    float n64_radius = r0 * sin(alpha) / (sin(PI - angle_modulo - alpha));
    float output_magnitude = magnitude * n64_radius / square_radius;
    stick_x = (int8_t)(cosf(angle) * output_magnitude);
    stick_y = (int8_t)(sinf(angle) * output_magnitude);
}
RECOMP_FUNC void func_80022A58(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80022A58: addu        $a0, $a0, $a1
    ctx->r4 = ADD32(ctx->r4, ctx->r5);
    // 0x80022A5C: sra         $a0, $a0, 1
    ctx->r4 = S32(SIGNED(ctx->r4) >> 1);
    // 0x80022A60: slti        $at, $a0, 0xF
    ctx->r1 = SIGNED(ctx->r4) < 0XF ? 1 : 0;
    // 0x80022A64: bnel        $at, $zero, L_80022A90
    if (ctx->r1 != 0) {
        // 0x80022A68: slti        $at, $a0, -0xE
        ctx->r1 = SIGNED(ctx->r4) < -0XE ? 1 : 0;
            goto L_80022A90;
    }
    goto skip_0;
    // 0x80022A68: slti        $at, $a0, -0xE
    ctx->r1 = SIGNED(ctx->r4) < -0XE ? 1 : 0;
    skip_0:
    // 0x80022A6C: addiu       $a0, $a0, -0xE
    ctx->r4 = ADD32(ctx->r4, -0XE);
    // 0x80022A70: slti        $at, $a0, 0x37
    ctx->r1 = SIGNED(ctx->r4) < 0X37 ? 1 : 0;
    // 0x80022A74: bnel        $at, $zero, L_80022A84
    if (ctx->r1 != 0) {
        // 0x80022A78: sll         $v0, $a0, 16
        ctx->r2 = S32(ctx->r4 << 16);
            goto L_80022A84;
    }
    goto skip_1;
    // 0x80022A78: sll         $v0, $a0, 16
    ctx->r2 = S32(ctx->r4 << 16);
    skip_1:
    // 0x80022A7C: addiu       $a0, $zero, 0x36
    ctx->r4 = ADD32(0, 0X36);
    // 0x80022A80: sll         $v0, $a0, 16
    ctx->r2 = S32(ctx->r4 << 16);
L_80022A84:
    // 0x80022A84: jr          $ra
    // 0x80022A88: sra         $v0, $v0, 16
    ctx->r2 = S32(SIGNED(ctx->r2) >> 16);
    return;
    // 0x80022A88: sra         $v0, $v0, 16
    ctx->r2 = S32(SIGNED(ctx->r2) >> 16);
    // 0x80022A8C: slti        $at, $a0, -0xE
    ctx->r1 = SIGNED(ctx->r4) < -0XE ? 1 : 0;
L_80022A90:
    // 0x80022A90: beq         $at, $zero, L_80022AB8
    if (ctx->r1 == 0) {
        // 0x80022A94: or          $v0, $zero, $zero
        ctx->r2 = 0 | 0;
            goto L_80022AB8;
    }
    // 0x80022A94: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
    // 0x80022A98: addiu       $a0, $a0, 0xE
    ctx->r4 = ADD32(ctx->r4, 0XE);
    // 0x80022A9C: slti        $at, $a0, -0x36
    ctx->r1 = SIGNED(ctx->r4) < -0X36 ? 1 : 0;
    // 0x80022AA0: beql        $at, $zero, L_80022AB0
    if (ctx->r1 == 0) {
        // 0x80022AA4: sll         $v0, $a0, 16
        ctx->r2 = S32(ctx->r4 << 16);
            goto L_80022AB0;
    }
    goto skip_2;
    // 0x80022AA4: sll         $v0, $a0, 16
    ctx->r2 = S32(ctx->r4 << 16);
    skip_2:
    // 0x80022AA8: addiu       $a0, $zero, -0x36
    ctx->r4 = ADD32(0, -0X36);
    // 0x80022AAC: sll         $v0, $a0, 16
    ctx->r2 = S32(ctx->r4 << 16);
L_80022AB0:
    // 0x80022AB0: jr          $ra
    // 0x80022AB4: sra         $v0, $v0, 16
    ctx->r2 = S32(SIGNED(ctx->r2) >> 16);
    return;
    // 0x80022AB4: sra         $v0, $v0, 16
    ctx->r2 = S32(SIGNED(ctx->r2) >> 16);
L_80022AB8:
    // 0x80022AB8: jr          $ra
    // 0x80022ABC: nop

    return;
    // 0x80022ABC: nop

;}
RECOMP_FUNC void Math_FAtanContFracF(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80081D68: lui         $at, 0x3F80
    ctx->r1 = S32(0X3F80 << 16);
    // 0x80081D6C: mtc1        $at, $f16
    ctx->f16.u32l = ctx->r1;
    // 0x80081D70: mtc1        $zero, $f2
    ctx->f2.u32l = 0;
    // 0x80081D74: addiu       $v0, $zero, 0x1
    ctx->r2 = ADD32(0, 0X1);
    // 0x80081D78: c.lt.s      $f16, $f12
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 12);
    c1cs = ctx->f16.fl < ctx->f12.fl;
    // 0x80081D7C: lui         $at, 0xBF80
    ctx->r1 = S32(0XBF80 << 16);
    // 0x80081D80: bc1fl       L_80081D94
    if (!c1cs) {
        // 0x80081D84: mtc1        $at, $f4
        ctx->f4.u32l = ctx->r1;
            goto L_80081D94;
    }
    goto skip_0;
    // 0x80081D84: mtc1        $at, $f4
    ctx->f4.u32l = ctx->r1;
    skip_0:
    // 0x80081D88: b           L_80081DB4
    // 0x80081D8C: div.s       $f12, $f16, $f12
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f12.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f12.fl);
        goto L_80081DB4;
    // 0x80081D8C: div.s       $f12, $f16, $f12
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f12.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f12.fl);
    // 0x80081D90: mtc1        $at, $f4
    ctx->f4.u32l = ctx->r1;
L_80081D94:
    // 0x80081D94: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
    // 0x80081D98: c.lt.s      $f12, $f4
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 4);
    c1cs = ctx->f12.fl < ctx->f4.fl;
    // 0x80081D9C: nop

    // 0x80081DA0: bc1f        L_80081DB4
    if (!c1cs) {
        // 0x80081DA4: nop
    
            goto L_80081DB4;
    }
    // 0x80081DA4: nop

    // 0x80081DA8: addiu       $v0, $zero, -0x1
    ctx->r2 = ADD32(0, -0X1);
    // 0x80081DAC: b           L_80081DB4
    // 0x80081DB0: div.s       $f12, $f16, $f12
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f12.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f12.fl);
        goto L_80081DB4;
    // 0x80081DB0: div.s       $f12, $f16, $f12
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f12.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f12.fl);
L_80081DB4:
    // 0x80081DB4: lui         $at, 0x41C0
    ctx->r1 = S32(0X41C0 << 16);
    // 0x80081DB8: mul.s       $f0, $f12, $f12
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f12.fl); 
    ctx->f0.fl = MUL_S(ctx->f12.fl, ctx->f12.fl);
    // 0x80081DBC: mtc1        $at, $f14
    ctx->f14.u32l = ctx->r1;
    // 0x80081DC0: addiu       $v1, $zero, 0x18
    ctx->r3 = ADD32(0, 0X18);
L_80081DC4:
    // 0x80081DC4: mul.s       $f18, $f14, $f14
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f18.fl = MUL_S(ctx->f14.fl, ctx->f14.fl);
    // 0x80081DC8: add.s       $f10, $f14, $f14
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f10.fl = ctx->f14.fl + ctx->f14.fl;
    // 0x80081DCC: addiu       $v1, $v1, -0x4
    ctx->r3 = ADD32(ctx->r3, -0X4);
    // 0x80081DD0: add.s       $f10, $f10, $f16
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f10.fl = ctx->f10.fl + ctx->f16.fl;
    // 0x80081DD4: mul.s       $f18, $f18, $f0
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f18.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f18.fl = MUL_S(ctx->f18.fl, ctx->f0.fl);
    // 0x80081DD8: sub.s       $f14, $f14, $f16
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f14.fl = ctx->f14.fl - ctx->f16.fl;
    // 0x80081DDC: mul.s       $f8, $f14, $f14
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f8.fl = MUL_S(ctx->f14.fl, ctx->f14.fl);
    // 0x80081DE0: add.s       $f10, $f10, $f2
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f10.fl = ctx->f10.fl + ctx->f2.fl;
    // 0x80081DE4: add.s       $f6, $f14, $f14
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f6.fl = ctx->f14.fl + ctx->f14.fl;
    // 0x80081DE8: mul.s       $f8, $f8, $f0
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f8.fl = MUL_S(ctx->f8.fl, ctx->f0.fl);
    // 0x80081DEC: div.s       $f2, $f18, $f10
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f18.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f2.fl = DIV_S(ctx->f18.fl, ctx->f10.fl);
    // 0x80081DF0: add.s       $f6, $f6, $f16
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f6.fl = ctx->f6.fl + ctx->f16.fl;
    // 0x80081DF4: sub.s       $f14, $f14, $f16
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f14.fl = ctx->f14.fl - ctx->f16.fl;
    // 0x80081DF8: mul.s       $f10, $f14, $f14
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f10.fl = MUL_S(ctx->f14.fl, ctx->f14.fl);
    // 0x80081DFC: add.s       $f6, $f6, $f2
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f6.fl = ctx->f6.fl + ctx->f2.fl;
    // 0x80081E00: add.s       $f18, $f14, $f14
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f18.fl = ctx->f14.fl + ctx->f14.fl;
    // 0x80081E04: mul.s       $f10, $f10, $f0
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f10.fl = MUL_S(ctx->f10.fl, ctx->f0.fl);
    // 0x80081E08: div.s       $f2, $f8, $f6
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 6);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f6.fl); 
    ctx->f2.fl = DIV_S(ctx->f8.fl, ctx->f6.fl);
    // 0x80081E0C: add.s       $f18, $f18, $f16
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f18.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f18.fl = ctx->f18.fl + ctx->f16.fl;
    // 0x80081E10: sub.s       $f14, $f14, $f16
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f14.fl = ctx->f14.fl - ctx->f16.fl;
    // 0x80081E14: mul.s       $f6, $f14, $f14
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f6.fl = MUL_S(ctx->f14.fl, ctx->f14.fl);
    // 0x80081E18: add.s       $f18, $f18, $f2
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f18.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f18.fl = ctx->f18.fl + ctx->f2.fl;
    // 0x80081E1C: add.s       $f8, $f14, $f14
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f8.fl = ctx->f14.fl + ctx->f14.fl;
    // 0x80081E20: mul.s       $f6, $f6, $f0
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f6.fl = MUL_S(ctx->f6.fl, ctx->f0.fl);
    // 0x80081E24: div.s       $f2, $f10, $f18
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f2.fl = DIV_S(ctx->f10.fl, ctx->f18.fl);
    // 0x80081E28: add.s       $f8, $f8, $f16
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f8.fl = ctx->f8.fl + ctx->f16.fl;
    // 0x80081E2C: sub.s       $f14, $f14, $f16
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f14.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f14.fl = ctx->f14.fl - ctx->f16.fl;
    // 0x80081E30: add.s       $f8, $f8, $f2
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f8.fl = ctx->f8.fl + ctx->f2.fl;
    // 0x80081E34: bne         $v1, $zero, L_80081DC4
    if (ctx->r3 != 0) {
        // 0x80081E38: div.s       $f2, $f6, $f8
        CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f8.fl); 
    ctx->f2.fl = DIV_S(ctx->f6.fl, ctx->f8.fl);
            goto L_80081DC4;
    }
    // 0x80081E38: div.s       $f2, $f6, $f8
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f8.fl); 
    ctx->f2.fl = DIV_S(ctx->f6.fl, ctx->f8.fl);
    // 0x80081E3C: blez        $v0, L_80081E5C
    if (SIGNED(ctx->r2) <= 0) {
        // 0x80081E40: nop
    
            goto L_80081E5C;
    }
    // 0x80081E40: nop

    // 0x80081E44: add.s       $f10, $f16, $f2
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f10.fl = ctx->f16.fl + ctx->f2.fl;
    // 0x80081E48: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081E4C: lwc1        $f6, 0x3180($at)
    ctx->f6.u32l = MEM_W(ctx->r1, 0X3180);
    // 0x80081E50: div.s       $f18, $f12, $f10
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f18.fl = DIV_S(ctx->f12.fl, ctx->f10.fl);
    // 0x80081E54: jr          $ra
    // 0x80081E58: sub.s       $f0, $f6, $f18
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f0.fl = ctx->f6.fl - ctx->f18.fl;
    return;
    // 0x80081E58: sub.s       $f0, $f6, $f18
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f0.fl = ctx->f6.fl - ctx->f18.fl;
L_80081E5C:
    // 0x80081E5C: bgezl       $v0, L_80081E80
    if (SIGNED(ctx->r2) >= 0) {
        // 0x80081E60: add.s       $f6, $f16, $f2
        CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f6.fl = ctx->f16.fl + ctx->f2.fl;
            goto L_80081E80;
    }
    goto skip_1;
    // 0x80081E60: add.s       $f6, $f16, $f2
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f6.fl = ctx->f16.fl + ctx->f2.fl;
    skip_1:
    // 0x80081E64: add.s       $f4, $f16, $f2
    CHECK_FR(ctx, 4);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f4.fl = ctx->f16.fl + ctx->f2.fl;
    // 0x80081E68: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081E6C: lwc1        $f8, 0x3184($at)
    ctx->f8.u32l = MEM_W(ctx->r1, 0X3184);
    // 0x80081E70: div.s       $f10, $f12, $f4
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 4);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f4.fl); 
    ctx->f10.fl = DIV_S(ctx->f12.fl, ctx->f4.fl);
    // 0x80081E74: jr          $ra
    // 0x80081E78: sub.s       $f0, $f8, $f10
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f0.fl = ctx->f8.fl - ctx->f10.fl;
    return;
    // 0x80081E78: sub.s       $f0, $f8, $f10
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f0.fl = ctx->f8.fl - ctx->f10.fl;
    // 0x80081E7C: add.s       $f6, $f16, $f2
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 2);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f2.fl); 
    ctx->f6.fl = ctx->f16.fl + ctx->f2.fl;
L_80081E80:
    // 0x80081E80: div.s       $f0, $f12, $f6
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 6);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f6.fl); 
    ctx->f0.fl = DIV_S(ctx->f12.fl, ctx->f6.fl);
    // 0x80081E84: jr          $ra
    // 0x80081E88: nop

    return;
    // 0x80081E88: nop

;}
RECOMP_FUNC void fatan2(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80081E8C: mtc1        $zero, $f2
    ctx->f2.u32l = 0;
    // 0x80081E90: addiu       $sp, $sp, -0x20
    ctx->r29 = ADD32(ctx->r29, -0X20);
    // 0x80081E94: sdc1        $f20, 0x10($sp)
    CHECK_FR(ctx, 20);
    SD(ctx->f20.u64, 0X10, ctx->r29);
    // 0x80081E98: c.eq.s      $f12, $f2
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f12.fl == ctx->f2.fl;
    // 0x80081E9C: mov.s       $f20, $f12
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 12);
    ctx->f20.fl = ctx->f12.fl;
    // 0x80081EA0: sw          $ra, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r31;
    // 0x80081EA4: bc1fl       L_80081EC8
    if (!c1cs) {
        // 0x80081EA8: c.eq.s      $f14, $f2
        CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
            goto L_80081EC8;
    }
    goto skip_0;
    // 0x80081EA8: c.eq.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
    skip_0:
    // 0x80081EAC: c.eq.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
    // 0x80081EB0: nop

    // 0x80081EB4: bc1fl       L_80081EC8
    if (!c1cs) {
        // 0x80081EB8: c.eq.s      $f14, $f2
        CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
            goto L_80081EC8;
    }
    goto skip_1;
    // 0x80081EB8: c.eq.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
    skip_1:
    // 0x80081EBC: b           L_80081F6C
    // 0x80081EC0: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
        goto L_80081F6C;
    // 0x80081EC0: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
    // 0x80081EC4: c.eq.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl == ctx->f2.fl;
L_80081EC8:
    // 0x80081EC8: nop

    // 0x80081ECC: bc1fl       L_80081EFC
    if (!c1cs) {
        // 0x80081ED0: c.lt.s      $f14, $f2
        CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl < ctx->f2.fl;
            goto L_80081EFC;
    }
    goto skip_2;
    // 0x80081ED0: c.lt.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl < ctx->f2.fl;
    skip_2:
    // 0x80081ED4: c.lt.s      $f20, $f2
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f20.fl < ctx->f2.fl;
    // 0x80081ED8: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081EDC: bc1f        L_80081EF0
    if (!c1cs) {
        // 0x80081EE0: nop
    
            goto L_80081EF0;
    }
    // 0x80081EE0: nop

    // 0x80081EE4: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081EE8: b           L_80081F6C
    // 0x80081EEC: lwc1        $f0, 0x3188($at)
    ctx->f0.u32l = MEM_W(ctx->r1, 0X3188);
        goto L_80081F6C;
    // 0x80081EEC: lwc1        $f0, 0x3188($at)
    ctx->f0.u32l = MEM_W(ctx->r1, 0X3188);
L_80081EF0:
    // 0x80081EF0: b           L_80081F6C
    // 0x80081EF4: lwc1        $f0, 0x318C($at)
    ctx->f0.u32l = MEM_W(ctx->r1, 0X318C);
        goto L_80081F6C;
    // 0x80081EF4: lwc1        $f0, 0x318C($at)
    ctx->f0.u32l = MEM_W(ctx->r1, 0X318C);
    // 0x80081EF8: c.lt.s      $f14, $f2
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f14.fl < ctx->f2.fl;
L_80081EFC:
    // 0x80081EFC: nop

    // 0x80081F00: bc1f        L_80081F64
    if (!c1cs) {
        // 0x80081F04: nop
    
            goto L_80081F64;
    }
    // 0x80081F04: nop

    // 0x80081F08: c.lt.s      $f20, $f2
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f20.fl < ctx->f2.fl;
    // 0x80081F0C: nop

    // 0x80081F10: bc1fl       L_80081F44
    if (!c1cs) {
        // 0x80081F14: div.s       $f8, $f20, $f14
        CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f8.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
            goto L_80081F44;
    }
    goto skip_3;
    // 0x80081F14: div.s       $f8, $f20, $f14
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f8.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
    skip_3:
    // 0x80081F18: div.s       $f4, $f20, $f14
    CHECK_FR(ctx, 4);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f4.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
    // 0x80081F1C: cvt.d.s     $f0, $f4
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 4);
    NAN_CHECK(ctx->f4.fl); 
    ctx->f0.d = CVT_D_S(ctx->f4.fl);
    // 0x80081F20: abs.d       $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f0.d = fabs(ctx->f0.d);
    // 0x80081F24: jal         0x80081D68
    // 0x80081F28: cvt.s.d     $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f12.fl = CVT_S_D(ctx->f0.d);
    Math_FAtanContFracF(rdram, ctx);
        goto after_0;
    // 0x80081F28: cvt.s.d     $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f12.fl = CVT_S_D(ctx->f0.d);
    after_0:
    // 0x80081F2C: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081F30: lwc1        $f6, 0x3190($at)
    ctx->f6.u32l = MEM_W(ctx->r1, 0X3190);
    // 0x80081F34: sub.s       $f0, $f6, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = ctx->f6.fl - ctx->f0.fl;
    // 0x80081F38: b           L_80081F6C
    // 0x80081F3C: neg.s       $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = -ctx->f0.fl;
        goto L_80081F6C;
    // 0x80081F3C: neg.s       $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = -ctx->f0.fl;
    // 0x80081F40: div.s       $f8, $f20, $f14
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f8.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
L_80081F44:
    // 0x80081F44: cvt.d.s     $f0, $f8
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f8.fl); 
    ctx->f0.d = CVT_D_S(ctx->f8.fl);
    // 0x80081F48: abs.d       $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f0.d = fabs(ctx->f0.d);
    // 0x80081F4C: jal         0x80081D68
    // 0x80081F50: cvt.s.d     $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f12.fl = CVT_S_D(ctx->f0.d);
    Math_FAtanContFracF(rdram, ctx);
        goto after_1;
    // 0x80081F50: cvt.s.d     $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.d); 
    ctx->f12.fl = CVT_S_D(ctx->f0.d);
    after_1:
    // 0x80081F54: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80081F58: lwc1        $f10, 0x3194($at)
    ctx->f10.u32l = MEM_W(ctx->r1, 0X3194);
    // 0x80081F5C: b           L_80081F6C
    // 0x80081F60: sub.s       $f0, $f10, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = ctx->f10.fl - ctx->f0.fl;
        goto L_80081F6C;
    // 0x80081F60: sub.s       $f0, $f10, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = ctx->f10.fl - ctx->f0.fl;
L_80081F64:
    // 0x80081F64: jal         0x80081D68
    // 0x80081F68: div.s       $f12, $f20, $f14
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f12.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
    Math_FAtanContFracF(rdram, ctx);
        goto after_2;
    // 0x80081F68: div.s       $f12, $f20, $f14
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 20);
    CHECK_FR(ctx, 14);
    NAN_CHECK(ctx->f20.fl); NAN_CHECK(ctx->f14.fl); 
    ctx->f12.fl = DIV_S(ctx->f20.fl, ctx->f14.fl);
    after_2:
L_80081F6C:
    // 0x80081F6C: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80081F70: ldc1        $f20, 0x10($sp)
    CHECK_FR(ctx, 20);
    ctx->f20.u64 = LD(ctx->r29, 0X10);
    // 0x80081F74: addiu       $sp, $sp, 0x20
    ctx->r29 = ADD32(ctx->r29, 0X20);
    // 0x80081F78: jr          $ra
    // 0x80081F7C: nop

    return;
    // 0x80081F7C: nop

;}
RECOMP_FUNC void func_80065954(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80065954: mtc1        $zero, $f4
    ctx->f4.u32l = 0;
    // 0x80065958: nop

    // 0x8006595C: c.lt.s      $f12, $f4
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 4);
    c1cs = ctx->f12.fl < ctx->f4.fl;
    // 0x80065960: nop

    // 0x80065964: bc1fl       L_8006597C
    if (!c1cs) {
        // 0x80065968: mov.s       $f2, $f12
        CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 12);
    ctx->f2.fl = ctx->f12.fl;
            goto L_8006597C;
    }
    goto skip_0;
    // 0x80065968: mov.s       $f2, $f12
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 12);
    ctx->f2.fl = ctx->f12.fl;
    skip_0:
    // 0x8006596C: neg.s       $f2, $f12
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 12);
    NAN_CHECK(ctx->f12.fl); 
    ctx->f2.fl = -ctx->f12.fl;
    // 0x80065970: jr          $ra
    // 0x80065974: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
    return;
    // 0x80065974: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
    // 0x80065978: mov.s       $f2, $f12
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 12);
    ctx->f2.fl = ctx->f12.fl;
L_8006597C:
    // 0x8006597C: jr          $ra
    // 0x80065980: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
    return;
    // 0x80065980: mov.s       $f0, $f2
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 2);
    ctx->f0.fl = ctx->f2.fl;
;}
RECOMP_FUNC void sins(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x800828F0: andi        $t6, $a0, 0xFFFF
    ctx->r14 = ctx->r4 & 0XFFFF;
    // 0x800828F4: srl         $t7, $t6, 4
    ctx->r15 = S32(U32(ctx->r14) >> 4);
    // 0x800828F8: andi        $t8, $t7, 0xFFFF
    ctx->r24 = ctx->r15 & 0XFFFF;
    // 0x800828FC: andi        $t9, $t8, 0x400
    ctx->r25 = ctx->r24 & 0X400;
    // 0x80082900: sw          $a0, 0x0($sp)
    MEM_W(0X0, ctx->r29) = ctx->r4;
    // 0x80082904: beq         $t9, $zero, L_80082928
    if (ctx->r25 == 0) {
        // 0x80082908: or          $v0, $t8, $zero
        ctx->r2 = ctx->r24 | 0;
            goto L_80082928;
    }
    // 0x80082908: or          $v0, $t8, $zero
    ctx->r2 = ctx->r24 | 0;
    // 0x8008290C: andi        $t0, $t8, 0x3FF
    ctx->r8 = ctx->r24 & 0X3FF;
    // 0x80082910: sll         $t1, $t0, 1
    ctx->r9 = S32(ctx->r8 << 1);
    // 0x80082914: negu        $t2, $t1
    ctx->r10 = SUB32(0, ctx->r9);
    // 0x80082918: lui         $v1, 0x800B
    ctx->r3 = S32(0X800B << 16);
    // 0x8008291C: addu        $v1, $v1, $t2
    ctx->r3 = ADD32(ctx->r3, ctx->r10);
    // 0x80082920: b           L_8008293C
    // 0x80082924: lh          $v1, -0x5452($v1)
    ctx->r3 = MEM_H(ctx->r3, -0X5452);
        goto L_8008293C;
    // 0x80082924: lh          $v1, -0x5452($v1)
    ctx->r3 = MEM_H(ctx->r3, -0X5452);
L_80082928:
    // 0x80082928: andi        $t3, $v0, 0x3FF
    ctx->r11 = ctx->r2 & 0X3FF;
    // 0x8008292C: sll         $t4, $t3, 1
    ctx->r12 = S32(ctx->r11 << 1);
    // 0x80082930: lui         $v1, 0x800B
    ctx->r3 = S32(0X800B << 16);
    // 0x80082934: addu        $v1, $v1, $t4
    ctx->r3 = ADD32(ctx->r3, ctx->r12);
    // 0x80082938: lh          $v1, -0x5C50($v1)
    ctx->r3 = MEM_H(ctx->r3, -0X5C50);
L_8008293C:
    // 0x8008293C: andi        $t5, $v0, 0x800
    ctx->r13 = ctx->r2 & 0X800;
    // 0x80082940: beq         $t5, $zero, L_80082958
    if (ctx->r13 == 0) {
        // 0x80082944: or          $v0, $v1, $zero
        ctx->r2 = ctx->r3 | 0;
            goto L_80082958;
    }
    // 0x80082944: or          $v0, $v1, $zero
    ctx->r2 = ctx->r3 | 0;
    // 0x80082948: negu        $v0, $v1
    ctx->r2 = SUB32(0, ctx->r3);
    // 0x8008294C: sll         $t6, $v0, 16
    ctx->r14 = S32(ctx->r2 << 16);
    // 0x80082950: jr          $ra
    // 0x80082954: sra         $v0, $t6, 16
    ctx->r2 = S32(SIGNED(ctx->r14) >> 16);
    return;
    // 0x80082954: sra         $v0, $t6, 16
    ctx->r2 = S32(SIGNED(ctx->r14) >> 16);
L_80082958:
    // 0x80082958: jr          $ra
    // 0x8008295C: nop

    return;
    // 0x8008295C: nop

;}
RECOMP_FUNC void coss(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x8008D3C0: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x8008D3C4: sw          $a0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r4;
    // 0x8008D3C8: or          $a1, $a0, $zero
    ctx->r5 = ctx->r4 | 0;
    // 0x8008D3CC: addiu       $a0, $a1, 0x4000
    ctx->r4 = ADD32(ctx->r5, 0X4000);
    // 0x8008D3D0: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x8008D3D4: andi        $t6, $a0, 0xFFFF
    ctx->r14 = ctx->r4 & 0XFFFF;
    // 0x8008D3D8: jal         0x800828F0
    // 0x8008D3DC: or          $a0, $t6, $zero
    ctx->r4 = ctx->r14 | 0;
    sins(rdram, ctx);
        goto after_0;
    // 0x8008D3DC: or          $a0, $t6, $zero
    ctx->r4 = ctx->r14 | 0;
    after_0:
    // 0x8008D3E0: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x8008D3E4: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x8008D3E8: jr          $ra
    // 0x8008D3EC: nop

    return;
    // 0x8008D3EC: nop

;}
RECOMP_FUNC void func_80066DE4(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80066DE4: mtc1        $zero, $f2
    ctx->f2.u32l = 0;
    // 0x80066DE8: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80066DEC: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80066DF0: c.le.s      $f12, $f2
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f12.fl <= ctx->f2.fl;
    // 0x80066DF4: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
    // 0x80066DF8: bc1fl       L_80066E20
    if (!c1cs) {
        // 0x80066DFC: lui         $at, 0x43B4
        ctx->r1 = S32(0X43B4 << 16);
            goto L_80066E20;
    }
    goto skip_0;
    // 0x80066DFC: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
    skip_0:
    // 0x80066E00: mtc1        $at, $f0
    ctx->f0.u32l = ctx->r1;
    // 0x80066E04: nop

    // 0x80066E08: add.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
L_80066E0C:
    // 0x80066E0C: c.le.s      $f12, $f2
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f12.fl <= ctx->f2.fl;
    // 0x80066E10: nop

    // 0x80066E14: bc1tl       L_80066E0C
    if (c1cs) {
        // 0x80066E18: add.s       $f12, $f12, $f0
        CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
            goto L_80066E0C;
    }
    goto skip_1;
    // 0x80066E18: add.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
    skip_1:
    // 0x80066E1C: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
L_80066E20:
    // 0x80066E20: mtc1        $at, $f0
    ctx->f0.u32l = ctx->r1;
    // 0x80066E24: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80066E28: c.lt.s      $f0, $f12
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    c1cs = ctx->f0.fl < ctx->f12.fl;
    // 0x80066E2C: nop

    // 0x80066E30: bc1f        L_80066E4C
    if (!c1cs) {
        // 0x80066E34: nop
    
            goto L_80066E4C;
    }
    // 0x80066E34: nop

    // 0x80066E38: sub.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
L_80066E3C:
    // 0x80066E3C: c.lt.s      $f0, $f12
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    c1cs = ctx->f0.fl < ctx->f12.fl;
    // 0x80066E40: nop

    // 0x80066E44: bc1tl       L_80066E3C
    if (c1cs) {
        // 0x80066E48: sub.s       $f12, $f12, $f0
        CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
            goto L_80066E3C;
    }
    goto skip_2;
    // 0x80066E48: sub.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
    skip_2:
L_80066E4C:
    // 0x80066E4C: lwc1        $f4, 0xF04($at)
    ctx->f4.u32l = MEM_W(ctx->r1, 0XF04);
    // 0x80066E50: mul.s       $f6, $f12, $f4
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 4);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f4.fl); 
    ctx->f6.fl = MUL_S(ctx->f12.fl, ctx->f4.fl);
    // 0x80066E54: trunc.w.s   $f8, $f6
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 6);
    ctx->f8.u32l = TRUNC_W_S(ctx->f6.fl);
    // 0x80066E58: mfc1        $a0, $f8
    ctx->r4 = (int32_t)ctx->f8.u32l;
    // 0x80066E5C: jal         0x800828F0
    // 0x80066E60: andi        $a0, $a0, 0xFFFF
    ctx->r4 = ctx->r4 & 0XFFFF;
    sins(rdram, ctx);
        goto after_0;
    // 0x80066E60: andi        $a0, $a0, 0xFFFF
    ctx->r4 = ctx->r4 & 0XFFFF;
    after_0:
    // 0x80066E64: mtc1        $v0, $f10
    ctx->f10.u32l = ctx->r2;
    // 0x80066E68: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80066E6C: lwc1        $f18, 0xF08($at)
    ctx->f18.u32l = MEM_W(ctx->r1, 0XF08);
    // 0x80066E70: cvt.s.w     $f16, $f10
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 10);
    ctx->f16.fl = CVT_S_W(ctx->f10.u32l);
    // 0x80066E74: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80066E78: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80066E7C: mul.s       $f0, $f16, $f18
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f0.fl = MUL_S(ctx->f16.fl, ctx->f18.fl);
    // 0x80066E80: jr          $ra
    // 0x80066E84: nop

    return;
    // 0x80066E84: nop

;}
RECOMP_FUNC void func_80066E88(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80066E88: mtc1        $zero, $f2
    ctx->f2.u32l = 0;
    // 0x80066E8C: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80066E90: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80066E94: c.le.s      $f12, $f2
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f12.fl <= ctx->f2.fl;
    // 0x80066E98: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
    // 0x80066E9C: bc1fl       L_80066EC4
    if (!c1cs) {
        // 0x80066EA0: lui         $at, 0x43B4
        ctx->r1 = S32(0X43B4 << 16);
            goto L_80066EC4;
    }
    goto skip_0;
    // 0x80066EA0: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
    skip_0:
    // 0x80066EA4: mtc1        $at, $f0
    ctx->f0.u32l = ctx->r1;
    // 0x80066EA8: nop

    // 0x80066EAC: add.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
L_80066EB0:
    // 0x80066EB0: c.le.s      $f12, $f2
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f12.fl <= ctx->f2.fl;
    // 0x80066EB4: nop

    // 0x80066EB8: bc1tl       L_80066EB0
    if (c1cs) {
        // 0x80066EBC: add.s       $f12, $f12, $f0
        CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
            goto L_80066EB0;
    }
    goto skip_1;
    // 0x80066EBC: add.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl + ctx->f0.fl;
    skip_1:
    // 0x80066EC0: lui         $at, 0x43B4
    ctx->r1 = S32(0X43B4 << 16);
L_80066EC4:
    // 0x80066EC4: mtc1        $at, $f0
    ctx->f0.u32l = ctx->r1;
    // 0x80066EC8: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80066ECC: c.lt.s      $f0, $f12
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    c1cs = ctx->f0.fl < ctx->f12.fl;
    // 0x80066ED0: nop

    // 0x80066ED4: bc1f        L_80066EF0
    if (!c1cs) {
        // 0x80066ED8: nop
    
            goto L_80066EF0;
    }
    // 0x80066ED8: nop

    // 0x80066EDC: sub.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
L_80066EE0:
    // 0x80066EE0: c.lt.s      $f0, $f12
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 12);
    c1cs = ctx->f0.fl < ctx->f12.fl;
    // 0x80066EE4: nop

    // 0x80066EE8: bc1tl       L_80066EE0
    if (c1cs) {
        // 0x80066EEC: sub.s       $f12, $f12, $f0
        CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
            goto L_80066EE0;
    }
    goto skip_2;
    // 0x80066EEC: sub.s       $f12, $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f0.fl); 
    ctx->f12.fl = ctx->f12.fl - ctx->f0.fl;
    skip_2:
L_80066EF0:
    // 0x80066EF0: lwc1        $f4, 0xF0C($at)
    ctx->f4.u32l = MEM_W(ctx->r1, 0XF0C);
    // 0x80066EF4: mul.s       $f6, $f12, $f4
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 4);
    NAN_CHECK(ctx->f12.fl); NAN_CHECK(ctx->f4.fl); 
    ctx->f6.fl = MUL_S(ctx->f12.fl, ctx->f4.fl);
    // 0x80066EF8: trunc.w.s   $f8, $f6
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 6);
    ctx->f8.u32l = TRUNC_W_S(ctx->f6.fl);
    // 0x80066EFC: mfc1        $a0, $f8
    ctx->r4 = (int32_t)ctx->f8.u32l;
    // 0x80066F00: jal         0x8008D3C0
    // 0x80066F04: andi        $a0, $a0, 0xFFFF
    ctx->r4 = ctx->r4 & 0XFFFF;
    coss(rdram, ctx);
        goto after_0;
    // 0x80066F04: andi        $a0, $a0, 0xFFFF
    ctx->r4 = ctx->r4 & 0XFFFF;
    after_0:
    // 0x80066F08: mtc1        $v0, $f10
    ctx->f10.u32l = ctx->r2;
    // 0x80066F0C: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80066F10: lwc1        $f18, 0xF10($at)
    ctx->f18.u32l = MEM_W(ctx->r1, 0XF10);
    // 0x80066F14: cvt.s.w     $f16, $f10
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 10);
    ctx->f16.fl = CVT_S_W(ctx->f10.u32l);
    // 0x80066F18: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80066F1C: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80066F20: mul.s       $f0, $f16, $f18
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f0.fl = MUL_S(ctx->f16.fl, ctx->f18.fl);
    // 0x80066F24: jr          $ra
    // 0x80066F28: nop

    return;
    // 0x80066F28: nop

;}
RECOMP_FUNC void func_80022B20(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80022B20: addiu       $sp, $sp, -0x40
    ctx->r29 = ADD32(ctx->r29, -0X40);
    // 0x80022B24: sw          $s0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r16;
    // 0x80022B28: lui         $s0, 0x800F
    ctx->r16 = S32(0X800F << 16);
    // 0x80022B2C: addiu       $s0, $s0, 0x7EA0
    ctx->r16 = ADD32(ctx->r16, 0X7EA0);
    // 0x80022B30: lw          $t6, 0x8($s0)
    ctx->r14 = MEM_W(ctx->r16, 0X8);
    // 0x80022B34: sw          $ra, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r31;
    // 0x80022B38: lb          $a0, 0x2($t6)
    ctx->r4 = MEM_B(ctx->r14, 0X2);
    // 0x80022B3C: jal         0x80022A58
    // 0x80022B40: or          $a1, $a0, $zero
    ctx->r5 = ctx->r4 | 0;
    func_80022A58(rdram, ctx);
        goto after_0;
    // 0x80022B40: or          $a1, $a0, $zero
    ctx->r5 = ctx->r4 | 0;
    after_0:
    // 0x80022B44: lw          $t7, 0x8($s0)
    ctx->r15 = MEM_W(ctx->r16, 0X8);
    // 0x80022B48: lb          $a0, 0x3($t7)
    ctx->r4 = MEM_B(ctx->r15, 0X3);
    // 0x80022B4C: sh          $v0, 0x3E($sp)
    MEM_H(0X3E, ctx->r29) = ctx->r2;
    // 0x80022B50: jal         0x80022A58
    // 0x80022B54: or          $a1, $a0, $zero
    ctx->r5 = ctx->r4 | 0;
    func_80022A58(rdram, ctx);
        goto after_1;
    // 0x80022B54: or          $a1, $a0, $zero
    ctx->r5 = ctx->r4 | 0;
    after_1:
    // 0x80022B58: lh          $v1, 0x3E($sp)
    ctx->r3 = MEM_H(ctx->r29, 0X3E);
    // 0x80022B5C: mtc1        $v0, $f6
    ctx->f6.u32l = ctx->r2;
    // 0x80022B60: multu       $v1, $v1
    result = U64(U32(ctx->r3)) * U64(U32(ctx->r3)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022B64: mtc1        $v1, $f8
    ctx->f8.u32l = ctx->r3;
    // 0x80022B68: cvt.s.w     $f12, $f6
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 6);
    ctx->f12.fl = CVT_S_W(ctx->f6.u32l);
    // 0x80022B6C: cvt.s.w     $f14, $f8
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 8);
    ctx->f14.fl = CVT_S_W(ctx->f8.u32l);
    // 0x80022B70: mflo        $t8
    ctx->r24 = lo;
    // 0x80022B74: nop

    // 0x80022B78: nop

    // 0x80022B7C: multu       $v0, $v0
    result = U64(U32(ctx->r2)) * U64(U32(ctx->r2)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022B80: mflo        $t9
    ctx->r25 = lo;
    // 0x80022B84: addu        $t0, $t8, $t9
    ctx->r8 = ADD32(ctx->r24, ctx->r25);
    // 0x80022B88: mtc1        $t0, $f4
    ctx->f4.u32l = ctx->r8;
    // 0x80022B8C: nop

    // 0x80022B90: cvt.s.w     $f0, $f4
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 4);
    ctx->f0.fl = CVT_S_W(ctx->f4.u32l);
    // 0x80022B94: sqrt.s      $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = sqrtf(ctx->f0.fl);
    // 0x80022B98: jal         0x80081E8C
    // 0x80022B9C: swc1        $f0, 0x38($sp)
    MEM_W(0X38, ctx->r29) = ctx->f0.u32l;
    fatan2(rdram, ctx);
        goto after_2;
    // 0x80022B9C: swc1        $f0, 0x38($sp)
    MEM_W(0X38, ctx->r29) = ctx->f0.u32l;
    after_2:
    // 0x80022BA0: lui         $at, 0x4334
    ctx->r1 = S32(0X4334 << 16);
    // 0x80022BA4: mtc1        $at, $f10
    ctx->f10.u32l = ctx->r1;
    // 0x80022BA8: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80022BAC: lwc1        $f18, -0x368($at)
    ctx->f18.u32l = MEM_W(ctx->r1, -0X368);
    // 0x80022BB0: mul.s       $f16, $f0, $f10
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f0.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f16.fl = MUL_S(ctx->f0.fl, ctx->f10.fl);
    // 0x80022BB4: jal         0x80065954
    // 0x80022BB8: div.s       $f12, $f16, $f18
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f18.fl);
    func_80065954(rdram, ctx);
        goto after_3;
    // 0x80022BB8: div.s       $f12, $f16, $f18
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f12.fl = DIV_S(ctx->f16.fl, ctx->f18.fl);
    after_3:
    // 0x80022BBC: swc1        $f0, 0x34($sp)
    MEM_W(0X34, ctx->r29) = ctx->f0.u32l;
    // 0x80022BC0: jal         0x80066E88
    // 0x80022BC4: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    func_80066E88(rdram, ctx);
        goto after_4;
    // 0x80022BC4: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    after_4:
    // 0x80022BC8: jal         0x80065954
    // 0x80022BCC: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    func_80065954(rdram, ctx);
        goto after_5;
    // 0x80022BCC: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    after_5:
    // 0x80022BD0: swc1        $f0, 0x24($sp)
    MEM_W(0X24, ctx->r29) = ctx->f0.u32l;
    // 0x80022BD4: jal         0x80066DE4
    // 0x80022BD8: lwc1        $f12, 0x34($sp)
    ctx->f12.u32l = MEM_W(ctx->r29, 0X34);
    func_80066DE4(rdram, ctx);
        goto after_6;
    // 0x80022BD8: lwc1        $f12, 0x34($sp)
    ctx->f12.u32l = MEM_W(ctx->r29, 0X34);
    after_6:
    // 0x80022BDC: jal         0x80065954
    // 0x80022BE0: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    func_80065954(rdram, ctx);
        goto after_7;
    // 0x80022BE0: mov.s       $f12, $f0
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 0);
    ctx->f12.fl = ctx->f0.fl;
    after_7:
    // 0x80022BE4: lui         $at, 0x4254
    ctx->r1 = S32(0X4254 << 16);
    // 0x80022BE8: mtc1        $at, $f6
    ctx->f6.u32l = ctx->r1;
    // 0x80022BEC: lwc1        $f8, 0x24($sp)
    ctx->f8.u32l = MEM_W(ctx->r29, 0X24);
    // 0x80022BF0: lui         $at, 0x41A0
    ctx->r1 = S32(0X41A0 << 16);
    // 0x80022BF4: mtc1        $at, $f16
    ctx->f16.u32l = ctx->r1;
    // 0x80022BF8: div.s       $f10, $f6, $f8
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f8.fl); 
    ctx->f10.fl = DIV_S(ctx->f6.fl, ctx->f8.fl);
    // 0x80022BFC: lwc1        $f4, 0x38($sp)
    ctx->f4.u32l = MEM_W(ctx->r29, 0X38);
    // 0x80022C00: lui         $at, 0x4248
    ctx->r1 = S32(0X4248 << 16);
    // 0x80022C04: lwc1        $f2, 0x34($sp)
    ctx->f2.u32l = MEM_W(ctx->r29, 0X34);
    // 0x80022C08: mul.s       $f18, $f0, $f16
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f0.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f18.fl = MUL_S(ctx->f0.fl, ctx->f16.fl);
    // 0x80022C0C: sub.s       $f6, $f10, $f18
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f10.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f6.fl = ctx->f10.fl - ctx->f18.fl;
    // 0x80022C10: c.le.s      $f6, $f4
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 4);
    c1cs = ctx->f6.fl <= ctx->f4.fl;
    // 0x80022C14: nop

    // 0x80022C18: bc1fl       L_80022C74
    if (!c1cs) {
        // 0x80022C1C: lw          $t1, 0x8($s0)
        ctx->r9 = MEM_W(ctx->r16, 0X8);
            goto L_80022C74;
    }
    goto skip_0;
    // 0x80022C1C: lw          $t1, 0x8($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X8);
    skip_0:
    // 0x80022C20: mtc1        $at, $f0
    ctx->f0.u32l = ctx->r1;
    // 0x80022C24: lui         $at, 0x4302
    ctx->r1 = S32(0X4302 << 16);
    // 0x80022C28: c.le.s      $f2, $f0
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 0);
    c1cs = ctx->f2.fl <= ctx->f0.fl;
    // 0x80022C2C: nop

    // 0x80022C30: bc1tl       L_80022C54
    if (c1cs) {
        // 0x80022C34: c.le.s      $f2, $f0
        CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 0);
    c1cs = ctx->f2.fl <= ctx->f0.fl;
            goto L_80022C54;
    }
    goto skip_1;
    // 0x80022C34: c.le.s      $f2, $f0
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 0);
    c1cs = ctx->f2.fl <= ctx->f0.fl;
    skip_1:
    // 0x80022C38: mtc1        $at, $f8
    ctx->f8.u32l = ctx->r1;
    // 0x80022C3C: nop

    // 0x80022C40: c.le.s      $f8, $f2
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 2);
    c1cs = ctx->f8.fl <= ctx->f2.fl;
    // 0x80022C44: nop

    // 0x80022C48: bc1fl       L_80022C74
    if (!c1cs) {
        // 0x80022C4C: lw          $t1, 0x8($s0)
        ctx->r9 = MEM_W(ctx->r16, 0X8);
            goto L_80022C74;
    }
    goto skip_2;
    // 0x80022C4C: lw          $t1, 0x8($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X8);
    skip_2:
    // 0x80022C50: c.le.s      $f2, $f0
    CHECK_FR(ctx, 2);
    CHECK_FR(ctx, 0);
    c1cs = ctx->f2.fl <= ctx->f0.fl;
L_80022C54:
    // 0x80022C54: addiu       $v0, $zero, -0x36
    ctx->r2 = ADD32(0, -0X36);
    // 0x80022C58: bc1f        L_80022C68
    if (!c1cs) {
        // 0x80022C5C: nop
    
            goto L_80022C68;
    }
    // 0x80022C5C: nop

    // 0x80022C60: b           L_80022C68
    // 0x80022C64: addiu       $v0, $zero, 0x36
    ctx->r2 = ADD32(0, 0X36);
        goto L_80022C68;
    // 0x80022C64: addiu       $v0, $zero, 0x36
    ctx->r2 = ADD32(0, 0X36);
L_80022C68:
    // 0x80022C68: b           L_80022C84
    // 0x80022C6C: sh          $v0, 0xADE($s0)
    MEM_H(0XADE, ctx->r16) = ctx->r2;
        goto L_80022C84;
    // 0x80022C6C: sh          $v0, 0xADE($s0)
    MEM_H(0XADE, ctx->r16) = ctx->r2;
    // 0x80022C70: lw          $t1, 0x8($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X8);
L_80022C74:
    // 0x80022C74: lh          $a1, 0xADE($s0)
    ctx->r5 = MEM_H(ctx->r16, 0XADE);
    // 0x80022C78: jal         0x80022A58
    // 0x80022C7C: lb          $a0, 0x2($t1)
    ctx->r4 = MEM_B(ctx->r9, 0X2);
    func_80022A58(rdram, ctx);
        goto after_8;
    // 0x80022C7C: lb          $a0, 0x2($t1)
    ctx->r4 = MEM_B(ctx->r9, 0X2);
    after_8:
    // 0x80022C80: sh          $v0, 0xADE($s0)
    MEM_H(0XADE, ctx->r16) = ctx->r2;
L_80022C84:
    // 0x80022C84: lw          $t2, 0x8($s0)
    ctx->r10 = MEM_W(ctx->r16, 0X8);
    // 0x80022C88: lh          $a1, 0xAE0($s0)
    ctx->r5 = MEM_H(ctx->r16, 0XAE0);
    // 0x80022C8C: jal         0x80022A58
    // 0x80022C90: lb          $a0, 0x3($t2)
    ctx->r4 = MEM_B(ctx->r10, 0X3);
    func_80022A58(rdram, ctx);
        goto after_9;
    // 0x80022C90: lb          $a0, 0x3($t2)
    ctx->r4 = MEM_B(ctx->r10, 0X3);
    after_9:
    // 0x80022C94: lw          $t3, 0x8($s0)
    ctx->r11 = MEM_W(ctx->r16, 0X8);
    // 0x80022C98: sh          $v0, 0xAE0($s0)
    MEM_H(0XAE0, ctx->r16) = ctx->r2;
    // 0x80022C9C: lh          $a1, 0xAE2($s0)
    ctx->r5 = MEM_H(ctx->r16, 0XAE2);
    // 0x80022CA0: jal         0x80022A58
    // 0x80022CA4: lb          $a0, 0x2($t3)
    ctx->r4 = MEM_B(ctx->r11, 0X2);
    func_80022A58(rdram, ctx);
        goto after_10;
    // 0x80022CA4: lb          $a0, 0x2($t3)
    ctx->r4 = MEM_B(ctx->r11, 0X2);
    after_10:
    // 0x80022CA8: lw          $t4, 0x8($s0)
    ctx->r12 = MEM_W(ctx->r16, 0X8);
    // 0x80022CAC: sh          $v0, 0xAE2($s0)
    MEM_H(0XAE2, ctx->r16) = ctx->r2;
    // 0x80022CB0: lh          $a1, 0xAE4($s0)
    ctx->r5 = MEM_H(ctx->r16, 0XAE4);
    // 0x80022CB4: jal         0x80022A58
    // 0x80022CB8: lb          $a0, 0x3($t4)
    ctx->r4 = MEM_B(ctx->r12, 0X3);
    func_80022A58(rdram, ctx);
        goto after_11;
    // 0x80022CB8: lb          $a0, 0x3($t4)
    ctx->r4 = MEM_B(ctx->r12, 0X3);
    after_11:
    // 0x80022CBC: lh          $v1, 0xADE($s0)
    ctx->r3 = MEM_H(ctx->r16, 0XADE);
    // 0x80022CC0: lh          $a0, 0xAE0($s0)
    ctx->r4 = MEM_H(ctx->r16, 0XAE0);
    // 0x80022CC4: lh          $a1, 0xAE2($s0)
    ctx->r5 = MEM_H(ctx->r16, 0XAE2);
    // 0x80022CC8: multu       $v1, $v1
    result = U64(U32(ctx->r3)) * U64(U32(ctx->r3)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022CCC: sh          $v0, 0xAE4($s0)
    MEM_H(0XAE4, ctx->r16) = ctx->r2;
    // 0x80022CD0: lh          $a2, 0xAE4($s0)
    ctx->r6 = MEM_H(ctx->r16, 0XAE4);
    // 0x80022CD4: mtc1        $v1, $f4
    ctx->f4.u32l = ctx->r3;
    // 0x80022CD8: mtc1        $a0, $f18
    ctx->f18.u32l = ctx->r4;
    // 0x80022CDC: cvt.s.w     $f14, $f4
    CHECK_FR(ctx, 14);
    CHECK_FR(ctx, 4);
    ctx->f14.fl = CVT_S_W(ctx->f4.u32l);
    // 0x80022CE0: mflo        $t5
    ctx->r13 = lo;
    // 0x80022CE4: cvt.s.w     $f12, $f18
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 18);
    ctx->f12.fl = CVT_S_W(ctx->f18.u32l);
    // 0x80022CE8: nop

    // 0x80022CEC: multu       $a0, $a0
    result = U64(U32(ctx->r4)) * U64(U32(ctx->r4)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022CF0: mflo        $t6
    ctx->r14 = lo;
    // 0x80022CF4: addu        $t7, $t5, $t6
    ctx->r15 = ADD32(ctx->r13, ctx->r14);
    // 0x80022CF8: mtc1        $t7, $f16
    ctx->f16.u32l = ctx->r15;
    // 0x80022CFC: multu       $a1, $a1
    result = U64(U32(ctx->r5)) * U64(U32(ctx->r5)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022D00: cvt.s.w     $f0, $f16
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 16);
    ctx->f0.fl = CVT_S_W(ctx->f16.u32l);
    // 0x80022D04: sqrt.s      $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = sqrtf(ctx->f0.fl);
    // 0x80022D08: mflo        $t8
    ctx->r24 = lo;
    // 0x80022D0C: nop

    // 0x80022D10: nop

    // 0x80022D14: multu       $a2, $a2
    result = U64(U32(ctx->r6)) * U64(U32(ctx->r6)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80022D18: mflo        $t9
    ctx->r25 = lo;
    // 0x80022D1C: addu        $t0, $t8, $t9
    ctx->r8 = ADD32(ctx->r24, ctx->r25);
    // 0x80022D20: mtc1        $t0, $f10
    ctx->f10.u32l = ctx->r8;
    // 0x80022D24: swc1        $f0, 0xAE8($s0)
    MEM_W(0XAE8, ctx->r16) = ctx->f0.u32l;
    // 0x80022D28: cvt.s.w     $f0, $f10
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 10);
    ctx->f0.fl = CVT_S_W(ctx->f10.u32l);
    // 0x80022D2C: sqrt.s      $f0, $f0
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 0);
    NAN_CHECK(ctx->f0.fl); 
    ctx->f0.fl = sqrtf(ctx->f0.fl);
    // 0x80022D30: jal         0x80081E8C
    // 0x80022D34: swc1        $f0, 0xAEC($s0)
    MEM_W(0XAEC, ctx->r16) = ctx->f0.u32l;
    fatan2(rdram, ctx);
        goto after_12;
    // 0x80022D34: swc1        $f0, 0xAEC($s0)
    MEM_W(0XAEC, ctx->r16) = ctx->f0.u32l;
    after_12:
    // 0x80022D38: lui         $at, 0x4334
    ctx->r1 = S32(0X4334 << 16);
    // 0x80022D3C: mtc1        $at, $f6
    ctx->f6.u32l = ctx->r1;
    // 0x80022D40: lui         $at, 0x800B
    ctx->r1 = S32(0X800B << 16);
    // 0x80022D44: lwc1        $f16, -0x364($at)
    ctx->f16.u32l = MEM_W(ctx->r1, -0X364);
    // 0x80022D48: mul.s       $f8, $f0, $f6
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 0);
    CHECK_FR(ctx, 6);
    NAN_CHECK(ctx->f0.fl); NAN_CHECK(ctx->f6.fl); 
    ctx->f8.fl = MUL_S(ctx->f0.fl, ctx->f6.fl);
    // 0x80022D4C: div.s       $f10, $f8, $f16
    CHECK_FR(ctx, 10);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f10.fl = DIV_S(ctx->f8.fl, ctx->f16.fl);
    // 0x80022D50: jal         0x80022AC0
    // 0x80022D54: swc1        $f10, 0xAF0($s0)
    MEM_W(0XAF0, ctx->r16) = ctx->f10.u32l;
    func_80022AC0(rdram, ctx);
        goto after_13;
    // 0x80022D54: swc1        $f10, 0xAF0($s0)
    MEM_W(0XAF0, ctx->r16) = ctx->f10.u32l;
    after_13:
    // 0x80022D58: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80022D5C: lw          $s0, 0x18($sp)
    ctx->r16 = MEM_W(ctx->r29, 0X18);
    // 0x80022D60: addiu       $sp, $sp, 0x40
    ctx->r29 = ADD32(ctx->r29, 0X40);
    // 0x80022D64: jr          $ra
    // 0x80022D68: nop

    return;
    // 0x80022D68: nop

;}
}
