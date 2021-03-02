/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "comphy_a3700.h"

DECLARE_GLOBAL_DATA_PTR;

#define A3700_LANE_MAX_NUM	3

struct sgmii_phy_init_data_fix {
	u16 addr;
	u16 value;
};

/*
 * In PHY mux initialization, comphy_mux_init() takes it for granted that
 * lanes' phy select bits in selector base register are ordered by lane number;
 * but for a3700, lane1 phy select bit is before lane0 in selector base
 * register as below, so it requires the mapping from the nominal lane index
 * defined in FS to the actual lane index by PHY Selector register order.
 *
 * RD00183FCh (00000011h) - PHY Selector
 *      bit 0: PCIE_GBE0_SEL -PHY Lane 1 Mode Select, 0h: GbE0 1h: PCIe
 *      bit 4: USB_GBE1_SEL -PHY Lane 0 Mode Select, 0h: GbE1 1h: USB
 *      bit 8: USB_SATA_SEL -PHY Lane 2 Mode Select, 0h: SATA 1h: USB
 */
struct a3700_comphy_lane_mux_map {
	u32 lane_num;		/* the nominal lane index defined in FS  */
	u32 phy_select_num;	/* the actual lane index in PHY Selector Reg */
};

struct a3700_comphy_lane_mux_map lane_mux_map[A3700_LANE_MAX_NUM] = {
/* Lane 0 */ {0, 1},
/* Lane 1 */ {1, 0},
/* Lane 2 */ {2, 2}
};

struct comphy_mux_data a3700_comphy_mux_data[] = {
/* Lane 0 */ {5, {{COMPHY_TYPE_UNCONNECTED, 0x0}, {COMPHY_TYPE_SGMII1, 0x0},
			{COMPHY_TYPE_USB3, 0x1}, {COMPHY_TYPE_USB3_HOST0, 0x1},
			{COMPHY_TYPE_USB3_DEVICE, 0x1} } },
/* Lane 1 */ {3, {{COMPHY_TYPE_UNCONNECTED, 0x0}, {COMPHY_TYPE_SGMII0, 0x0},
			{COMPHY_TYPE_PEX0, 0x1} } },
/* Lane 2 */ {4, {{COMPHY_TYPE_UNCONNECTED, 0x0}, {COMPHY_TYPE_SATA0, 0x0},
			{COMPHY_TYPE_USB3_HOST0, 0x1},
			{COMPHY_TYPE_USB3_DEVICE, 0x1} } },
};

/* Changes to 40M1G25 mode data required for running 40M3G125 init mode */
static struct sgmii_phy_init_data_fix sgmii_phy_init_fix[] = {
	{0x005, 0x07CC}, {0x015, 0x0000}, {0x01B, 0x0000}, {0x01D, 0x0000},
	{0x01E, 0x0000}, {0x01F, 0x0000}, {0x020, 0x0000}, {0x021, 0x0030},
	{0x026, 0x0888}, {0x04D, 0x0152}, {0x04F, 0xA020}, {0x050, 0x07CC},
	{0x053, 0xE9CA}, {0x055, 0xBD97}, {0x071, 0x3015}, {0x076, 0x03AA},
	{0x07C, 0x0FDF}, {0x0C2, 0x3030}, {0x0C3, 0x8000}, {0x0E2, 0x5550},
	{0x0E3, 0x12A4}, {0x0E4, 0x7D00}, {0x0E6, 0x0C83}, {0x101, 0xFCC0},
	{0x104, 0x0C10}
};

