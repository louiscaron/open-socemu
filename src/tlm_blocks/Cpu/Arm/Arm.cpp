#include "Cpu/Arm/Arm.h"

#if 0
// using this namespace to simplify streaming
using namespace std;

#if 0
void Arm::disas_thumb_insn(void)
{
    uint32_t val, insn, op, rm, rn, rd, shift, cond;
    int32_t offset;
    int i;
    uint32_t tmp;
    uint32_t tmp2;
    uint32_t addr;

    insn = this->rd_i_s(this->pc);
    this->pc += 2;

    switch (insn >> 12) {
    case 0: case 1:

        rd = insn & 7;
        op = (insn >> 11) & 3;
        if (op == 3) {
            /* add/subtract */
            rn = (insn >> 3) & 7;
            tmp = load_reg(s, rn);
            if (insn & (1 << 10)) {
                /* immediate */
                tmp2 = (insn >> 6) & 7;
            } else {
                /* reg */
                rm = (insn >> 6) & 7;
                tmp2 = load_reg(rm);
            }
            if (insn & (1 << 9)) {
                if (this->condexec_mask)
                    tcg_gen_sub_i32(tmp, tmp, tmp2);
                else
                    gen_helper_sub_cc(tmp, tmp, tmp2);
            } else {
                if (s->condexec_mask)
                    tcg_gen_add_i32(tmp, tmp, tmp2);
                else
                    gen_helper_add_cc(tmp, tmp, tmp2);
            }
            dead_tmp(tmp2);
            store_reg(s, rd, tmp);
        } else {
            /* shift immediate */
            rm = (insn >> 3) & 7;
            shift = (insn >> 6) & 0x1f;
            tmp = load_reg(s, rm);
            gen_arm_shift_im(tmp, op, shift, s->condexec_mask == 0);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            store_reg(s, rd, tmp);
        }
        break;
    case 2: case 3:
        /* arithmetic large immediate */
        op = (insn >> 11) & 3;
        rd = (insn >> 8) & 0x7;
        if (op == 0) { /* mov */
            tmp = new_tmp();
            tcg_gen_movi_i32(tmp, insn & 0xff);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            store_reg(s, rd, tmp);
        } else {
            tmp = load_reg(s, rd);
            tmp2 = new_tmp();
            tcg_gen_movi_i32(tmp2, insn & 0xff);
            switch (op) {
            case 1: /* cmp */
                gen_helper_sub_cc(tmp, tmp, tmp2);
                dead_tmp(tmp);
                dead_tmp(tmp2);
                break;
            case 2: /* add */
                if (s->condexec_mask)
                    tcg_gen_add_i32(tmp, tmp, tmp2);
                else
                    gen_helper_add_cc(tmp, tmp, tmp2);
                dead_tmp(tmp2);
                store_reg(s, rd, tmp);
                break;
            case 3: /* sub */
                if (s->condexec_mask)
                    tcg_gen_sub_i32(tmp, tmp, tmp2);
                else
                    gen_helper_sub_cc(tmp, tmp, tmp2);
                dead_tmp(tmp2);
                store_reg(s, rd, tmp);
                break;
            }
        }
        break;
    case 4:
        if (insn & (1 << 11)) {
            rd = (insn >> 8) & 7;
            /* load pc-relative.  Bit 1 of PC is ignored.  */
            val = s->pc + 2 + ((insn & 0xff) * 4);
            val &= ~(uint32_t)2;
            addr = new_tmp();
            tcg_gen_movi_i32(addr, val);
            tmp = gen_ld32(addr, IS_USER(s));
            dead_tmp(addr);
            store_reg(s, rd, tmp);
            break;
        }
        if (insn & (1 << 10)) {
            /* data processing extended or blx */
            rd = (insn & 7) | ((insn >> 4) & 8);
            rm = (insn >> 3) & 0xf;
            op = (insn >> 8) & 3;
            switch (op) {
            case 0: /* add */
                tmp = load_reg(s, rd);
                tmp2 = load_reg(s, rm);
                tcg_gen_add_i32(tmp, tmp, tmp2);
                dead_tmp(tmp2);
                store_reg(s, rd, tmp);
                break;
            case 1: /* cmp */
                tmp = load_reg(s, rd);
                tmp2 = load_reg(s, rm);
                gen_helper_sub_cc(tmp, tmp, tmp2);
                dead_tmp(tmp2);
                dead_tmp(tmp);
                break;
            case 2: /* mov/cpy */
                tmp = load_reg(s, rm);
                store_reg(s, rd, tmp);
                break;
            case 3:/* branch [and link] exchange thumb register */
                tmp = load_reg(s, rm);
                if (insn & (1 << 7)) {
                    val = (uint32_t)s->pc | 1;
                    tmp2 = new_tmp();
                    tcg_gen_movi_i32(tmp2, val);
                    store_reg(s, 14, tmp2);
                }
                gen_bx(s, tmp);
                break;
            }
            break;
        }

        /* data processing register */
        rd = insn & 7;
        rm = (insn >> 3) & 7;
        op = (insn >> 6) & 0xf;
        if (op == 2 || op == 3 || op == 4 || op == 7) {
            /* the shift/rotate ops want the operands backwards */
            val = rm;
            rm = rd;
            rd = val;
            val = 1;
        } else {
            val = 0;
        }

        if (op == 9) { /* neg */
            tmp = new_tmp();
            tcg_gen_movi_i32(tmp, 0);
        } else if (op != 0xf) { /* mvn doesn't read its first operand */
            tmp = load_reg(s, rd);
        } else {
            TCGV_UNUSED(tmp);
        }

        tmp2 = load_reg(s, rm);
        switch (op) {
        case 0x0: /* and */
            tcg_gen_and_i32(tmp, tmp, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            break;
        case 0x1: /* eor */
            tcg_gen_xor_i32(tmp, tmp, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            break;
        case 0x2: /* lsl */
            if (s->condexec_mask) {
                gen_helper_shl(tmp2, tmp2, tmp);
            } else {
                gen_helper_shl_cc(tmp2, tmp2, tmp);
                gen_logic_CC(tmp2);
            }
            break;
        case 0x3: /* lsr */
            if (s->condexec_mask) {
                gen_helper_shr(tmp2, tmp2, tmp);
            } else {
                gen_helper_shr_cc(tmp2, tmp2, tmp);
                gen_logic_CC(tmp2);
            }
            break;
        case 0x4: /* asr */
            if (s->condexec_mask) {
                gen_helper_sar(tmp2, tmp2, tmp);
            } else {
                gen_helper_sar_cc(tmp2, tmp2, tmp);
                gen_logic_CC(tmp2);
            }
            break;
        case 0x5: /* adc */
            if (s->condexec_mask)
                gen_adc(tmp, tmp2);
            else
                gen_helper_adc_cc(tmp, tmp, tmp2);
            break;
        case 0x6: /* sbc */
            if (s->condexec_mask)
                gen_sub_carry(tmp, tmp, tmp2);
            else
                gen_helper_sbc_cc(tmp, tmp, tmp2);
            break;
        case 0x7: /* ror */
            if (s->condexec_mask) {
                tcg_gen_andi_i32(tmp, tmp, 0x1f);
                tcg_gen_rotr_i32(tmp2, tmp2, tmp);
            } else {
                gen_helper_ror_cc(tmp2, tmp2, tmp);
                gen_logic_CC(tmp2);
            }
            break;
        case 0x8: /* tst */
            tcg_gen_and_i32(tmp, tmp, tmp2);
            gen_logic_CC(tmp);
            rd = 16;
            break;
        case 0x9: /* neg */
            if (s->condexec_mask)
                tcg_gen_neg_i32(tmp, tmp2);
            else
                gen_helper_sub_cc(tmp, tmp, tmp2);
            break;
        case 0xa: /* cmp */
            gen_helper_sub_cc(tmp, tmp, tmp2);
            rd = 16;
            break;
        case 0xb: /* cmn */
            gen_helper_add_cc(tmp, tmp, tmp2);
            rd = 16;
            break;
        case 0xc: /* orr */
            tcg_gen_or_i32(tmp, tmp, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            break;
        case 0xd: /* mul */
            tcg_gen_mul_i32(tmp, tmp, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            break;
        case 0xe: /* bic */
            tcg_gen_andc_i32(tmp, tmp, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp);
            break;
        case 0xf: /* mvn */
            tcg_gen_not_i32(tmp2, tmp2);
            if (!s->condexec_mask)
                gen_logic_CC(tmp2);
            val = 1;
            rm = rd;
            break;
        }
        if (rd != 16) {
            if (val) {
                store_reg(s, rm, tmp2);
                if (op != 0xf)
                    dead_tmp(tmp);
            } else {
                store_reg(s, rd, tmp);
                dead_tmp(tmp2);
            }
        } else {
            dead_tmp(tmp);
            dead_tmp(tmp2);
        }
        break;

    case 5:
        /* load/store register offset.  */
        rd = insn & 7;
        rn = (insn >> 3) & 7;
        rm = (insn >> 6) & 7;
        op = (insn >> 9) & 7;
        addr = load_reg(s, rn);
        tmp = load_reg(s, rm);
        tcg_gen_add_i32(addr, addr, tmp);
        dead_tmp(tmp);

        if (op < 3) /* store */
            tmp = load_reg(s, rd);

        switch (op) {
        case 0: /* str */
            gen_st32(tmp, addr, IS_USER(s));
            break;
        case 1: /* strh */
            gen_st16(tmp, addr, IS_USER(s));
            break;
        case 2: /* strb */
            gen_st8(tmp, addr, IS_USER(s));
            break;
        case 3: /* ldrsb */
            tmp = gen_ld8s(addr, IS_USER(s));
            break;
        case 4: /* ldr */
            tmp = gen_ld32(addr, IS_USER(s));
            break;
        case 5: /* ldrh */
            tmp = gen_ld16u(addr, IS_USER(s));
            break;
        case 6: /* ldrb */
            tmp = gen_ld8u(addr, IS_USER(s));
            break;
        case 7: /* ldrsh */
            tmp = gen_ld16s(addr, IS_USER(s));
            break;
        }
        if (op >= 3) /* load */
            store_reg(s, rd, tmp);
        dead_tmp(addr);
        break;

    case 6:
        /* load/store word immediate offset */
        rd = insn & 7;
        rn = (insn >> 3) & 7;
        addr = load_reg(s, rn);
        val = (insn >> 4) & 0x7c;
        tcg_gen_addi_i32(addr, addr, val);

        if (insn & (1 << 11)) {
            /* load */
            tmp = gen_ld32(addr, IS_USER(s));
            store_reg(s, rd, tmp);
        } else {
            /* store */
            tmp = load_reg(s, rd);
            gen_st32(tmp, addr, IS_USER(s));
        }
        dead_tmp(addr);
        break;

    case 7:
        /* load/store byte immediate offset */
        rd = insn & 7;
        rn = (insn >> 3) & 7;
        addr = load_reg(s, rn);
        val = (insn >> 6) & 0x1f;
        tcg_gen_addi_i32(addr, addr, val);

        if (insn & (1 << 11)) {
            /* load */
            tmp = gen_ld8u(addr, IS_USER(s));
            store_reg(s, rd, tmp);
        } else {
            /* store */
            tmp = load_reg(s, rd);
            gen_st8(tmp, addr, IS_USER(s));
        }
        dead_tmp(addr);
        break;

    case 8:
        /* load/store halfword immediate offset */
        rd = insn & 7;
        rn = (insn >> 3) & 7;
        addr = load_reg(s, rn);
        val = (insn >> 5) & 0x3e;
        tcg_gen_addi_i32(addr, addr, val);

        if (insn & (1 << 11)) {
            /* load */
            tmp = gen_ld16u(addr, IS_USER(s));
            store_reg(s, rd, tmp);
        } else {
            /* store */
            tmp = load_reg(s, rd);
            gen_st16(tmp, addr, IS_USER(s));
        }
        dead_tmp(addr);
        break;

    case 9:
        /* load/store from stack */
        rd = (insn >> 8) & 7;
        addr = load_reg(s, 13);
        val = (insn & 0xff) * 4;
        tcg_gen_addi_i32(addr, addr, val);

        if (insn & (1 << 11)) {
            /* load */
            tmp = gen_ld32(addr, IS_USER(s));
            store_reg(s, rd, tmp);
        } else {
            /* store */
            tmp = load_reg(s, rd);
            gen_st32(tmp, addr, IS_USER(s));
        }
        dead_tmp(addr);
        break;

    case 10:
        /* add to high reg */
        rd = (insn >> 8) & 7;
        if (insn & (1 << 11)) {
            /* SP */
            tmp = load_reg(s, 13);
        } else {
            /* PC. bit 1 is ignored.  */
            tmp = new_tmp();
            tcg_gen_movi_i32(tmp, (s->pc + 2) & ~(uint32_t)2);
        }
        val = (insn & 0xff) * 4;
        tcg_gen_addi_i32(tmp, tmp, val);
        store_reg(s, rd, tmp);
        break;

    case 11:
        /* misc */
        op = (insn >> 8) & 0xf;
        switch (op) {
        case 0:
            /* adjust stack pointer */
            tmp = load_reg(s, 13);
            val = (insn & 0x7f) * 4;
            if (insn & (1 << 7))
                val = -(int32_t)val;
            tcg_gen_addi_i32(tmp, tmp, val);
            store_reg(s, 13, tmp);
            break;

        case 2: /* sign/zero extend.  */
            ARCH(6);
            rd = insn & 7;
            rm = (insn >> 3) & 7;
            tmp = load_reg(s, rm);
            switch ((insn >> 6) & 3) {
            case 0: gen_sxth(tmp); break;
            case 1: gen_sxtb(tmp); break;
            case 2: gen_uxth(tmp); break;
            case 3: gen_uxtb(tmp); break;
            }
            store_reg(s, rd, tmp);
            break;
        case 4: case 5: case 0xc: case 0xd:
            /* push/pop */
            addr = load_reg(s, 13);
            if (insn & (1 << 8))
                offset = 4;
            else
                offset = 0;
            for (i = 0; i < 8; i++) {
                if (insn & (1 << i))
                    offset += 4;
            }
            if ((insn & (1 << 11)) == 0) {
                tcg_gen_addi_i32(addr, addr, -offset);
            }
            for (i = 0; i < 8; i++) {
                if (insn & (1 << i)) {
                    if (insn & (1 << 11)) {
                        /* pop */
                        tmp = gen_ld32(addr, IS_USER(s));
                        store_reg(s, i, tmp);
                    } else {
                        /* push */
                        tmp = load_reg(s, i);
                        gen_st32(tmp, addr, IS_USER(s));
                    }
                    /* advance to the next address.  */
                    tcg_gen_addi_i32(addr, addr, 4);
                }
            }
            TCGV_UNUSED(tmp);
            if (insn & (1 << 8)) {
                if (insn & (1 << 11)) {
                    /* pop pc */
                    tmp = gen_ld32(addr, IS_USER(s));
                    /* don't set the pc until the rest of the instruction
                       has completed */
                } else {
                    /* push lr */
                    tmp = load_reg(s, 14);
                    gen_st32(tmp, addr, IS_USER(s));
                }
                tcg_gen_addi_i32(addr, addr, 4);
            }
            if ((insn & (1 << 11)) == 0) {
                tcg_gen_addi_i32(addr, addr, -offset);
            }
            /* write back the new stack pointer */
            store_reg(s, 13, addr);
            /* set the new PC value */
            if ((insn & 0x0900) == 0x0900)
                gen_bx(s, tmp);
            break;

        case 1: case 3: case 9: case 11: /* czb */
            rm = insn & 7;
            tmp = load_reg(s, rm);
            s->condlabel = gen_new_label();
            s->condjmp = 1;
            if (insn & (1 << 11))
                tcg_gen_brcondi_i32(TCG_COND_EQ, tmp, 0, s->condlabel);
            else
                tcg_gen_brcondi_i32(TCG_COND_NE, tmp, 0, s->condlabel);
            dead_tmp(tmp);
            offset = ((insn & 0xf8) >> 2) | (insn & 0x200) >> 3;
            val = (uint32_t)s->pc + 2;
            val += offset;
            gen_jmp(s, val);
            break;

        case 15: /* IT, nop-hint.  */
            if ((insn & 0xf) == 0) {
                gen_nop_hint(s, (insn >> 4) & 0xf);
                break;
            }
            /* If Then.  */
            s->condexec_cond = (insn >> 4) & 0xe;
            s->condexec_mask = insn & 0x1f;
            /* No actual code generated for this insn, just setup state.  */
            break;

        case 0xe: /* bkpt */
            gen_set_condexec(s);
            gen_set_pc_im(s->pc - 2);
            gen_exception(EXCP_BKPT);
            s->is_jmp = DISAS_JUMP;
            break;

        case 0xa: /* rev */
            ARCH(6);
            rn = (insn >> 3) & 0x7;
            rd = insn & 0x7;
            tmp = load_reg(s, rn);
            switch ((insn >> 6) & 3) {
            case 0: tcg_gen_bswap32_i32(tmp, tmp); break;
            case 1: gen_rev16(tmp); break;
            case 3: gen_revsh(tmp); break;
            default: goto illegal_op;
            }
            store_reg(s, rd, tmp);
            break;

        case 6: /* cps */
            ARCH(6);
            if (IS_USER(s))
                break;
            if (IS_M(env)) {
                tmp = tcg_const_i32((insn & (1 << 4)) != 0);
                /* PRIMASK */
                if (insn & 1) {
                    addr = tcg_const_i32(16);
                    gen_helper_v7m_msr(cpu_env, addr, tmp);
                    tcg_temp_free_i32(addr);
                }
                /* FAULTMASK */
                if (insn & 2) {
                    addr = tcg_const_i32(17);
                    gen_helper_v7m_msr(cpu_env, addr, tmp);
                    tcg_temp_free_i32(addr);
                }
                tcg_temp_free_i32(tmp);
                gen_lookup_tb(s);
            } else {
                if (insn & (1 << 4))
                    shift = CPSR_A | CPSR_I | CPSR_F;
                else
                    shift = 0;
                gen_set_psr_im(s, ((insn & 7) << 6), 0, shift);
            }
            break;

        default:
            goto undef;
        }
        break;

    case 12:
        /* load/store multiple */
        rn = (insn >> 8) & 0x7;
        addr = load_reg(s, rn);
        for (i = 0; i < 8; i++) {
            if (insn & (1 << i)) {
                if (insn & (1 << 11)) {
                    /* load */
                    tmp = gen_ld32(addr, IS_USER(s));
                    store_reg(s, i, tmp);
                } else {
                    /* store */
                    tmp = load_reg(s, i);
                    gen_st32(tmp, addr, IS_USER(s));
                }
                /* advance to the next address */
                tcg_gen_addi_i32(addr, addr, 4);
            }
        }
        /* Base register writeback.  */
        if ((insn & (1 << rn)) == 0) {
            store_reg(s, rn, addr);
        } else {
            dead_tmp(addr);
        }
        break;

    case 13:
        /* conditional branch or swi */
        cond = (insn >> 8) & 0xf;
        if (cond == 0xe)
            goto undef;

        if (cond == 0xf) {
            /* swi */
            gen_set_condexec(s);
            gen_set_pc_im(s->pc);
            s->is_jmp = DISAS_SWI;
            break;
        }
        /* generate a conditional jump to next instruction */
        s->condlabel = gen_new_label();
        gen_test_cc(cond ^ 1, s->condlabel);
        s->condjmp = 1;

        /* jump to the offset */
        val = (uint32_t)s->pc + 2;
        offset = ((int32_t)insn << 24) >> 24;
        val += offset << 1;
        gen_jmp(s, val);
        break;

    case 14:
        if (insn & (1 << 11)) {
            if (disas_thumb2_insn(env, s, insn))
              goto undef32;
            break;
        }
        /* unconditional branch */
        val = (uint32_t)s->pc;
        offset = ((int32_t)insn << 21) >> 21;
        val += (offset << 1) + 2;
        gen_jmp(s, val);
        break;

    case 15:
        if (disas_thumb2_insn(env, s, insn))
            goto undef32;
        break;
    }
    return;
undef32:
    gen_set_condexec(s);
    gen_set_pc_im(s->pc - 4);
    gen_exception(EXCP_UDEF);
    s->is_jmp = DISAS_JUMP;
    return;
illegal_op:
undef:
    gen_set_condexec(s);
    gen_set_pc_im(s->pc - 2);
    gen_exception(EXCP_UDEF);
    s->is_jmp = DISAS_JUMP;
}
#endif

void Arm::disas_arm_insn()
{
    unsigned int cond, insn, val, op1, i, shift, rm, rs, rn, rd, sh;
    uint32_t tmp;
    uint32_t tmp2;
    uint32_t tmp3;
    uint32_t addr;
    uint64_t tmp64;

    // read the next instruction
    insn = this->rd_i_l(this->pc);

    // check here if there was an exception raised (when fetching instruction for example)

    // check here if we have reached an instruction breakpoint (instruction was fetch was OK)

    // increment the PC
    this->pc += 4;

    // M variants do not implement ARM mode
    if (IS_M())
        goto illegal_op;
    cond = insn >> 28;
    if (cond == 0xf){
        /* Unconditional instructions.  */
        if (((insn >> 25) & 7) == 1) {
            /* NEON Data processing.  */
            if (!arm_feature(env, ARM_FEATURE_NEON))
                goto illegal_op;

            if (disas_neon_data_insn(env, s, insn))
                goto illegal_op;
            return;
        }
        if ((insn & 0x0f100000) == 0x04000000) {
            /* NEON load/store.  */
            if (!arm_feature(env, ARM_FEATURE_NEON))
                goto illegal_op;

            if (disas_neon_ls_insn(env, s, insn))
                goto illegal_op;
            return;
        }
        if ((insn & 0x0d70f000) == 0x0550f000)
            return; /* PLD */
        else if ((insn & 0x0ffffdff) == 0x01010000) {
            ARCH(6);
            /* setend */
            if (insn & (1 << 9)) {
                /* BE8 mode not implemented.  */
                goto illegal_op;
            }
            return;
        } else if ((insn & 0x0fffff00) == 0x057ff000) {
            switch ((insn >> 4) & 0xf) {
            case 1: /* clrex */
                ARCH(6K);
                gen_clrex(s);
                return;
            case 4: /* dsb */
            case 5: /* dmb */
            case 6: /* isb */
                ARCH(7);
                /* We don't emulate caches so these are a no-op.  */
                return;
            default:
                goto illegal_op;
            }
        } else if ((insn & 0x0e5fffe0) == 0x084d0500) {
            /* srs */
            int32_t offset;
            if (IS_USER(s))
                goto illegal_op;
            ARCH(6);
            op1 = (insn & 0x1f);
            if (op1 == (env->uncached_cpsr & CPSR_M)) {
                addr = load_reg(s, 13);
            } else {
                addr = new_tmp();
                tmp = tcg_const_i32(op1);
                gen_helper_get_r13_banked(addr, cpu_env, tmp);
                tcg_temp_free_i32(tmp);
            }
            i = (insn >> 23) & 3;
            switch (i) {
            case 0: offset = -4; break; /* DA */
            case 1: offset = 0; break; /* IA */
            case 2: offset = -8; break; /* DB */
            case 3: offset = 4; break; /* IB */
            default: abort();
            }
            if (offset)
                tcg_gen_addi_i32(addr, addr, offset);
            tmp = load_reg(s, 14);
            gen_st32(tmp, addr, 0);
            tmp = load_cpu_field(spsr);
            tcg_gen_addi_i32(addr, addr, 4);
            gen_st32(tmp, addr, 0);
            if (insn & (1 << 21)) {
                /* Base writeback.  */
                switch (i) {
                case 0: offset = -8; break;
                case 1: offset = 4; break;
                case 2: offset = -4; break;
                case 3: offset = 0; break;
                default: abort();
                }
                if (offset)
                    tcg_gen_addi_i32(addr, addr, offset);
                if (op1 == (env->uncached_cpsr & CPSR_M)) {
                    store_reg(s, 13, addr);
                } else {
                    tmp = tcg_const_i32(op1);
                    gen_helper_set_r13_banked(cpu_env, tmp, addr);
                    tcg_temp_free_i32(tmp);
                    dead_tmp(addr);
                }
            } else {
                dead_tmp(addr);
            }
        } else if ((insn & 0x0e5fffe0) == 0x081d0a00) {
            /* rfe */
            int32_t offset;
            if (IS_USER(s))
                goto illegal_op;
            ARCH(6);
            rn = (insn >> 16) & 0xf;
            addr = load_reg(s, rn);
            i = (insn >> 23) & 3;
            switch (i) {
            case 0: offset = -4; break; /* DA */
            case 1: offset = 0; break; /* IA */
            case 2: offset = -8; break; /* DB */
            case 3: offset = 4; break; /* IB */
            default: abort();
            }
            if (offset)
                tcg_gen_addi_i32(addr, addr, offset);
            /* Load PC into tmp and CPSR into tmp2.  */
            tmp = gen_ld32(addr, 0);
            tcg_gen_addi_i32(addr, addr, 4);
            tmp2 = gen_ld32(addr, 0);
            if (insn & (1 << 21)) {
                /* Base writeback.  */
                switch (i) {
                case 0: offset = -8; break;
                case 1: offset = 4; break;
                case 2: offset = -4; break;
                case 3: offset = 0; break;
                default: abort();
                }
                if (offset)
                    tcg_gen_addi_i32(addr, addr, offset);
                store_reg(s, rn, addr);
            } else {
                dead_tmp(addr);
            }
            gen_rfe(s, tmp, tmp2);
            return;
        } else if ((insn & 0x0e000000) == 0x0a000000) {
            /* branch link and change to thumb (blx <offset>) */
            int32_t offset;

            val = (uint32_t)s->pc;
            tmp = new_tmp();
            tcg_gen_movi_i32(tmp, val);
            store_reg(s, 14, tmp);
            /* Sign-extend the 24-bit offset */
            offset = (((int32_t)insn) << 8) >> 8;
            /* offset * 4 + bit24 * 2 + (thumb bit) */
            val += (offset << 2) | ((insn >> 23) & 2) | 1;
            /* pipeline offset */
            val += 4;
            gen_bx_im(s, val);
            return;
        } else if ((insn & 0x0e000f00) == 0x0c000100) {
            if (arm_feature(env, ARM_FEATURE_IWMMXT)) {
                /* iWMMXt register transfer.  */
                if (env->cp15.c15_cpar & (1 << 1))
                    if (!disas_iwmmxt_insn(env, s, insn))
                        return;
            }
        } else if ((insn & 0x0fe00000) == 0x0c400000) {
            /* Coprocessor double register transfer.  */
        } else if ((insn & 0x0f000010) == 0x0e000010) {
            /* Additional coprocessor register transfer.  */
        } else if ((insn & 0x0ff10020) == 0x01000000) {
            uint32_t mask;
            uint32_t val;
            /* cps (privileged) */
            if (IS_USER(s))
                return;
            mask = val = 0;
            if (insn & (1 << 19)) {
                if (insn & (1 << 8))
                    mask |= CPSR_A;
                if (insn & (1 << 7))
                    mask |= CPSR_I;
                if (insn & (1 << 6))
                    mask |= CPSR_F;
                if (insn & (1 << 18))
                    val |= mask;
            }
            if (insn & (1 << 17)) {
                mask |= CPSR_M;
                val |= (insn & 0x1f);
            }
            if (mask) {
                gen_set_psr_im(s, mask, 0, val);
            }
            return;
        }
        goto illegal_op;
    }
    if (cond != 0xe) {
        /* if not always execute, we generate a conditional jump to
           next instruction */
        s->condlabel = gen_new_label();
        gen_test_cc(cond ^ 1, s->condlabel);
        s->condjmp = 1;
    }
    if ((insn & 0x0f900000) == 0x03000000) {
        if ((insn & (1 << 21)) == 0) {
            ARCH(6T2);
            rd = (insn >> 12) & 0xf;
            val = ((insn >> 4) & 0xf000) | (insn & 0xfff);
            if ((insn & (1 << 22)) == 0) {
                /* MOVW */
                tmp = new_tmp();
                tcg_gen_movi_i32(tmp, val);
            } else {
                /* MOVT */
                tmp = load_reg(s, rd);
                tcg_gen_ext16u_i32(tmp, tmp);
                tcg_gen_ori_i32(tmp, tmp, val << 16);
            }
            store_reg(s, rd, tmp);
        } else {
            if (((insn >> 12) & 0xf) != 0xf)
                goto illegal_op;
            if (((insn >> 16) & 0xf) == 0) {
                gen_nop_hint(s, insn & 0xff);
            } else {
                /* CPSR = immediate */
                val = insn & 0xff;
                shift = ((insn >> 8) & 0xf) * 2;
                if (shift)
                    val = (val >> shift) | (val << (32 - shift));
                i = ((insn & (1 << 22)) != 0);
                if (gen_set_psr_im(s, msr_mask(env, s, (insn >> 16) & 0xf, i), i, val))
                    goto illegal_op;
            }
        }
    } else if ((insn & 0x0f900000) == 0x01000000
               && (insn & 0x00000090) != 0x00000090) {
        /* miscellaneous instructions */
        op1 = (insn >> 21) & 3;
        sh = (insn >> 4) & 0xf;
        rm = insn & 0xf;
        switch (sh) {
        case 0x0: /* move program status register */
            if (op1 & 1) {
                /* PSR = reg */
                tmp = load_reg(s, rm);
                i = ((op1 & 2) != 0);
                if (gen_set_psr(s, msr_mask(env, s, (insn >> 16) & 0xf, i), i, tmp))
                    goto illegal_op;
            } else {
                /* reg = PSR */
                rd = (insn >> 12) & 0xf;
                if (op1 & 2) {
                    if (IS_USER(s))
                        goto illegal_op;
                    tmp = load_cpu_field(spsr);
                } else {
                    tmp = new_tmp();
                    gen_helper_cpsr_read(tmp);
                }
                store_reg(s, rd, tmp);
            }
            break;
        case 0x1:
            if (op1 == 1) {
                /* branch/exchange thumb (bx).  */
                tmp = load_reg(s, rm);
                gen_bx(s, tmp);
            } else if (op1 == 3) {
                /* clz */
                rd = (insn >> 12) & 0xf;
                tmp = load_reg(s, rm);
                gen_helper_clz(tmp, tmp);
                store_reg(s, rd, tmp);
            } else {
                goto illegal_op;
            }
            break;
        case 0x2:
            if (op1 == 1) {
                ARCH(5J); /* bxj */
                /* Trivial implementation equivalent to bx.  */
                tmp = load_reg(s, rm);
                gen_bx(s, tmp);
            } else {
                goto illegal_op;
            }
            break;
        case 0x3:
            if (op1 != 1)
              goto illegal_op;

            /* branch link/exchange thumb (blx) */
            tmp = load_reg(s, rm);
            tmp2 = new_tmp();
            tcg_gen_movi_i32(tmp2, s->pc);
            store_reg(s, 14, tmp2);
            gen_bx(s, tmp);
            break;
        case 0x5: /* saturating add/subtract */
            rd = (insn >> 12) & 0xf;
            rn = (insn >> 16) & 0xf;
            tmp = load_reg(s, rm);
            tmp2 = load_reg(s, rn);
            if (op1 & 2)
                gen_helper_double_saturate(tmp2, tmp2);
            if (op1 & 1)
                gen_helper_sub_saturate(tmp, tmp, tmp2);
            else
                gen_helper_add_saturate(tmp, tmp, tmp2);
            dead_tmp(tmp2);
            store_reg(s, rd, tmp);
            break;
        case 7: /* bkpt */
            gen_set_condexec(s);
            gen_set_pc_im(s->pc - 4);
            gen_exception(EXCP_BKPT);
            s->is_jmp = DISAS_JUMP;
            break;
        case 0x8: /* signed multiply */
        case 0xa:
        case 0xc:
        case 0xe:
            rs = (insn >> 8) & 0xf;
            rn = (insn >> 12) & 0xf;
            rd = (insn >> 16) & 0xf;
            if (op1 == 1) {
                /* (32 * 16) >> 16 */
                tmp = load_reg(s, rm);
                tmp2 = load_reg(s, rs);
                if (sh & 4)
                    tcg_gen_sari_i32(tmp2, tmp2, 16);
                else
                    gen_sxth(tmp2);
                tmp64 = gen_muls_i64_i32(tmp, tmp2);
                tcg_gen_shri_i64(tmp64, tmp64, 16);
                tmp = new_tmp();
                tcg_gen_trunc_i64_i32(tmp, tmp64);
                tcg_temp_free_i64(tmp64);
                if ((sh & 2) == 0) {
                    tmp2 = load_reg(s, rn);
                    gen_helper_add_setq(tmp, tmp, tmp2);
                    dead_tmp(tmp2);
                }
                store_reg(s, rd, tmp);
            } else {
                /* 16 * 16 */
                tmp = load_reg(s, rm);
                tmp2 = load_reg(s, rs);
                gen_mulxy(tmp, tmp2, sh & 2, sh & 4);
                dead_tmp(tmp2);
                if (op1 == 2) {
                    tmp64 = tcg_temp_new_i64();
                    tcg_gen_ext_i32_i64(tmp64, tmp);
                    dead_tmp(tmp);
                    gen_addq(s, tmp64, rn, rd);
                    gen_storeq_reg(s, rn, rd, tmp64);
                    tcg_temp_free_i64(tmp64);
                } else {
                    if (op1 == 0) {
                        tmp2 = load_reg(s, rn);
                        gen_helper_add_setq(tmp, tmp, tmp2);
                        dead_tmp(tmp2);
                    }
                    store_reg(s, rd, tmp);
                }
            }
            break;
        default:
            goto illegal_op;
        }
    } else if (((insn & 0x0e000000) == 0 &&
                (insn & 0x00000090) != 0x90) ||
               ((insn & 0x0e000000) == (1 << 25))) {
        int set_cc, logic_cc, shiftop;

        op1 = (insn >> 21) & 0xf;
        set_cc = (insn >> 20) & 1;
        logic_cc = table_logic_cc[op1] & set_cc;

        /* data processing instruction */
        if (insn & (1 << 25)) {
            /* immediate operand */
            val = insn & 0xff;
            shift = ((insn >> 8) & 0xf) * 2;
            if (shift) {
                val = (val >> shift) | (val << (32 - shift));
            }
            tmp2 = new_tmp();
            tcg_gen_movi_i32(tmp2, val);
            if (logic_cc && shift) {
                gen_set_CF_bit31(tmp2);
            }
        } else {
            /* register */
            rm = (insn) & 0xf;
            tmp2 = load_reg(s, rm);
            shiftop = (insn >> 5) & 3;
            if (!(insn & (1 << 4))) {
                shift = (insn >> 7) & 0x1f;
                gen_arm_shift_im(tmp2, shiftop, shift, logic_cc);
            } else {
                rs = (insn >> 8) & 0xf;
                tmp = load_reg(s, rs);
                gen_arm_shift_reg(tmp2, shiftop, tmp, logic_cc);
            }
        }
        if (op1 != 0x0f && op1 != 0x0d) {
            rn = (insn >> 16) & 0xf;
            tmp = load_reg(s, rn);
        } else {
            TCGV_UNUSED(tmp);
        }
        rd = (insn >> 12) & 0xf;
        switch(op1) {
        case 0x00:
            tcg_gen_and_i32(tmp, tmp, tmp2);
            if (logic_cc) {
                gen_logic_CC(tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x01:
            tcg_gen_xor_i32(tmp, tmp, tmp2);
            if (logic_cc) {
                gen_logic_CC(tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x02:
            if (set_cc && rd == 15) {
                /* SUBS r15, ... is used for exception return.  */
                if (IS_USER(s)) {
                    goto illegal_op;
                }
                gen_helper_sub_cc(tmp, tmp, tmp2);
                gen_exception_return(s, tmp);
            } else {
                if (set_cc) {
                    gen_helper_sub_cc(tmp, tmp, tmp2);
                } else {
                    tcg_gen_sub_i32(tmp, tmp, tmp2);
                }
                store_reg_bx(env, s, rd, tmp);
            }
            break;
        case 0x03:
            if (set_cc) {
                gen_helper_sub_cc(tmp, tmp2, tmp);
            } else {
                tcg_gen_sub_i32(tmp, tmp2, tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x04:
            if (set_cc) {
                gen_helper_add_cc(tmp, tmp, tmp2);
            } else {
                tcg_gen_add_i32(tmp, tmp, tmp2);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x05:
            if (set_cc) {
                gen_helper_adc_cc(tmp, tmp, tmp2);
            } else {
                gen_add_carry(tmp, tmp, tmp2);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x06:
            if (set_cc) {
                gen_helper_sbc_cc(tmp, tmp, tmp2);
            } else {
                gen_sub_carry(tmp, tmp, tmp2);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x07:
            if (set_cc) {
                gen_helper_sbc_cc(tmp, tmp2, tmp);
            } else {
                gen_sub_carry(tmp, tmp2, tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x08:
            if (set_cc) {
                tcg_gen_and_i32(tmp, tmp, tmp2);
                gen_logic_CC(tmp);
            }
            dead_tmp(tmp);
            break;
        case 0x09:
            if (set_cc) {
                tcg_gen_xor_i32(tmp, tmp, tmp2);
                gen_logic_CC(tmp);
            }
            dead_tmp(tmp);
            break;
        case 0x0a:
            if (set_cc) {
                gen_helper_sub_cc(tmp, tmp, tmp2);
            }
            dead_tmp(tmp);
            break;
        case 0x0b:
            if (set_cc) {
                gen_helper_add_cc(tmp, tmp, tmp2);
            }
            dead_tmp(tmp);
            break;
        case 0x0c:
            tcg_gen_or_i32(tmp, tmp, tmp2);
            if (logic_cc) {
                gen_logic_CC(tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        case 0x0d:
            if (logic_cc && rd == 15) {
                /* MOVS r15, ... is used for exception return.  */
                if (IS_USER(s)) {
                    goto illegal_op;
                }
                gen_exception_return(s, tmp2);
            } else {
                if (logic_cc) {
                    gen_logic_CC(tmp2);
                }
                store_reg_bx(env, s, rd, tmp2);
            }
            break;
        case 0x0e:
            tcg_gen_andc_i32(tmp, tmp, tmp2);
            if (logic_cc) {
                gen_logic_CC(tmp);
            }
            store_reg_bx(env, s, rd, tmp);
            break;
        default:
        case 0x0f:
            tcg_gen_not_i32(tmp2, tmp2);
            if (logic_cc) {
                gen_logic_CC(tmp2);
            }
            store_reg_bx(env, s, rd, tmp2);
            break;
        }
        if (op1 != 0x0f && op1 != 0x0d) {
            dead_tmp(tmp2);
        }
    } else {
        /* other instructions */
        op1 = (insn >> 24) & 0xf;
        switch(op1) {
        case 0x0:
        case 0x1:
            /* multiplies, extra load/stores */
            sh = (insn >> 5) & 3;
            if (sh == 0) {
                if (op1 == 0x0) {
                    rd = (insn >> 16) & 0xf;
                    rn = (insn >> 12) & 0xf;
                    rs = (insn >> 8) & 0xf;
                    rm = (insn) & 0xf;
                    op1 = (insn >> 20) & 0xf;
                    switch (op1) {
                    case 0: case 1: case 2: case 3: case 6:
                        /* 32 bit mul */
                        tmp = load_reg(s, rs);
                        tmp2 = load_reg(s, rm);
                        tcg_gen_mul_i32(tmp, tmp, tmp2);
                        dead_tmp(tmp2);
                        if (insn & (1 << 22)) {
                            /* Subtract (mls) */
                            ARCH(6T2);
                            tmp2 = load_reg(s, rn);
                            tcg_gen_sub_i32(tmp, tmp2, tmp);
                            dead_tmp(tmp2);
                        } else if (insn & (1 << 21)) {
                            /* Add */
                            tmp2 = load_reg(s, rn);
                            tcg_gen_add_i32(tmp, tmp, tmp2);
                            dead_tmp(tmp2);
                        }
                        if (insn & (1 << 20))
                            gen_logic_CC(tmp);
                        store_reg(s, rd, tmp);
                        break;
                    default:
                        /* 64 bit mul */
                        tmp = load_reg(s, rs);
                        tmp2 = load_reg(s, rm);
                        if (insn & (1 << 22))
                            tmp64 = gen_muls_i64_i32(tmp, tmp2);
                        else
                            tmp64 = gen_mulu_i64_i32(tmp, tmp2);
                        if (insn & (1 << 21)) /* mult accumulate */
                            gen_addq(s, tmp64, rn, rd);
                        if (!(insn & (1 << 23))) { /* double accumulate */
                            ARCH(6);
                            gen_addq_lo(s, tmp64, rn);
                            gen_addq_lo(s, tmp64, rd);
                        }
                        if (insn & (1 << 20))
                            gen_logicq_cc(tmp64);
                        gen_storeq_reg(s, rn, rd, tmp64);
                        tcg_temp_free_i64(tmp64);
                        break;
                    }
                } else {
                    rn = (insn >> 16) & 0xf;
                    rd = (insn >> 12) & 0xf;
                    if (insn & (1 << 23)) {
                        /* load/store exclusive */
                        op1 = (insn >> 21) & 0x3;
                        if (op1)
                            ARCH(6K);
                        else
                            ARCH(6);
                        addr = tcg_temp_local_new_i32();
                        load_reg_var(s, addr, rn);
                        if (insn & (1 << 20)) {
                            switch (op1) {
                            case 0: /* ldrex */
                                gen_load_exclusive(s, rd, 15, addr, 2);
                                break;
                            case 1: /* ldrexd */
                                gen_load_exclusive(s, rd, rd + 1, addr, 3);
                                break;
                            case 2: /* ldrexb */
                                gen_load_exclusive(s, rd, 15, addr, 0);
                                break;
                            case 3: /* ldrexh */
                                gen_load_exclusive(s, rd, 15, addr, 1);
                                break;
                            default:
                                abort();
                            }
                        } else {
                            rm = insn & 0xf;
                            switch (op1) {
                            case 0:  /*  strex */
                                gen_store_exclusive(s, rd, rm, 15, addr, 2);
                                break;
                            case 1: /*  strexd */
                                gen_store_exclusive(s, rd, rm, rm + 1, addr, 2);
                                break;
                            case 2: /*  strexb */
                                gen_store_exclusive(s, rd, rm, 15, addr, 0);
                                break;
                            case 3: /* strexh */
                                gen_store_exclusive(s, rd, rm, 15, addr, 1);
                                break;
                            default:
                                abort();
                            }
                        }
                        tcg_temp_free(addr);
                    } else {
                        /* SWP instruction */
                        rm = (insn) & 0xf;

                        /* ??? This is not really atomic.  However we know
                           we never have multiple CPUs running in parallel,
                           so it is good enough.  */
                        addr = load_reg(s, rn);
                        tmp = load_reg(s, rm);
                        if (insn & (1 << 22)) {
                            tmp2 = gen_ld8u(addr, IS_USER(s));
                            gen_st8(tmp, addr, IS_USER(s));
                        } else {
                            tmp2 = gen_ld32(addr, IS_USER(s));
                            gen_st32(tmp, addr, IS_USER(s));
                        }
                        dead_tmp(addr);
                        store_reg(s, rd, tmp2);
                    }
                }
            } else {
                int address_offset;
                int load;
                /* Misc load/store */
                rn = (insn >> 16) & 0xf;
                rd = (insn >> 12) & 0xf;
                addr = load_reg(s, rn);
                if (insn & (1 << 24))
                    gen_add_datah_offset(s, insn, 0, addr);
                address_offset = 0;
                if (insn & (1 << 20)) {
                    /* load */
                    switch(sh) {
                    case 1:
                        tmp = gen_ld16u(addr, IS_USER(s));
                        break;
                    case 2:
                        tmp = gen_ld8s(addr, IS_USER(s));
                        break;
                    default:
                    case 3:
                        tmp = gen_ld16s(addr, IS_USER(s));
                        break;
                    }
                    load = 1;
                } else if (sh & 2) {
                    /* doubleword */
                    if (sh & 1) {
                        /* store */
                        tmp = load_reg(s, rd);
                        gen_st32(tmp, addr, IS_USER(s));
                        tcg_gen_addi_i32(addr, addr, 4);
                        tmp = load_reg(s, rd + 1);
                        gen_st32(tmp, addr, IS_USER(s));
                        load = 0;
                    } else {
                        /* load */
                        tmp = gen_ld32(addr, IS_USER(s));
                        store_reg(s, rd, tmp);
                        tcg_gen_addi_i32(addr, addr, 4);
                        tmp = gen_ld32(addr, IS_USER(s));
                        rd++;
                        load = 1;
                    }
                    address_offset = -4;
                } else {
                    /* store */
                    tmp = load_reg(s, rd);
                    gen_st16(tmp, addr, IS_USER(s));
                    load = 0;
                }
                /* Perform base writeback before the loaded value to
                   ensure correct behavior with overlapping index registers.
                   ldrd with base writeback is is undefined if the
                   destination and index registers overlap.  */
                if (!(insn & (1 << 24))) {
                    gen_add_datah_offset(s, insn, address_offset, addr);
                    store_reg(s, rn, addr);
                } else if (insn & (1 << 21)) {
                    if (address_offset)
                        tcg_gen_addi_i32(addr, addr, address_offset);
                    store_reg(s, rn, addr);
                } else {
                    dead_tmp(addr);
                }
                if (load) {
                    /* Complete the load.  */
                    store_reg(s, rd, tmp);
                }
            }
            break;
        case 0x4:
        case 0x5:
            goto do_ldst;
        case 0x6:
        case 0x7:
            if (insn & (1 << 4)) {
                ARCH(6);
                /* Armv6 Media instructions.  */
                rm = insn & 0xf;
                rn = (insn >> 16) & 0xf;
                rd = (insn >> 12) & 0xf;
                rs = (insn >> 8) & 0xf;
                switch ((insn >> 23) & 3) {
                case 0: /* Parallel add/subtract.  */
                    op1 = (insn >> 20) & 7;
                    tmp = load_reg(s, rn);
                    tmp2 = load_reg(s, rm);
                    sh = (insn >> 5) & 7;
                    if ((op1 & 3) == 0 || sh == 5 || sh == 6)
                        goto illegal_op;
                    gen_arm_parallel_addsub(op1, sh, tmp, tmp2);
                    dead_tmp(tmp2);
                    store_reg(s, rd, tmp);
                    break;
                case 1:
                    if ((insn & 0x00700020) == 0) {
                        /* Halfword pack.  */
                        tmp = load_reg(s, rn);
                        tmp2 = load_reg(s, rm);
                        shift = (insn >> 7) & 0x1f;
                        if (insn & (1 << 6)) {
                            /* pkhtb */
                            if (shift == 0)
                                shift = 31;
                            tcg_gen_sari_i32(tmp2, tmp2, shift);
                            tcg_gen_andi_i32(tmp, tmp, 0xffff0000);
                            tcg_gen_ext16u_i32(tmp2, tmp2);
                        } else {
                            /* pkhbt */
                            if (shift)
                                tcg_gen_shli_i32(tmp2, tmp2, shift);
                            tcg_gen_ext16u_i32(tmp, tmp);
                            tcg_gen_andi_i32(tmp2, tmp2, 0xffff0000);
                        }
                        tcg_gen_or_i32(tmp, tmp, tmp2);
                        dead_tmp(tmp2);
                        store_reg(s, rd, tmp);
                    } else if ((insn & 0x00200020) == 0x00200000) {
                        /* [us]sat */
                        tmp = load_reg(s, rm);
                        shift = (insn >> 7) & 0x1f;
                        if (insn & (1 << 6)) {
                            if (shift == 0)
                                shift = 31;
                            tcg_gen_sari_i32(tmp, tmp, shift);
                        } else {
                            tcg_gen_shli_i32(tmp, tmp, shift);
                        }
                        sh = (insn >> 16) & 0x1f;
                        if (sh != 0) {
                            tmp2 = tcg_const_i32(sh);
                            if (insn & (1 << 22))
                                gen_helper_usat(tmp, tmp, tmp2);
                            else
                                gen_helper_ssat(tmp, tmp, tmp2);
                            tcg_temp_free_i32(tmp2);
                        }
                        store_reg(s, rd, tmp);
                    } else if ((insn & 0x00300fe0) == 0x00200f20) {
                        /* [us]sat16 */
                        tmp = load_reg(s, rm);
                        sh = (insn >> 16) & 0x1f;
                        if (sh != 0) {
                            tmp2 = tcg_const_i32(sh);
                            if (insn & (1 << 22))
                                gen_helper_usat16(tmp, tmp, tmp2);
                            else
                                gen_helper_ssat16(tmp, tmp, tmp2);
                            tcg_temp_free_i32(tmp2);
                        }
                        store_reg(s, rd, tmp);
                    } else if ((insn & 0x00700fe0) == 0x00000fa0) {
                        /* Select bytes.  */
                        tmp = load_reg(s, rn);
                        tmp2 = load_reg(s, rm);
                        tmp3 = new_tmp();
                        tcg_gen_ld_i32(tmp3, cpu_env, offsetof(CPUState, GE));
                        gen_helper_sel_flags(tmp, tmp3, tmp, tmp2);
                        dead_tmp(tmp3);
                        dead_tmp(tmp2);
                        store_reg(s, rd, tmp);
                    } else if ((insn & 0x000003e0) == 0x00000060) {
                        tmp = load_reg(s, rm);
                        shift = (insn >> 10) & 3;
                        /* ??? In many cases it's not neccessary to do a
                           rotate, a shift is sufficient.  */
                        if (shift != 0)
                            tcg_gen_rotri_i32(tmp, tmp, shift * 8);
                        op1 = (insn >> 20) & 7;
                        switch (op1) {
                        case 0: gen_sxtb16(tmp);  break;
                        case 2: gen_sxtb(tmp);    break;
                        case 3: gen_sxth(tmp);    break;
                        case 4: gen_uxtb16(tmp);  break;
                        case 6: gen_uxtb(tmp);    break;
                        case 7: gen_uxth(tmp);    break;
                        default: goto illegal_op;
                        }
                        if (rn != 15) {
                            tmp2 = load_reg(s, rn);
                            if ((op1 & 3) == 0) {
                                gen_add16(tmp, tmp2);
                            } else {
                                tcg_gen_add_i32(tmp, tmp, tmp2);
                                dead_tmp(tmp2);
                            }
                        }
                        store_reg(s, rd, tmp);
                    } else if ((insn & 0x003f0f60) == 0x003f0f20) {
                        /* rev */
                        tmp = load_reg(s, rm);
                        if (insn & (1 << 22)) {
                            if (insn & (1 << 7)) {
                                gen_revsh(tmp);
                            } else {
                                ARCH(6T2);
                                gen_helper_rbit(tmp, tmp);
                            }
                        } else {
                            if (insn & (1 << 7))
                                gen_rev16(tmp);
                            else
                                tcg_gen_bswap32_i32(tmp, tmp);
                        }
                        store_reg(s, rd, tmp);
                    } else {
                        goto illegal_op;
                    }
                    break;
                case 2: /* Multiplies (Type 3).  */
                    tmp = load_reg(s, rm);
                    tmp2 = load_reg(s, rs);
                    if (insn & (1 << 20)) {
                        /* Signed multiply most significant [accumulate].  */
                        tmp64 = gen_muls_i64_i32(tmp, tmp2);
                        if (insn & (1 << 5))
                            tcg_gen_addi_i64(tmp64, tmp64, 0x80000000u);
                        tcg_gen_shri_i64(tmp64, tmp64, 32);
                        tmp = new_tmp();
                        tcg_gen_trunc_i64_i32(tmp, tmp64);
                        tcg_temp_free_i64(tmp64);
                        if (rd != 15) {
                            tmp2 = load_reg(s, rd);
                            if (insn & (1 << 6)) {
                                tcg_gen_sub_i32(tmp, tmp, tmp2);
                            } else {
                                tcg_gen_add_i32(tmp, tmp, tmp2);
                            }
                            dead_tmp(tmp2);
                        }
                        store_reg(s, rn, tmp);
                    } else {
                        if (insn & (1 << 5))
                            gen_swap_half(tmp2);
                        gen_smul_dual(tmp, tmp2);
                        /* This addition cannot overflow.  */
                        if (insn & (1 << 6)) {
                            tcg_gen_sub_i32(tmp, tmp, tmp2);
                        } else {
                            tcg_gen_add_i32(tmp, tmp, tmp2);
                        }
                        dead_tmp(tmp2);
                        if (insn & (1 << 22)) {
                            /* smlald, smlsld */
                            tmp64 = tcg_temp_new_i64();
                            tcg_gen_ext_i32_i64(tmp64, tmp);
                            dead_tmp(tmp);
                            gen_addq(s, tmp64, rd, rn);
                            gen_storeq_reg(s, rd, rn, tmp64);
                            tcg_temp_free_i64(tmp64);
                        } else {
                            /* smuad, smusd, smlad, smlsd */
                            if (rd != 15)
                              {
                                tmp2 = load_reg(s, rd);
                                gen_helper_add_setq(tmp, tmp, tmp2);
                                dead_tmp(tmp2);
                              }
                            store_reg(s, rn, tmp);
                        }
                    }
                    break;
                case 3:
                    op1 = ((insn >> 17) & 0x38) | ((insn >> 5) & 7);
                    switch (op1) {
                    case 0: /* Unsigned sum of absolute differences.  */
                        ARCH(6);
                        tmp = load_reg(s, rm);
                        tmp2 = load_reg(s, rs);
                        gen_helper_usad8(tmp, tmp, tmp2);
                        dead_tmp(tmp2);
                        if (rd != 15) {
                            tmp2 = load_reg(s, rd);
                            tcg_gen_add_i32(tmp, tmp, tmp2);
                            dead_tmp(tmp2);
                        }
                        store_reg(s, rn, tmp);
                        break;
                    case 0x20: case 0x24: case 0x28: case 0x2c:
                        /* Bitfield insert/clear.  */
                        ARCH(6T2);
                        shift = (insn >> 7) & 0x1f;
                        i = (insn >> 16) & 0x1f;
                        i = i + 1 - shift;
                        if (rm == 15) {
                            tmp = new_tmp();
                            tcg_gen_movi_i32(tmp, 0);
                        } else {
                            tmp = load_reg(s, rm);
                        }
                        if (i != 32) {
                            tmp2 = load_reg(s, rd);
                            gen_bfi(tmp, tmp2, tmp, shift, (1u << i) - 1);
                            dead_tmp(tmp2);
                        }
                        store_reg(s, rd, tmp);
                        break;
                    case 0x12: case 0x16: case 0x1a: case 0x1e: /* sbfx */
                    case 0x32: case 0x36: case 0x3a: case 0x3e: /* ubfx */
                        ARCH(6T2);
                        tmp = load_reg(s, rm);
                        shift = (insn >> 7) & 0x1f;
                        i = ((insn >> 16) & 0x1f) + 1;
                        if (shift + i > 32)
                            goto illegal_op;
                        if (i < 32) {
                            if (op1 & 0x20) {
                                gen_ubfx(tmp, shift, (1u << i) - 1);
                            } else {
                                gen_sbfx(tmp, shift, i);
                            }
                        }
                        store_reg(s, rd, tmp);
                        break;
                    default:
                        goto illegal_op;
                    }
                    break;
                }
                break;
            }
        do_ldst:
            /* Check for undefined extension instructions
             * per the ARM Bible IE:
             * xxxx 0111 1111 xxxx  xxxx xxxx 1111 xxxx
             */
            sh = (0xf << 20) | (0xf << 4);
            if (op1 == 0x7 && ((insn & sh) == sh))
            {
                goto illegal_op;
            }
            /* load/store byte/word */
            rn = (insn >> 16) & 0xf;
            rd = (insn >> 12) & 0xf;
            tmp2 = load_reg(s, rn);
            i = (IS_USER(s) || (insn & 0x01200000) == 0x00200000);
            if (insn & (1 << 24))
                gen_add_data_offset(s, insn, tmp2);
            if (insn & (1 << 20)) {
                /* load */
                if (insn & (1 << 22)) {
                    tmp = gen_ld8u(tmp2, i);
                } else {
                    tmp = gen_ld32(tmp2, i);
                }
            } else {
                /* store */
                tmp = load_reg(s, rd);
                if (insn & (1 << 22))
                    gen_st8(tmp, tmp2, i);
                else
                    gen_st32(tmp, tmp2, i);
            }
            if (!(insn & (1 << 24))) {
                gen_add_data_offset(s, insn, tmp2);
                store_reg(s, rn, tmp2);
            } else if (insn & (1 << 21)) {
                store_reg(s, rn, tmp2);
            } else {
                dead_tmp(tmp2);
            }
            if (insn & (1 << 20)) {
                /* Complete the load.  */
                if (rd == 15)
                    gen_bx(s, tmp);
                else
                    store_reg(s, rd, tmp);
            }
            break;
        case 0x08:
        case 0x09:
            {
                int j, n, user, loaded_base;
                TCGv loaded_var;
                /* load/store multiple words */
                /* XXX: store correct base if write back */
                user = 0;
                if (insn & (1 << 22)) {
                    if (IS_USER(s))
                        goto illegal_op; /* only usable in supervisor mode */

                    if ((insn & (1 << 15)) == 0)
                        user = 1;
                }
                rn = (insn >> 16) & 0xf;
                addr = load_reg(s, rn);

                /* compute total size */
                loaded_base = 0;
                TCGV_UNUSED(loaded_var);
                n = 0;
                for(i=0;i<16;i++) {
                    if (insn & (1 << i))
                        n++;
                }
                /* XXX: test invalid n == 0 case ? */
                if (insn & (1 << 23)) {
                    if (insn & (1 << 24)) {
                        /* pre increment */
                        tcg_gen_addi_i32(addr, addr, 4);
                    } else {
                        /* post increment */
                    }
                } else {
                    if (insn & (1 << 24)) {
                        /* pre decrement */
                        tcg_gen_addi_i32(addr, addr, -(n * 4));
                    } else {
                        /* post decrement */
                        if (n != 1)
                        tcg_gen_addi_i32(addr, addr, -((n - 1) * 4));
                    }
                }
                j = 0;
                for(i=0;i<16;i++) {
                    if (insn & (1 << i)) {
                        if (insn & (1 << 20)) {
                            /* load */
                            tmp = gen_ld32(addr, IS_USER(s));
                            if (i == 15) {
                                gen_bx(s, tmp);
                            } else if (user) {
                                tmp2 = tcg_const_i32(i);
                                gen_helper_set_user_reg(tmp2, tmp);
                                tcg_temp_free_i32(tmp2);
                                dead_tmp(tmp);
                            } else if (i == rn) {
                                loaded_var = tmp;
                                loaded_base = 1;
                            } else {
                                store_reg(s, i, tmp);
                            }
                        } else {
                            /* store */
                            if (i == 15) {
                                /* special case: r15 = PC + 8 */
                                val = (long)s->pc + 4;
                                tmp = new_tmp();
                                tcg_gen_movi_i32(tmp, val);
                            } else if (user) {
                                tmp = new_tmp();
                                tmp2 = tcg_const_i32(i);
                                gen_helper_get_user_reg(tmp, tmp2);
                                tcg_temp_free_i32(tmp2);
                            } else {
                                tmp = load_reg(s, i);
                            }
                            gen_st32(tmp, addr, IS_USER(s));
                        }
                        j++;
                        /* no need to add after the last transfer */
                        if (j != n)
                            tcg_gen_addi_i32(addr, addr, 4);
                    }
                }
                if (insn & (1 << 21)) {
                    /* write back */
                    if (insn & (1 << 23)) {
                        if (insn & (1 << 24)) {
                            /* pre increment */
                        } else {
                            /* post increment */
                            tcg_gen_addi_i32(addr, addr, 4);
                        }
                    } else {
                        if (insn & (1 << 24)) {
                            /* pre decrement */
                            if (n != 1)
                                tcg_gen_addi_i32(addr, addr, -((n - 1) * 4));
                        } else {
                            /* post decrement */
                            tcg_gen_addi_i32(addr, addr, -(n * 4));
                        }
                    }
                    store_reg(s, rn, addr);
                } else {
                    dead_tmp(addr);
                }
                if (loaded_base) {
                    store_reg(s, rn, loaded_var);
                }
                if ((insn & (1 << 22)) && !user) {
                    /* Restore CPSR from SPSR.  */
                    tmp = load_cpu_field(spsr);
                    gen_set_cpsr(tmp, 0xffffffff);
                    dead_tmp(tmp);
                    s->is_jmp = DISAS_UPDATE;
                }
            }
            break;
        case 0xa:
        case 0xb:
            {
                int32_t offset;

                /* branch (and link) */
                val = (int32_t)s->pc;
                if (insn & (1 << 24)) {
                    tmp = new_tmp();
                    tcg_gen_movi_i32(tmp, val);
                    store_reg(s, 14, tmp);
                }
                offset = (((int32_t)insn << 8) >> 8);
                val += (offset << 2) + 4;
                gen_jmp(s, val);
            }
            break;
        case 0xc:
        case 0xd:
        case 0xe:
            /* Coprocessor.  */
            if (disas_coproc_insn(env, s, insn))
                goto illegal_op;
            break;
        case 0xf:
            /* swi */
            gen_set_pc_im(s->pc);
            s->is_jmp = DISAS_SWI;
            break;
        default:
        illegal_op:
            gen_set_condexec(s);
            gen_set_pc_im(s->pc - 4);
            gen_exception(EXCP_UDEF);
            s->is_jmp = DISAS_JUMP;
            break;
        }
    }
}

void Arm::thread_process()
{
    // initialization

    // forever loop
    while (1)
    {
        if (this->thumb) {
            disas_thumb_insn();
        } else {
            disas_arm_insn();
        }

    }
}

void
Arm::irq_s_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    TLM_INT_SANITY(trans);

    // retrieve the required parameters
    uint32_t* ptr = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

    // check if it is a set or clear command
    if (*ptr)
    {
//        m_arm->irq_set();
        // notify the interrupt (after setting the interrupt in processor)
        m_interrupt.notify();
    }
    else
    {
//        m_arm->irq_clear();
    }

    // there was no error in the processing of the transaction
    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    return;
}

void
Arm::fiq_s_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    TLM_INT_SANITY(trans);

    // retrieve the required parameters
    uint32_t* ptr = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

    // check if it is a set or clear command
    if (*ptr)
    {
//        m_arm->fiq_set();
        // notify the interrupt (after setting the interrupt in arm)
        m_interrupt.notify();
    }
    else
    {
//        m_arm->fiq_clear();
    }

    // there was no error in the processing of the transaction
    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    return;
}

#endif
