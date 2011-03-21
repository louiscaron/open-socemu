#ifndef PHY_H_
#define PHY_H_

/// Physical control peripheral (PHY)

#include "Generic/Peripheral/Peripheral.h"

/// Registers definition
enum
{
    REG_PHY_CR0,
    REG_PHY_SR_OP_STATE,
    REG_PHY_SR_STATUS0,
    REG_PHY_STATUS,
    REG_PHY_STATUS_INT_EN,
    REG_PHY_DC_PC_CTX_IDX = 0x24/4,
    REG_PHY_DC_HOPST,
    REG_PHY_DC_PRTY_LO = 0x30/4,
    REG_PHY_DC_PRTY_HI,
    REG_PHY_DC_BTA_LO,
    REG_PHY_DC_BTA_HI,
    REG_PHY_DC_CLSDEV,
    REG_PHY_DC_IPA_LO,
    REG_PHY_DC_IPA_HI,
    REG_PHY_DC_PG_UAP,
    REG_PHY_DC_FHS_PYLD,
    REG_PHY_DC_N_IQ,
    REG_PHY_DC_IQ_TO,
    REG_PHY_DC_IQ_RESPTO,
    REG_PHY_DC_N_PG,
    REG_PHY_DC_PG_TO,
    REG_PHY_DC_PG_RESPTO,
    REG_PHY_DC_NEWCON_TO,
    REG_PHY_DC_PG_CKOFF,
    REG_PHY_DC_N_PCLK_OFT,
    REG_PHY_DC_N_CLK_OFT,
    REG_PHY_DC_N_CLK_WAKE_CNT,
    REG_PHY_DC_NCLK_CTL,
    REG_PHY_DC_NBTC_PCLK,
    REG_PHY_DC_NBTC_CLK,
    REG_PHY_DC_ACODE_DLY,
    REG_PHY_DC_CR_OP_ST_EN_CNT,
    REG_PHY_DC_RTX_START_CNT,
    REG_PHY_DC_PLL_EN_CNT,
    REG_PHY_FSM_INTRP_ENABLE,
    REG_PHY_DC_SMPL_CTL,
    REG_PHY_DC_SLOT_OFFSET,
    REG_PHY_DC_WIN_PKT0,
    REG_PHY_DC_WIN_PKT1,
    REG_PHY_DC_WIN_PKT2,
    REG_PHY_DC_WIN_TXCONFX,
    REG_PHY_TXCONFX_ST,
    REG_PHY_DC_ICOEX,
    REG_PHY_DC_ICOEX_FREQ0,
    REG_PHY_DC_ICOEX_FREQ1,
    REG_PHY_DC_ICOEX_FREQ2,
    REG_PHY_DC_ICOEX_FREQ3,
    REG_PHY_DC_BT_CLK_OFFSET,
    REG_PHY_DC_DIAC_LO,
    REG_PHY_DC_DIAC_HI,
    REG_PHY_DC_GIAC_LO,
    REG_PHY_DC_GIAC_HI,
    REG_PHY_DC_DUAL_SYNCH,
    REG_PHY_DC_WIN_PKT3,
    REG_PHY_DC_2CLB_ECI_LSW,
    REG_PHY_DC_2CLB_ECI_MSW,
    REG_PHY_DC_CAP_SYNCCNT,
    REG_PHY_DC_SCAN_PG_CFG,
    REG_PHY_DC_PCXCLKDIFF,
    REG_PHY_ENCRYPTION_KEY0 = 0x184/4,
    REG_PHY_ENCRYPTION_KEY1,
    REG_PHY_ENCRYPTION_KEY2,
    REG_PHY_ENCRYPTION_KEY3,
    REG_PHY_PC_IDX_PTR = 0x264/4,
    REG_PHY_PCX_CTL,
    REG_PHY_PCX_PCKOFF,
    REG_PHY_PCX_CKOFF,
    REG_PHY_PCX_TIMER_CTL,
    REG_PHY_PCX_TIMER_VAL,
    REG_PHY_PC_ACSCD_LO,
    REG_PHY_PC_ACSCD_HI,
    REG_PHY_PCX_UAP,
    REG_PHY_PCX_NAP,
    REG_PHY_PCX_PBTCLK,
    REG_PHY_PCX_BTCLK,
    REG_PHY_PCX_LAST_SYNCTR,
    REG_PHY_PCX_NAT_CKOFF,
    REG_PHY_PCX2_PBTCLK,
    REG_PHY_PCX2_BTCLK,
    REG_PHY_AMODB1 = 0x2b8/4,
    REG_PHY_AMODB2,
    REG_PHY_MOD_CALC_CTL,
    REG_PHY_A_IN_AMODB1,
    REG_PHY_B_IN_AMODB1,
    REG_PHY_A_IN_AMODB2,
    REG_PHY_B_IN_AMODB2,
    REG_PHY_PCX_ENC_BC = 0x300/4,
    REG_PHY_PCX_MC_CFG,
    REG_PHY_DC_MSS_PC_INDX = 0x330/4,
    REG_PHY_TX_POWER,
    REG_PHY_XCDX_HOLD_CTL,
    REG_PHY_TASK_IND = 0x370/4,
    REG_PHY_TASKX_CTL,
    REG_PHY_TASKX_SCH_TIME0,
    REG_PHY_WIMAX_CAP_PCLK,
    REG_PHY_TASKX_T_TO0,
    REG_PHY_TASKX_T_TO1,
    REG_PHY_TASK_MIN_T_TO0,
    REG_PHY_TASK_MIN_T_TO1,
    REG_PHY_TASK_RESULT0,
    REG_PHY_TASK_RESULT1,
    REG_PHY_PC0_HS_CAP_CLK,
    REG_PHY_PC1_HS_CAP_CLK,
    REG_PHY_PC2_HS_CAP_CLK,
    REG_PHY_PC3_HS_CAP_CLK,
    REG_PHY_ENH_CTL,
    REG_PHY_DI_TEST_CTL = 0x430/4,
    REG_PHY_DI_MOD_INDEX_CTL,
    REG_PHY_DI_RF_CTL0,
    REG_PHY_DI_RF_CTL1,
    REG_PHY_DI_TMODE_CTL,
    REG_PHY_DI_TMODE_RX_HF,
    REG_PHY_DI_TMODE_TX_HF,
    REG_PHY_DI_LCU_SUBSTATE0,
    REG_PHY_DI_LCU_SUBSTATE1,
    REG_PHY_DI_TX_PATTERN = 0x470/4,
    REG_PHY_DI_TX_PATTERN_SEL,
    REG_PHY_SCO_PC_CONTEXT,
    REG_PHY_DI_PHY_FPGA_VID,
    REG_PHY_DI_PHY_FPGA_48M_DCM,
    REG_PHY_SCOX_D_SCO = 0x4b0/4,
    REG_PHY_SCOX_T_SCO,
    REG_PHY_PCX_N_POLL,
    REG_PHY_PARK_AVAILABLE01,
    REG_PHY_PARK_CTL,
    REG_PHY_PARK_STATUS,
    REG_PHY_DC_SRI_IST_BIT = 0x510/4,
    REG_PHY_DC_SRI_DS_BIT,
    REG_PHY_DC_SRI_IST0,
    REG_PHY_DC_SRI_IST1,
    REG_PHY_DC_SRI_DS0,
    REG_PHY_DC_SRI_DS1,
    REG_PHY_DC_SRI_DS2,
    REG_PHY_DC_SRI_DS3,
    REG_PHY_DC_SRI_JTAG_ACCESS,
    REG_PHY_DC_SRI_RF_CTL0,
    REG_PHY_DC_SRI_RF_CTL1,
    REG_PHY_DC_SRI_RSSI_VAL,
    REG_PHY_DC_SRI_PWR_CTL,
    REG_PHY_DC_T_SYNTH_PU,
    REG_PHY_DC_T_SL_CTL,
    REG_PHY_DC_T_PA_RAMP,
    REG_PHY_DC_T_PA_DOWN,
    REG_PHY_DC_T_TX_PU,
    REG_PHY_DC_T_RX_PU,
    REG_PHY_DC_AVAILABLE01,
    REG_PHY_DC_SRI_SCHP0_SEL,
    REG_PHY_DC_SRI_SCHP1_SEL,
    REG_PHY_DC_SRI_SCHP2_SEL,
    REG_PHY_DC_SRI_SCHP3_SEL,
    REG_PHY_DC_AFH_CTL = 0x5b0/4,
    REG_PHY_DC_IND_D_PTR,
    REG_PHY_DC_HD_ACC_CFG,
    REG_PHY_DC_UP_OPCODE,
    REG_PHY_UP_REG_B,
    REG_PHY_UP_REG_C,
    REG_PHY_UP_REG_D,
    REG_PHY_DC_REG_A,
    REG_PHY_DC_REG_B,
    REG_PHY_DC_REG_C,
    REG_PHY_DC_REG_D,
    REG_PHY_DC_OPCODE0,
    REG_PHY_DC_OPCODE1,
    REG_PHY_DC_OPCODE2,
    REG_PHY_DC_OPCODE3,
    REG_PHY_DC_OPCODE4,
    REG_PHY_DC_OPCODE5,
    REG_PHY_DC_OPCODE6,
    REG_PHY_DC_OPCODE7,
    REG_PHY_DC_OPCODE8,
    REG_PHY_DC_OPCODE9,
    REG_PHY_DC_OPCODE10,
    REG_PHY_DC_OPCODE11,
    REG_PHY_DC_OPCODE12,
    REG_PHY_DC_OPCODE13,
    REG_PHY_DC_OPCODE14,
    REG_PHY_DC_OPCODE15,
    REG_PHY_DC_LOG_DATA0,
    REG_PHY_DC_LOG_DATA1,
    REG_PHY_DC_LOG_DATA2,
    REG_PHY_DC_LOG_DATA3,
    REG_PHY_DC_LOG_DATA4,
    REG_PHY_DC_FHOUT,
    REG_PHY_DC_FK_REMAP,
    REG_PHY_DC_MISC_CTL,
    REG_PHY_DC_X_CLK,
    REG_PHY_USED_CARRIERS_32T0,
    REG_PHY_USED_CARRIERS_64T0,
    REG_PHY_USED_CARRIERS_79T0,
    REG_PHY_USED_CARRIERS_32T1,
    REG_PHY_USED_CARRIERS_64T1,
    REG_PHY_USED_CARRIERS_79T1,
    REG_PHY_AFH_MEM_START = 0x800/4,
    REG_PHY_AFH_MEM_END = 0x940/4,
    REG_PHY_DC_SAVED_FHOUT,
    REG_PHY_NULL_DEF = 0x988/4,
    REG_PHY_POLL_DEF,
    REG_PHY_FHS_DEF,
    REG_PHY_DM1_DEF,
    REG_PHY_DH1_DEF,
    REG_PHY_DM3_DEF,
    REG_PHY_DH3_DEF,
    REG_PHY_DM5_DEF,
    REG_PHY_DH5_DEF,
    REG_PHY_DH1_2_DEF,
    REG_PHY_DH3_2_DEF,
    REG_PHY_DH5_2_DEF,
    REG_PHY_DH1_3_DEF,
    REG_PHY_DH3_3_DEF,
    REG_PHY_DH5_3_DEF,
    REG_PHY_AUX1_DEF,
    REG_PHY_HV1_DEF,
    REG_PHY_HV2_DEF,
    REG_PHY_HV3_DEF,
    REG_PHY_EV3_DEF,
    REG_PHY_EV4_DEF,
    REG_PHY_EV5_DEF,
    REG_PHY_DV_DEF,
    REG_PHY_EV3_2_DEF,
    REG_PHY_EV5_2_DEF,
    REG_PHY_EV3_3_DEF,
    REG_PHY_EV5_3_DEF,
    REG_PHY_PS_DEF,
    REG_PHY_RSV0_DEF,
    REG_PHY_RSV1_DEF,
    REG_PHY_PKT_RPL_DEF_2,
    REG_PHY_PKT_RPL_DEF_3,
    REG_PHY_PKT_RPL_DEF_4,
    REG_PHY_PKT_RPL_DEF_5,
    REG_PHY_PKT_RPL_DEF_6,
    REG_PHY_PKT_RPL_DEF_7,
    REG_PHY_PKT_RPL_DEF_8,
    REG_PHY_PKT_RPL_DEF_9,
    REG_PHY_PKT_RPL_DEF_10,
    REG_PHY_PKT_RPL_DEF_11,
    REG_PHY_PKT_RPL_DEF_12,
    REG_PHY_PKT_RPL_DEF_13,
    REG_PHY_PKT_RPL_DEF_14,
    REG_PHY_PKT_RPL_DEF_15,
    REG_PHY_PROG_BT_TRIGGER_CFG,
    REG_PHY_PROG_BT_INT0_CFG,
    REG_PHY_PROG_BT_INT1_CFG,
    REG_PHY_PROG_BT_INT2_CFG,
    REG_PHY_PROG_BT_INT3_CFG,
    REG_PHY_TX_PKT_INFO,
    REG_PHY_TX_PKT_PYLD_HDR,
    REG_PHY_TX_FHS_PKT_HDR,
    REG_PHY_TX_DFT_PKT_HDR,
    REG_PHY_TX_CORRUPT_CFG,
    REG_PHY_LOGICAL_CONN_INFO,
    REG_PHY_TXD_RXD_SCO_LENGTH,
    REG_PHY_TXD_RXD_CTL_CFG,
    REG_PHY_RXD_CHK_NOTX_FHS_CFG,
    REG_PHY_RXD_CHK_NOTX_ACL_CFG,
    REG_PHY_RXD_CHK_NOTX_SCO_CFG,
    REG_PHY_RXD_HDR_OK_CFG,
    REG_PHY_PKT_RX_1SLOT_MAX_CNT,
    REG_PHY_PKT_RX_3SLOT_MAX_CNT,
    REG_PHY_PKT_RX_5SLOT_MAX_CNT,
    REG_PHY_MULTI_S_TERM_CNT,
    REG_PHY_IPS_SYNCH_WIN,
    REG_PHY_SYNCH_WIN,
    REG_PHY_SYNCH_TRIGGER_OFFSET,
    REG_PHY_RTX_DMA_CTL,
    REG_PHY_PSK_SYNCH_WORD_LO,
    REG_PHY_PSK_SYNCH_WORD_HI,
    REG_PHY_PSK_PYLD_MOD_CFG,
    REG_PHY_PKT_HDR_STATUS,
    REG_PHY_PKT_LOG,
    REG_PHY_PKT_PICKY_CNT_TH,
    REG_PHY_PYLD_PICKY_ERR_CNT,
    REG_PHY_RTX_MEM_START = 0xb00/4,
    REG_PHY_COUNT
};

struct Phy : Peripheral<REG_PHY_COUNT>
{
    // Module has a thread
    SC_HAS_PROCESS(Phy);

    /// Constructor
    Phy(sc_core::sc_module_name name)
    : Peripheral<REG_PHY_COUNT>(name)
    {
        // initialize the registers content

        // create the module threads
        SC_THREAD(sri_process);
    }

private:
    /// Module threads
    void
    sri_process();

    /** Register read function
     * @param[in] offset Offset of the register to read
     * @return The value read
     */
    uint32_t
    reg_rd(uint32_t offset);

    /** Register write function
     * @param[in] offset Offset of the register to read
     * @param[in] offset Value to write in the register
     */
    void
    reg_wr(uint32_t offset, uint32_t value);

    /// Indicate if the current SRI access is a write
    bool m_sriwrite;

    /// Event used to wake up the SRI thread
    sc_core::sc_event m_srievent;
};

#endif /*PHY_H_*/