/* 40M1G25 mode init data */
static u16 sgmii_phy_init[512] = {
	/* 0       1       2       3       4       5       6       7 */
	/*-----------------------------------------------------------*/
	/* 8       9       A       B       C       D       E       F */
	0x3110, 0xFD83, 0x6430, 0x412F, 0x82C0, 0x06FA, 0x4500, 0x6D26,	/* 00 */
	0xAFC0, 0x8000, 0xC000, 0x0000, 0x2000, 0x49CC, 0x0BC9, 0x2A52,	/* 08 */
	0x0BD2, 0x0CDE, 0x13D2, 0x0CE8, 0x1149, 0x10E0, 0x0000, 0x0000,	/* 10 */
	0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x4134, 0x0D2D, 0xFFFF,	/* 18 */
	0xFFE0, 0x4030, 0x1016, 0x0030, 0x0000, 0x0800, 0x0866, 0x0000,	/* 20 */
	0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,	/* 28 */
	0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 30 */
	0x0000, 0x0000, 0x000F, 0x6A62, 0x1988, 0x3100, 0x3100, 0x3100,	/* 38 */
	0x3100, 0xA708, 0x2430, 0x0830, 0x1030, 0x4610, 0xFF00, 0xFF00,	/* 40 */
	0x0060, 0x1000, 0x0400, 0x0040, 0x00F0, 0x0155, 0x1100, 0xA02A,	/* 48 */
	0x06FA, 0x0080, 0xB008, 0xE3ED, 0x5002, 0xB592, 0x7A80, 0x0001,	/* 50 */
	0x020A, 0x8820, 0x6014, 0x8054, 0xACAA, 0xFC88, 0x2A02, 0x45CF,	/* 58 */
	0x000F, 0x1817, 0x2860, 0x064F, 0x0000, 0x0204, 0x1800, 0x6000,	/* 60 */
	0x810F, 0x4F23, 0x4000, 0x4498, 0x0850, 0x0000, 0x000E, 0x1002,	/* 68 */
	0x9D3A, 0x3009, 0xD066, 0x0491, 0x0001, 0x6AB0, 0x0399, 0x3780,	/* 70 */
	0x0040, 0x5AC0, 0x4A80, 0x0000, 0x01DF, 0x0000, 0x0007, 0x0000,	/* 78 */
	0x2D54, 0x00A1, 0x4000, 0x0100, 0xA20A, 0x0000, 0x0000, 0x0000,	/* 80 */
	0x0000, 0x0000, 0x0000, 0x7400, 0x0E81, 0x1000, 0x1242, 0x0210,	/* 88 */
	0x80DF, 0x0F1F, 0x2F3F, 0x4F5F, 0x6F7F, 0x0F1F, 0x2F3F, 0x4F5F,	/* 90 */
	0x6F7F, 0x4BAD, 0x0000, 0x0000, 0x0800, 0x0000, 0x2400, 0xB651,	/* 98 */
	0xC9E0, 0x4247, 0x0A24, 0x0000, 0xAF19, 0x1004, 0x0000, 0x0000,	/* A0 */
	0x0000, 0x0013, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* B0 */
	0x0000, 0x0000, 0x0000, 0x0060, 0x0000, 0x0000, 0x0000, 0x0000,	/* B8 */
	0x0000, 0x0000, 0x3010, 0xFA00, 0x0000, 0x0000, 0x0000, 0x0003,	/* C0 */
	0x1618, 0x8200, 0x8000, 0x0400, 0x050F, 0x0000, 0x0000, 0x0000,	/* C8 */
	0x4C93, 0x0000, 0x1000, 0x1120, 0x0010, 0x1242, 0x1242, 0x1E00,	/* D0 */
	0x0000, 0x0000, 0x0000, 0x00F8, 0x0000, 0x0041, 0x0800, 0x0000,	/* D8 */
	0x82A0, 0x572E, 0x2490, 0x14A9, 0x4E00, 0x0000, 0x0803, 0x0541,	/* E0 */
	0x0C15, 0x0000, 0x0000, 0x0400, 0x2626, 0x0000, 0x0000, 0x4200,	/* E8 */
	0x0000, 0xAA55, 0x1020, 0x0000, 0x0000, 0x5010, 0x0000, 0x0000,	/* F0 */
	0x0000, 0x0000, 0x5000, 0x0000, 0x0000, 0x0000, 0x02F2, 0x0000,	/* F8 */
	0x101F, 0xFDC0, 0x4000, 0x8010, 0x0110, 0x0006, 0x0000, 0x0000,	/*100 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*108 */
	0x04CF, 0x0000, 0x04CF, 0x0000, 0x04CF, 0x0000, 0x04C6, 0x0000,	/*110 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*118 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*120 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*128 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*130 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*138 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*140 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*148 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*150 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*158 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*160 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*168 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*170 */
	0x0000, 0x0000, 0x0000, 0x00F0, 0x08A2, 0x3112, 0x0A14, 0x0000,	/*178 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*180 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*188 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*190 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*198 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1F0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000	/*1F8 */
};

/*
 * comphy_poll_reg
 *
 * return: 1 on success, 0 on timeout
 */
static u32 comphy_poll_reg(void *addr, u32 val, u32 mask, u32 timeout,
			   u8 op_type)
{
	u32 rval = 0xDEAD;

	for (; timeout > 0; timeout--) {
		if (op_type == POLL_16B_REG)
			rval = readw(addr);	/* 16 bit */
		else
			rval = readl(addr) ;	/* 32 bit */

		if ((rval & mask) == val)
			return 1;

		udelay(10000);
	}

	debug("Time out waiting (%p = %#010x)\n", addr, rval);
	return 0;
}

/*
 * usb3_reg_set_indirect
 *
 * return: void
 */
static inline void usb3_reg_set_indirect(u32 reg_offset, u16 data, u16 mask)
{
	/*
	 * When Lane 2 PHY is for USB3, access the PHY registers
	 * through indirect Address and Data registers INDIR_ACC_PHY_ADDR
	 * (RD00E0178h [31:0]) and INDIR_ACC_PHY_DATA (RD00E017Ch [31:0])
	 * within the SATA Host Controller registers, Lane 2 base register
	 * offset is 0x200
	 */
	reg_set((void __iomem *)rh_vsreg_addr, reg_offset +
		USB3PHY_LANE2_REG_BASE_OFFSET, 0xFFFFFFFF);
	reg_set((void __iomem *)rh_vsreg_data, data, mask);
	return;
}

/*
 * sata_reg_set_indirect
 *
 * return: void
 */
static inline void sata_reg_set_indirect(u32 reg_offset, u16 data, u16 mask)
{
	reg_set((void __iomem *)rh_vsreg_addr, reg_offset, 0xFFFFFFFF);
	reg_set((void __iomem *)rh_vsreg_data, data, mask);
	return;
}

/*
 * comphy_pcie_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_pcie_power_up(u32 speed, u32 invert)
{
	int	ret;

	debug_enter();

	/*
	 * 1. Enable max PLL.
	 */
	reg_set16((void __iomem *)LANE_CFG1_ADDR(PCIE),
		  bf_use_max_pll_rate, 0);

	/*
	 * 2. Select 20 bit SERDES interface.
	 */
	reg_set16((void __iomem *)GLOB_CLK_SRC_LO_ADDR(PCIE),
		  bf_cfg_sel_20b, 0);

	/*
	 * 3. Force to use reg setting for PCIe mode
	 */
	reg_set16((void __iomem *)MISC_REG1_ADDR(PCIE),
		  bf_sel_bits_pcie_force, 0);

	/*
	 * 4. Change RX wait
	 */
	reg_set16((void __iomem *)PWR_MGM_TIM1_ADDR(PCIE), 0x10C, 0xFFFF);

	/*
	 * 5. Enable idle sync
	 */
	reg_set16((void __iomem *)UNIT_CTRL_ADDR(PCIE),
		  0x60 | rb_idle_sync_en, 0xFFFF);

	/*
	 * 6. Enable the output of 100M/125M/500M clock
	 */
	reg_set16((void __iomem *)MISC_REG0_ADDR(PCIE),
		  0xA00D | rb_clk500m_en | rb_clk100m_125m_en, 0xFFFF);

	/*
	 * 7. Enable TX
	 */
	reg_set((void __iomem *)PHY_REF_CLK_ADDR, 0x1342, 0xFFFFFFFF);

	/*
	 * 8. Check crystal jumper setting and program the Power and PLL
	 *    Control accordingly
	 */
	if (get_ref_clk() == 40) {
		reg_set16((void __iomem *)PWR_PLL_CTRL_ADDR(PCIE),
			  0xFC63, 0xFFFF); /* 40 MHz */
	} else {
		reg_set16((void __iomem *)PWR_PLL_CTRL_ADDR(PCIE),
			  0xFC62, 0xFFFF); /* 25 MHz */
	}

	/*
	 * 9. Override Speed_PLL value and use MAC PLL
	 */
	reg_set16((void __iomem *)KVCO_CAL_CTRL_ADDR(PCIE),
		  0x0040 | rb_use_max_pll_rate, 0xFFFF);

	/*
	 * 10. Check the Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT) {
		reg_set16((void __iomem *)SYNC_PATTERN_ADDR(PCIE),
			  phy_txd_inv, 0);
	}

	if (invert & COMPHY_POLARITY_RXD_INVERT) {
		reg_set16((void __iomem *)SYNC_PATTERN_ADDR(PCIE),
			  phy_rxd_inv, 0);
	}

	/*
	 * 11. Release SW reset
	 */
	reg_set16((void __iomem *)GLOB_PHY_CTRL0_ADDR(PCIE),
		  rb_mode_core_clk_freq_sel | rb_mode_pipe_width_32,
		  bf_soft_rst | bf_mode_refdiv);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert PCLK enabled */
	ret = comphy_poll_reg((void *)LANE_STAT1_ADDR(PCIE),	/* address */
			      rb_txdclk_pclk_en,		/* value */
			      rb_txdclk_pclk_en,		/* mask */
			      PLL_LOCK_TIMEOUT,			/* timeout */
			      POLL_16B_REG);			/* 16bit */
	if (ret == 0)
		printf("Failed to lock PCIe PLL\n");

	debug_exit();

	/* Return the status of the PLL */
	return ret;
}

/*
 * comphy_sata_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_sata_power_up(u32 invert)
{
	int	ret;
	u32	data = 0;

	debug_enter();

	/*
	 * 0. Check the Polarity invert bits
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		data |= bs_txd_inv;

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		data |= bs_rxd_inv;
	sata_reg_set_indirect(vphy_sync_pattern_reg, data,
			      bs_txd_inv | bs_rxd_inv);

	/*
	 * 1. Select 40-bit data width width
	 */
	sata_reg_set_indirect(vphy_loopback_reg0, 0x800, bs_phyintf_40bit);

	/*
	 * 2. Select reference clock and PHY mode (SATA)
	 */
	if (get_ref_clk() == 40) {
		sata_reg_set_indirect(vphy_power_reg0,
				      0x3, 0x00FF); /* 40 MHz */
	} else {
		sata_reg_set_indirect(vphy_power_reg0,
				      0x1, 0x00FF); /* 25 MHz */
	}

	/*
	 * 3. Use maximum PLL rate (no power save)
	 */
	sata_reg_set_indirect(vphy_calctl_reg, bs_max_pll_rate,
			      bs_max_pll_rate);

	/*
	 * 4. Reset reserved bit (??)
	 */
	sata_reg_set_indirect(vphy_reserve_reg, 0, bs_phyctrl_frm_pin);

	/*
	 * 5. Set vendor-specific configuration (??)
	 */
	reg_set((void __iomem *)rh_vs0_a, vsata_ctrl_reg, 0xFFFFFFFF);
	reg_set((void __iomem *)rh_vs0_d, bs_phy_pu_pll, bs_phy_pu_pll);

	/* Wait for > 55 us to allow PLL be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert SATA PLL enabled */
	reg_set((void __iomem *)rh_vsreg_addr, vphy_loopback_reg0, 0xFFFFFFFF);
	ret = comphy_poll_reg((void *)rh_vsreg_data,	/* address */
			      bs_pll_ready_tx,		/* value */
			      bs_pll_ready_tx,		/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to lock SATA PLL\n");

	debug_exit();

	return ret;
}

/*
 * usb_phy_reg_set_direct
 *
 * return: void
 */
static void usb_phy_reg_set_direct(u32 reg_offset, u16 data, u16 mask)
{
	/*
	 * When Lane 0 PHY is for USB3, access the PHY registers directly
	 */
	reg_set16((void __iomem *)(reg_offset * PHY_SHFT(USB3)
		      + PHY_BASE(USB3)), data, mask);

	return;
}

/*
 * comphy_usb3_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb3_power_up(u32 speed, u32 invert,
				bool indirect_reg_access)
{
	int	ret;
	void	(*fp_usb3_phy_reg_set)(u32, u16, u16) = usb_phy_reg_set_direct;

	debug_enter();

	if (indirect_reg_access)
		fp_usb3_phy_reg_set = usb3_reg_set_indirect;

	/*
	 * 1. Power up OTG module
	 */
	reg_set((void __iomem *)USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

	/*
	 * 2. Set counter for 100us pulse in USB3 Host and Device
	 * restore default burst size limit (Reference Clock 31:24)
	 */
	reg_set((void __iomem *)USB3_CTRPUL_VAL_REG,
		0x8 << 24, rb_usb3_ctr_100ns);


	/* 0xd005c300 = 0x1001 */
	/* set PRD_TXDEEMPH (3.5db de-emph) */
	fp_usb3_phy_reg_set(PHY_REG_LANE_CFG0_ADDR, 0x1, 0xFF);

	/*
	 * Set BIT0: enable transmitter in high impedance mode
	 * Set BIT[3:4]: delay 2 clock cycles for HiZ off latency
	 * Set BIT6: Tx detect Rx at HiZ mode
	 * Unset BIT15: set to 0 to set USB3 De-emphasize level to -3.5db
	 *              together with bit 0 of COMPHY_REG_LANE_CFG0_ADDR
	 *		register
	 */
	fp_usb3_phy_reg_set(PHY_REG_LANE_CFG1_ADDR,
			    TX_DET_RX_MODE | GEN2_TX_DATA_DLY_DEFT |
			    TX_ELEC_IDLE_MODE_EN, PRD_TXDEEMPH1_MASK |
			    TX_DET_RX_MODE | GEN2_TX_DATA_DLY_MASK |
			    TX_ELEC_IDLE_MODE_EN);


	/* 0xd005c310 = 0x93: set Spread Spectrum Clock Enabled  */
	fp_usb3_phy_reg_set(PHY_REG_LANE_CFG4_ADDR,
			    bf_spread_spectrum_clock_en, 0x80);

	/*
	 * set Override Margining Controls From the MAC: Use margining signals
	 * from lane configuration
	 */
	fp_usb3_phy_reg_set(PHY_REG_TEST_MODE_CTRL_ADDR,
			    rb_mode_margin_override, 0xFFFF);

	/* set Lane-to-Lane Bundle Clock Sampling Period = per PCLK cycles */
	/* set Mode Clock Source = PCLK is generated from REFCLK */
	fp_usb3_phy_reg_set(PHY_REG_GLOB_CLK_SRC_LO_ADDR, 0x0, 0xFF);

	/* set G2 Spread Spectrum Clock Amplitude at 4K */
	fp_usb3_phy_reg_set(PHY_REG_GEN2_SETTINGS_2, g2_tx_ssc_amp,
			    0xF000);

	/*
	 * unset G3 Spread Spectrum Clock Amplitude & set G3 TX and RX Register
	 * Master Current Select
	 */
	fp_usb3_phy_reg_set(PHY_REG_GEN2_SETTINGS_3, 0x0, 0xFFFF);

	/*
	 * 3. Check crystal jumper setting and program the Power and PLL
	 * Control accordingly
	 * 4. Change RX wait
	 */
	if (get_ref_clk() == 40) {
		/* 40 MHz */
		fp_usb3_phy_reg_set(PHY_PWR_PLL_CTRL_ADDR,
				    PHY_PWR_PLL_CTRL_ADDR_VAL |
				    rf_ref_clock_speed_40m,
				    PHY_PWR_PLL_CTRL_ADDR_VAL |
				    rf_ref_freq_sel_mask);
		fp_usb3_phy_reg_set(PHY_REG_PWR_MGM_TIM1_ADDR,
				    CFG_PM_RXDEN_WAIT_1_UNIT |
				    CFG_PM_RXDLOZ_WAIT_12_UNIT,
				    0xFFFF);
	} else {
		/* 25 MHz */
		fp_usb3_phy_reg_set(PHY_PWR_PLL_CTRL_ADDR,
				    PHY_PWR_PLL_CTRL_ADDR_VAL |
				    usb3_ref_clock_speed_25m,
				    PHY_PWR_PLL_CTRL_ADDR_VAL |
				    rf_ref_freq_sel_mask);
		fp_usb3_phy_reg_set(PHY_REG_PWR_MGM_TIM1_ADDR,
				    CFG_PM_RXDEN_WAIT_1_UNIT |
				    CFG_PM_RXDLOZ_WAIT_7_UNIT,
				    0xFFFF);
	}

	/*
	 * 5. Enable idle sync
	 */
	fp_usb3_phy_reg_set(PHY_REG_UNIT_CTRL_ADDR, 0x60 | rb_idle_sync_en,
			    0xFFFF);

	/*
	 * 6. Enable the output of 500M clock
	 */
	fp_usb3_phy_reg_set(PHY_MISC_REG0_ADDR, 0xA00D | rb_clk500m_en, 0xFFFF);

	/*
	 * 7. Set 20-bit data width
	 */
	fp_usb3_phy_reg_set(PHY_DIG_LB_EN_ADDR, 0x0400, 0xFFFF);

	/*
	 * 8. Override Speed_PLL value and use MAC PLL
	 */
	fp_usb3_phy_reg_set(PHY_REG_KVCO_CAL_CTRL_ADDR,
			    0x0040 | rb_use_max_pll_rate, 0xFFFF);

	/*
	 * 9. Check the Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT) {
		fp_usb3_phy_reg_set(PHY_SYNC_PATTERN_ADDR, phy_txd_inv,
				    phy_txd_inv);
	}

	if (invert & COMPHY_POLARITY_RXD_INVERT) {
		fp_usb3_phy_reg_set(PHY_SYNC_PATTERN_ADDR, phy_rxd_inv,
				    phy_rxd_inv);
	}

	/* 10. Set max speed generation to USB3.0 5Gbps */
	fp_usb3_phy_reg_set(PHY_SYNC_MASK_GEN_REG, PHY_GEN_USB3_5G,
			    PHY_GEN_MAX_MASK);

	/* 11. Set capacitor value for FFE gain peaking to 0xF */
	fp_usb3_phy_reg_set(PHY_REG_GEN3_SETTINGS_3, PHY_GEN_FFE_CAP_SEL_VALUE,
			    PHY_GEN_FFE_CAP_SEL_MASK);

	/*
	 * 12. Release SW reset
	 */
	fp_usb3_phy_reg_set(PHY_REG_GLOB_PHY_CTRL0_ADDR,
			    rb_mode_core_clk_freq_sel | rb_mode_pipe_width_32 |
			    0x20, 0xFFFF);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert PCLK enabled */
	if (indirect_reg_access) {
		reg_set((void __iomem *)rh_vsreg_addr,
			PHY_REG_LANE_STAT1_ADDR + USB3PHY_LANE2_REG_BASE_OFFSET,
			0xFFFFFFFF);
		ret = comphy_poll_reg((void *)rh_vsreg_data,	/* address */
				      rb_txdclk_pclk_en,	/* value */
				      rb_txdclk_pclk_en,	/* mask */
				      PLL_LOCK_TIMEOUT,	/* timeout */
				      POLL_32B_REG);	/* 32bit */
	} else {
		ret = comphy_poll_reg((void *)LANE_STAT1_ADDR(USB3),
				/* address */
			      rb_txdclk_pclk_en,	/* value */
			      rb_txdclk_pclk_en,	/* mask */
			      PLL_LOCK_TIMEOUT,	/* timeout */
			      POLL_16B_REG);	/* 16bit */
	}
	if (ret == 0)
		printf("Failed to lock USB3 PLL\n");

	/* No matter host mode and device mode, it works with Hard ID detection
	 * Unset DP and DM pulldown for USB2 Device mode
	 */
	reg_set((void __iomem *)USB2_OTG_PHY_CTRL_ADDR, 0x0,
		rb_usb2_dp_pulldn_dev_mode | rb_usb2_dm_pulldn_dev_mode);

	/* Disbale VBus interrupt which will be enable again in kernel */
	reg_set((void __iomem *)USB3_TOP_INT_ENABLE_REG, 0x0, vbus_int_enable);

	/* Clear VBus interrupt to prepare a clean state for kernel */
	reg_set((void __iomem *)USB3_TOP_INT_STATUS_REG,
		vbus_int_state, vbus_int_state);

	debug_exit();

	return ret;
}

/*
 * comphy_usb2_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb2_power_up(u8 usb32)
{
	int	ret;

	debug_enter();

	if (usb32 != 0 && usb32 != 1) {
		printf("invalid usb32 value: (%d), should be either 0 or 1\n",
		       usb32);
		debug_exit();
		return 0;
	}

	/*
	 * 0. Setup PLL. 40MHz clock uses defaults.
	 *    See "PLL Settings for Typical REFCLK" table
	 */
	if (get_ref_clk() == 25) {
		reg_set((void __iomem *)USB2_PHY_BASE(usb32),
			5 | (96 << 16), 0x3F | (0xFF << 16) | (0x3 << 28));
	}

	/*
	 * 1. PHY pull up and disable USB2 suspend
	 */
	reg_set((void __iomem *)USB2_PHY_CTRL_ADDR(usb32),
		RB_USB2PHY_SUSPM(usb32) | RB_USB2PHY_PU(usb32), 0);

	if (usb32 != 0) {
		/*
		 * 2. Power up OTG module
		 */
		reg_set((void __iomem *)USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

		/*
		 * 3. Configure PHY charger detection
		 */
		reg_set((void __iomem *)USB2_PHY_CHRGR_DET_ADDR, 0,
			rb_cdp_en | rb_dcp_en | rb_pd_en | rb_cdp_dm_auto |
			rb_enswitch_dp | rb_enswitch_dm | rb_pu_chrg_dtc);
	}

	/* Assert PLL calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_pllcal_done,	/* value */
			      rb_usb2phy_pllcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 PLL calibration\n");

	/* Assert impedance calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_impcal_done,	/* value */
			      rb_usb2phy_impcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 impedance calibration\n");

	/* Assert squetch calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_RX_CHAN_CTRL1_ADDR(usb32),
			      rb_usb2phy_sqcal_done,	/* value */
			      rb_usb2phy_sqcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 unknown calibration\n");

	/* Assert PLL is ready */
	ret = comphy_poll_reg((void *)USB2_PHY_PLL_CTRL0_ADDR(usb32),
			      rb_usb2phy_pll_ready,		/* value */
			      rb_usb2phy_pll_ready,		/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */

	if (ret == 0)
		printf("Failed to lock USB2 PLL\n");

	if (usb32) {
		/*
		 * Disbale VBus interrupt which will be
		 * enable again in kernel
		 */
		reg_set((void __iomem *)USB3_TOP_INT_ENABLE_REG, 0x0,
			vbus_int_enable);
		/* Clear VBus interrupt to prepare a clean state for kernel */
		reg_set((void __iomem *)USB3_TOP_INT_STATUS_REG,
			vbus_int_state, vbus_int_state);
	}

	debug_exit();

	return ret;
}

/*
 * comphy_emmc_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_emmc_power_up(void)
{
	debug_enter();

	/*
	 * 1. Bus power ON, Bus voltage 1.8V
	 */
	reg_set((void __iomem *)SDIO_HOST_CTRL1_ADDR, 0xB00, 0xF00);

	/*
	 * 2. Set FIFO parameters
	 */
	reg_set((void __iomem *)SDIO_SDHC_FIFO_ADDR, 0x315, 0xFFFFFFFF);

	/*
	 * 3. Set Capabilities 1_2
	 */
	reg_set((void __iomem *)SDIO_CAP_12_ADDR, 0x25FAC8B2, 0xFFFFFFFF);

	/*
	 * 4. Set Endian
	 */
	reg_set((void __iomem *)SDIO_ENDIAN_ADDR, 0x00c00000, 0);

	/*
	 * 4. Init PHY
	 */
	reg_set((void __iomem *)SDIO_PHY_TIMING_ADDR, 0x80000000, 0x80000000);
	reg_set((void __iomem *)SDIO_PHY_PAD_CTRL0_ADDR, 0x50000000,
		0xF0000000);

	/*
	 * 5. DLL reset
	 */
	reg_set((void __iomem *)SDIO_DLL_RST_ADDR, 0xFFFEFFFF, 0);
	reg_set((void __iomem *)SDIO_DLL_RST_ADDR, 0x00010000, 0);

	debug_exit();

	return 1;
}

/*
 * comphy_sgmii_power_up
 *
 * return:
 */
static void comphy_sgmii_phy_init(u32 lane, u32 speed)
{
	const int fix_arr_sz = ARRAY_SIZE(sgmii_phy_init_fix);
	int addr, fix_idx;
	u16 val;

	fix_idx = 0;
	for (addr = 0; addr < 512; addr++) {
		/*
		 * All PHY register values are defined in full for 3.125Gbps
		 * SERDES speed. The values required for 1.25 Gbps are almost
		 * the same and only few registers should be "fixed" in
		 * comparison to 3.125 Gbps values. These register values are
		 * stored in "sgmii_phy_init_fix" array.
		 */
		if ((speed != COMPHY_SPEED_1_25G) &&
		    (sgmii_phy_init_fix[fix_idx].addr == addr)) {
			/* Use new value */
			val = sgmii_phy_init_fix[fix_idx].value;
			if (fix_idx < fix_arr_sz)
				fix_idx++;
		} else {
			val = sgmii_phy_init[addr];
		}

		phy_write16(lane, addr, val, 0xFFFF);
	}
}

/*
 * comphy_sgmii_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_sgmii_power_up(u32 lane, u32 speed, u32 invert)
{
	int	ret;

	debug_enter();

	/*
	 * 1. Configure PHY to SATA/SAS mode by setting pin PIN_PIPE_SEL=0
	 */
	reg_set((void __iomem *)COMPHY_SEL_ADDR, 0, rf_compy_select(lane));

	/*
	 * 2. Reset PHY by setting PHY input port PIN_RESET=1.
	 * 3. Set PHY input port PIN_TX_IDLE=1, PIN_PU_IVREF=1 to keep
	 *    PHY TXP/TXN output to idle state during PHY initialization
	 * 4. Set PHY input port PIN_PU_PLL=0, PIN_PU_RX=0, PIN_PU_TX=0.
	 */
	reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
		rb_pin_reset_comphy | rb_pin_tx_idle | rb_pin_pu_iveref,
		rb_pin_reset_core | rb_pin_pu_pll |
		rb_pin_pu_rx | rb_pin_pu_tx);

	/*
	 * 5. Release reset to the PHY by setting PIN_RESET=0.
	 */
	reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
		0, rb_pin_reset_comphy);

	/*
	 * 7. Set PIN_PHY_GEN_TX[3:0] and PIN_PHY_GEN_RX[3:0] to decide
	 *    COMPHY bit rate
	 */
	if (speed == COMPHY_SPEED_3_125G) { /* 3.125 GHz */
		reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
			(0x8 << rf_gen_rx_sel_shift) |
			(0x8 << rf_gen_tx_sel_shift),
			rf_gen_rx_select | rf_gen_tx_select);

	} else if (speed == COMPHY_SPEED_1_25G) { /* 1.25 GHz */
		reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
			(0x6 << rf_gen_rx_sel_shift) |
			(0x6 << rf_gen_tx_sel_shift),
			rf_gen_rx_select | rf_gen_tx_select);
	} else {
		printf("Unsupported COMPHY speed!\n");
		return 0;
	}

	/*
	 * 8. Wait 1mS for bandgap and reference clocks to stabilize;
	 *    then start SW programming.
	 */
	mdelay(10);

	/* 9. Program COMPHY register PHY_MODE */
	phy_write16(lane, PHY_PWR_PLL_CTRL_ADDR,
		    PHY_MODE_SGMII, rf_phy_mode_mask);

	/*
	 * 10. Set COMPHY register REFCLK_SEL to select the correct REFCLK
	 *     source
	 */
	phy_write16(lane, PHY_MISC_REG0_ADDR, 0, rb_ref_clk_sel);

	/*
	 * 11. Set correct reference clock frequency in COMPHY register
	 *     REF_FREF_SEL.
	 */
	if (get_ref_clk() == 40) {
		phy_write16(lane, PHY_PWR_PLL_CTRL_ADDR,
			    0x4 << rf_ref_freq_sel_shift, rf_ref_freq_sel_mask);
	} else {
		/* 25MHz */
		phy_write16(lane, PHY_PWR_PLL_CTRL_ADDR,
			    0x1 << rf_ref_freq_sel_shift, rf_ref_freq_sel_mask);
	}

	/* 12. Program COMPHY register PHY_GEN_MAX[1:0] */
	/*
	 * This step is mentioned in the flow received from verification team.
	 * However the PHY_GEN_MAX value is only meaningful for other
	 * interfaces (not SGMII). For instance, it selects SATA speed
	 * 1.5/3/6 Gbps or PCIe speed  2.5/5 Gbps
	 */

	/*
	 * 13. Program COMPHY register SEL_BITS to set correct parallel data
	 *     bus width
	 */
	/* 10bit */
	phy_write16(lane, PHY_DIG_LB_EN_ADDR, 0, rf_data_width_mask);

	/*
	 * 14. As long as DFE function needs to be enabled in any mode,
	 *     COMPHY register DFE_UPDATE_EN[5:0] shall be programmed to 0x3F
	 *     for real chip during COMPHY power on.
	 */
	/*
	 * The step 14 exists (and empty) in the original initialization flow
	 * obtained from the verification team. According to the functional
	 * specification DFE_UPDATE_EN already has the default value 0x3F
	 */

	/*
	 * 15. Program COMPHY GEN registers.
	 *     These registers should be programmed based on the lab testing
	 *     result to achieve optimal performance. Please contact the CEA
	 *     group to get the related GEN table during real chip bring-up.
	 *     We only requred to run though the entire registers programming
	 *     flow defined by "comphy_sgmii_phy_init" when the REF clock is
	 *     40 MHz. For REF clock 25 MHz the default values stored in PHY
	 *     registers are OK.
	 */
	debug("Running C-DPI phy init %s mode\n",
	      speed == COMPHY_SPEED_3_125G ? "2G5" : "1G");
	if (get_ref_clk() == 40)
		comphy_sgmii_phy_init(lane, speed);

	/*
	 * 16. [Simulation Only] should not be used for real chip.
	 *     By pass power up calibration by programming EXT_FORCE_CAL_DONE
	 *     (R02h[9]) to 1 to shorten COMPHY simulation time.
	 */
	/*
	 * 17. [Simulation Only: should not be used for real chip]
	 *     Program COMPHY register FAST_DFE_TIMER_EN=1 to shorten RX
	 *     training simulation time.
	 */

	/*
	 * 18. Check the PHY Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		phy_write16(lane, PHY_SYNC_PATTERN_ADDR, phy_txd_inv, 0);

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		phy_write16(lane, PHY_SYNC_PATTERN_ADDR, phy_rxd_inv, 0);

	/*
	 * 19. Set PHY input ports PIN_PU_PLL, PIN_PU_TX and PIN_PU_RX to 1
	 *     to start PHY power up sequence. All the PHY register
	 *     programming should be done before PIN_PU_PLL=1. There should be
	 *     no register programming for normal PHY operation from this point.
	 */
	reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
		rb_pin_pu_pll | rb_pin_pu_rx | rb_pin_pu_tx,
		rb_pin_pu_pll | rb_pin_pu_rx | rb_pin_pu_tx);

	/*
	 * 20. Wait for PHY power up sequence to finish by checking output ports
	 *     PIN_PLL_READY_TX=1 and PIN_PLL_READY_RX=1.
	 */
	ret = comphy_poll_reg((void *)COMPHY_PHY_STAT1_ADDR(lane), /* address */
			      rb_pll_ready_tx | rb_pll_ready_rx, /* value */
			      rb_pll_ready_tx | rb_pll_ready_rx, /* mask */
			      PLL_LOCK_TIMEOUT,			/* timeout */
			      POLL_32B_REG);			/* 32bit */
	if (ret == 0)
		printf("Failed to lock PLL for SGMII PHY %d\n", lane);

	/*
	 * 21. Set COMPHY input port PIN_TX_IDLE=0
	 */
	reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane),
		0x0, rb_pin_tx_idle);

	/*
	 * 22. After valid data appear on PIN_RXDATA bus, set PIN_RX_INIT=1.
	 *     to start RX initialization. PIN_RX_INIT_DONE will be cleared to
	 *     0 by the PHY. After RX initialization is done, PIN_RX_INIT_DONE
	 *     will be set to 1 by COMPHY. Set PIN_RX_INIT=0 after
	 *     PIN_RX_INIT_DONE= 1.
	 *     Please refer to RX initialization part for details.
	 */
	reg_set((void __iomem *)COMPHY_PHY_CFG1_ADDR(lane), rb_phy_rx_init,
		0x0);

	ret = comphy_poll_reg((void *)COMPHY_PHY_STAT1_ADDR(lane), /* address */
			      rb_rx_init_done,			/* value */
			      rb_rx_init_done,			/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);			/* 32bit */
	if (ret == 0)
		printf("Failed to init RX of SGMII PHY %d\n", lane);

	debug_exit();

	return ret;
}

void comphy_dedicated_phys_init(void)
{
	int node, usb32, ret = 1;
	const void *blob = gd->fdt_blob;

	debug_enter();

	for (usb32 = 0; usb32 <= 1; usb32++) {
		/*
		 * There are 2 UTMI PHYs in this SOC.
		 * One is independendent and one is paired with USB3 port (OTG)
		 */
		if (usb32 == 0) {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada3700-ehci");
		} else {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada3700-xhci");
		}

		if (node > 0) {
			if (fdtdec_get_is_enabled(blob, node)) {
				ret = comphy_usb2_power_up(usb32);
				if (ret == 0)
					printf("Failed to initialize UTMI PHY\n");
				else
					debug("UTMI PHY init succeed\n");
			} else {
				debug("USB%d node is disabled\n",
				      usb32 == 0 ? 2 : 3);
			}
		} else {
			debug("No USB%d node in DT\n", usb32 == 0 ? 2 : 3);
		}
	}

	node = fdt_node_offset_by_compatible(blob, -1,
					     "marvell,armada-8k-sdhci");
	if (node <= 0) {
		node = fdt_node_offset_by_compatible(
			blob, -1, "marvell,armada-3700-sdhci");
	}

	if (node > 0) {
		if (fdtdec_get_is_enabled(blob, node)) {
			ret = comphy_emmc_power_up();
			if (ret == 0)
				printf("Failed to initialize SDIO/eMMC PHY\n");
			else
				debug("SDIO/eMMC PHY init succeed\n");
		} else {
			debug("SDIO/eMMC node is disabled\n");
		}
	}  else {
		debug("No SDIO/eMMC node in DT\n");
	}

	debug_exit();
}

static int comphy_a3700_get_phy_select_num(u32 lane_num, u32 *phy_select_num)
{
	u32 i;

	for (i = 0; i < A3700_LANE_MAX_NUM; i++) {
		if (lane_num == lane_mux_map[i].lane_num) {
			*phy_select_num = lane_mux_map[i].phy_select_num;
			return 0;
		}
	}

	debug("No lane %d in the lane mux map table!\n", lane_num);
	return 1;
}

static void comphy_a3700_mux_init(struct chip_serdes_phy_config *chip_cfg,
				   struct comphy_map *serdes_map)
{
	u32 lane, phy_select_num;
	u32 comphy_max_count = chip_cfg->comphy_lanes_count;
	struct comphy_mux_data ordered_mux_data[A3700_LANE_MAX_NUM];
	struct comphy_map ordered_serdes_map[A3700_LANE_MAX_NUM];

	debug_enter();

	/*
	 * In PHY mux initialization, comphy_mux_init() takes it for granted
	 * that lanes' phy select bits in selector base register are ordered by
	 * lane number; but for a3700, lane1 phy select bit is before lane0 in
	 * selector base register as below, so mux data and serdes map need
	 * to be re-ordered to align with the lane index order in PHY Selector
	 * register.
	 *
	 * RD00183FCh (00000011h) - PHY Selector
	 *      bit 0: PCIE_GBE0_SEL -PHY Lane 1 Mode Select, 0h: GbE0 1h: PCIe
	 *      bit 4: USB_GBE1_SEL -PHY Lane 0 Mode Select, 0h: GbE1 1h: USB
	 *      bit 8: USB_SATA_SEL -PHY Lane 2 Mode Select, 0h: SATA 1h: USB
	 */
	memset(ordered_mux_data, 0, sizeof(ordered_mux_data));
	memset(ordered_serdes_map, 0, sizeof(ordered_serdes_map));
	for (lane = 0; lane < comphy_max_count; lane++) {
		if (comphy_a3700_get_phy_select_num(lane, &phy_select_num))
			return;
		memcpy(&ordered_mux_data[phy_select_num],
		       &a3700_comphy_mux_data[lane],
		       sizeof(struct comphy_mux_data));
		memcpy(&ordered_serdes_map[phy_select_num],
		       serdes_map + lane,
		       sizeof(struct comphy_map));
	}

	chip_cfg->mux_data = ordered_mux_data;
	comphy_mux_init(chip_cfg, ordered_serdes_map,
			(void __iomem *)COMPHY_SEL_ADDR);

	debug_exit();
}

int comphy_a3700_init(struct chip_serdes_phy_config *chip_cfg,
		      struct comphy_map *serdes_map)
{
	struct comphy_map *comphy_map;
	u32 comphy_max_count = chip_cfg->comphy_lanes_count;
	u32 lane, ret = 0;

	debug_enter();

	if (comphy_max_count > A3700_LANE_MAX_NUM) {
		printf("Comphy number %d is too large\n", comphy_max_count);
		return 1;
	}

	/* PHY mux initialize */
	comphy_a3700_mux_init(chip_cfg, serdes_map);

	for (lane = 0, comphy_map = serdes_map; lane < comphy_max_count;
	     lane++, comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x invert=%d\n",
		      comphy_map->type, comphy_map->invert);

		switch (comphy_map->type) {
		case COMPHY_TYPE_UNCONNECTED:
			continue;
			break;

		case COMPHY_TYPE_PEX0:
			ret = comphy_pcie_power_up(comphy_map->speed,
						   comphy_map->invert);
			break;

		case COMPHY_TYPE_USB3:
		case COMPHY_TYPE_USB3_HOST0:
		case COMPHY_TYPE_USB3_DEVICE:
			ret = comphy_usb3_power_up(comphy_map->speed,
						   comphy_map->invert,
						   (lane == 2) ? true : false);
			break;

		case COMPHY_TYPE_SGMII0:
		case COMPHY_TYPE_SGMII1:
			ret = comphy_sgmii_power_up(lane, comphy_map->speed,
						    comphy_map->invert);
			break;

		case COMPHY_TYPE_SATA0:
			ret = comphy_sata_power_up(comphy_map->invert);
			break;

		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n",
			      lane);
			ret = 1;
			break;
		}
		if (ret == 0)
			printf("PLL is not locked - Failed to initialize lane %d\n",
			       lane);
	}

	debug_exit();
	return ret;
}
