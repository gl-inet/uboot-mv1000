/*
 * ***************************************************************************
 * Copyright (C) 2017 Marvell International Ltd.
 * ***************************************************************************
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************
 */

/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <libfdt.h>
#include <mvebu/mvebu_chip_sar.h>
#include <mach/clock.h>

#include <sar-uclass.h>

#define CPU_CLOCK_ID	0
#define DDR_CLOCK_ID	1
#define RING_CLOCK_ID	2

/* SAR AP806 registers */
#define SAR_CLOCK_FREQ_MODE_OFFSET	0
#define SAR_CLOCK_FREQ_MODE_MASK	(0x1f << SAR_CLOCK_FREQ_MODE_OFFSET)
#define SAR_TEST_MODE_ENABLE_OFFSET	5
#define SAR_TEST_MODE_ENABLE_MASK	(0x1 << SAR_TEST_MODE_ENABLE_OFFSET)
#define SAR_SKIP_LINK_I2C_INIT_OFFSET	6
#define SAR_SKIP_LINK_I2C_INIT_MASK	(0x1 << SAR_SKIP_LINK_I2C_INIT_OFFSET)
#define SAR_POR_BYPASS_OFFSET		7
#define SAR_POR_BYPASS_MASK		(0x1 << SAR_POR_BYPASS_OFFSET)
#define SAR_BOOT_SOURCE_OFFSET		8
#define SAR_BOOT_SOURCE_MASK		(0x7 << SAR_BOOT_SOURCE_OFFSET)
#define SAR_PIDI_C2C_IHB_SELECT_OFFSET	11
#define SAR_PIDI_C2C_IHB_SELECT_MASK	(0x1 << SAR_PIDI_C2C_IHB_SELECT_OFFSET)
#define SAR_I2C_INIT_ENABLE_OFFSET	12
#define SAR_I2C_INIT_ENABLE_MASK	(0x1 << SAR_I2C_INIT_ENABLE_OFFSET)
#define SAR_SSCG_DISABLE_OFFSET		13
#define SAR_SSCG_DISABLE_MASK		(0x1 << SAR_SSCG_DISABLE_OFFSET)
#define SAR_PIDI_HW_TRAINING_DIS_OFFSET	14
#define SAR_PIDI_HW_TRAINING_DIS_MASK	(0x1 << SAR_PIDI_HW_TRAINING_DIS_OFFSET)
#define SAR_CPU_FMAX_REFCLK_OFFSET	15
#define SAR_CPU_FMAX_REFCLK_MASK	(0x1 << SAR_CPU_FMAX_REFCLK_OFFSET)
#define SAR_IHB_DIFF_REFCLK_DIS_OFFSET	16
#define SAR_IHB_DIFF_REFCLK_DIS_MASK	(0x1 << SAR_IHB_DIFF_REFCLK_DIS_OFFSET)
#define SAR_REF_CLK_MSTR_OFFSET		17
#define SAR_REF_CLK_MSTR_MASK		(0x1 << SAR_REF_CLK_MSTR_OFFSET)
#define SAR_CPU_WAKE_UP_OFFSET		18
#define SAR_CPU_WAKE_UP_MASK		(0x1 << SAR_CPU_WAKE_UP_OFFSET)
#define SAR_XTAL_BYPASS_OFFSET		19
#define SAR_XTAL_BYPASS_MASK		(0x1 << SAR_XTAL_BYPASS_OFFSET)
#define SAR_PIDI_LOW_SPEED_OFFSET	20
#define SAR_PIDI_LOW_SPEED_MASK		(0x1 << SAR_PIDI_LOW_SPEED_OFFSET)

#define AP806_SAR_1_REG			4
#define SAR1_PLL2_OFFSET		(9)
#define SAR1_PLL2_MASK			(0x1f << SAR1_PLL2_OFFSET)
#define SAR1_PLL1_OFFSET		(14)
#define SAR1_PLL1_MASK			(0x1f << SAR1_PLL1_OFFSET)
#define SAR1_PLL0_OFFSET		(19)
#define SAR1_PLL0_MASK			(0x1f << SAR1_PLL0_OFFSET)
#define SAR1_PIDI_CONNECT_OFFSET	(24)
#define SAR1_PIDI_CONNECT_MASK		(1 << SAR1_PIDI_CONNECT_OFFSET)

struct sar_info {
	char *name;
	u32 offset;
	u32 mask;
};

struct sar_info ap806_sar_0[] = {
	{"Clock Freq mode		  ",
		SAR_CLOCK_FREQ_MODE_OFFSET, SAR_CLOCK_FREQ_MODE_MASK },
	{"Test mode enable		  ",
		SAR_TEST_MODE_ENABLE_OFFSET, SAR_TEST_MODE_ENABLE_MASK },
	{"Skip link i2c init		  ",
		SAR_SKIP_LINK_I2C_INIT_OFFSET, SAR_SKIP_LINK_I2C_INIT_MASK },
	{"Por ByPass			  ",
		SAR_POR_BYPASS_OFFSET, SAR_POR_BYPASS_MASK },
	{"Boot Source			  ",
		SAR_BOOT_SOURCE_OFFSET, SAR_BOOT_SOURCE_MASK },
	{"PIDI C2C IHB select		  ",
		SAR_PIDI_C2C_IHB_SELECT_OFFSET, SAR_PIDI_C2C_IHB_SELECT_MASK },
	{"I2C init enable		  ",
		SAR_I2C_INIT_ENABLE_OFFSET, SAR_I2C_INIT_ENABLE_MASK },
	{"SSCG disable			  ",
		SAR_SSCG_DISABLE_OFFSET, SAR_SSCG_DISABLE_MASK },
	{"PIDI hw training disable	  ",
		SAR_PIDI_HW_TRAINING_DIS_OFFSET,
		SAR_PIDI_HW_TRAINING_DIS_MASK },
	{"CPU Fmax refclk select	  ",
		SAR_CPU_FMAX_REFCLK_OFFSET, SAR_CPU_FMAX_REFCLK_MASK },
	{"IHB differential refclk disable ",
		SAR_IHB_DIFF_REFCLK_DIS_OFFSET, SAR_IHB_DIFF_REFCLK_DIS_MASK },
	{"Ref clk mstr			  ",
		SAR_REF_CLK_MSTR_OFFSET, SAR_REF_CLK_MSTR_MASK },
	{"CPU wake up			  ",
		SAR_CPU_WAKE_UP_OFFSET, SAR_CPU_WAKE_UP_MASK },
	{"Xtal ByPass			  ",
		SAR_XTAL_BYPASS_OFFSET, SAR_XTAL_BYPASS_MASK },
	{"PIDI low speed		  ",
		SAR_PIDI_LOW_SPEED_OFFSET, SAR_PIDI_LOW_SPEED_MASK },
	{"",			-1,			-1},
};

struct sar_info ap806_sar_1[] = {
	{"PIDI connect       ", SAR1_PIDI_CONNECT_OFFSET,
				SAR1_PIDI_CONNECT_MASK },
	{"PLL0 Config        ", SAR1_PLL0_OFFSET, SAR1_PLL0_MASK },
	{"PLL1 Config        ", SAR1_PLL1_OFFSET, SAR1_PLL1_MASK },
	{"PLL2 Config        ", SAR1_PLL2_OFFSET, SAR1_PLL2_MASK },
	{"",			-1,			-1},
};

enum clocking_options {
	CPU_2000_DDR_1200_RCLK_1200 = 0x0,
	CPU_2000_DDR_1050_RCLK_1050 = 0x1,
	CPU_1600_DDR_800_RCLK_800 = 0x4,
	CPU_1800_DDR_1200_RCLK_1200 = 0x6,
	CPU_1800_DDR_1050_RCLK_1050 = 0x7,
	CPU_1600_DDR_1050_RCLK_1050 = 0x0d,
	CPU_1000_DDR_650_RCLK_650 = 0x13,
	CPU_1300_DDR_800_RCLK_800 = 0x14,
	CPU_1300_DDR_650_RCLK_650 = 0x17,
	CPU_1200_DDR_800_RCLK_800 = 0x19,
	CPU_1400_DDR_800_RCLK_800 = 0x1a,
	CPU_600_DDR_800_RCLK_800 = 0x1b,
	CPU_800_DDR_800_RCLK_800 = 0x1c,
	CPU_1000_DDR_800_RCLK_800 = 0x1d,
};

static const u32 pll_freq_tbl[16][4] = {
	/* CPU */   /* DDR */   /* Ring */
	{2.0 * GHz, 1.2  * GHz, 1.2  * GHz, CPU_2000_DDR_1200_RCLK_1200},
	{2.0 * GHz, 1.05 * GHz, 1.05 * GHz, CPU_2000_DDR_1050_RCLK_1050},
	{1.8 * GHz, 1.2  * GHz, 1.2  * GHz, CPU_1800_DDR_1200_RCLK_1200},
	{1.8 * GHz, 1.05 * GHz, 1.05 * GHz, CPU_1800_DDR_1050_RCLK_1050},
	{1.6 * GHz, 1.05 * GHz, 1.05 * GHz, CPU_1600_DDR_1050_RCLK_1050},
	{1.3 * GHz, 800  * MHz, 800  * MHz, CPU_1300_DDR_800_RCLK_800},
	{1.3 * GHz, 650  * MHz, 650  * MHz, CPU_1300_DDR_650_RCLK_650},
	{1.6 * GHz, 800  * MHz, 800  * MHz, CPU_1600_DDR_800_RCLK_800},
	{1.0 * GHz, 650  * MHz, 650  * MHz, CPU_1000_DDR_650_RCLK_650},
	{1.2 * GHz, 800  * MHz, 800  * MHz, CPU_1200_DDR_800_RCLK_800},
	{1.4 * GHz, 800  * MHz, 800  * MHz, CPU_1400_DDR_800_RCLK_800},
	{600 * MHz, 800  * MHz, 800  * MHz, CPU_600_DDR_800_RCLK_800},
	{800 * MHz, 800  * MHz, 800  * MHz, CPU_800_DDR_800_RCLK_800},
	{1.0 * GHz, 800  * MHz, 800  * MHz, CPU_1000_DDR_800_RCLK_800}
};

static u32 sar_get_clock_freq_mode(struct udevice *dev)
{
	u32 i;
	struct dm_sar_pdata *priv = dev_get_priv(dev);

	u32 clock_freq = (readl(priv->sar_base) & SAR_CLOCK_FREQ_MODE_MASK) >>
			  SAR_CLOCK_FREQ_MODE_OFFSET;
	for (i = 0; i < 16; i++) {
		if (pll_freq_tbl[i][3] == clock_freq)
			return i;
	}
	error("sar regs: unsuported clock freq mode %d", clock_freq);
	return -1;
}

int ap806_sar_value_get(struct udevice *dev, enum mvebu_sar_opts sar_opt,
		struct sar_val *val)
{
	u32 clock_type, clock_freq_mode;

	switch (sar_opt) {
	case(SAR_CPU_FREQ):
		clock_type = CPU_CLOCK_ID;
		break;
	case(SAR_DDR_FREQ):
		clock_type = DDR_CLOCK_ID;
		break;
	case(SAR_AP_FABRIC_FREQ):
		clock_type = RING_CLOCK_ID;
		break;
	default:
		error("AP806-SAR: Unsupported SAR option %d.\n", sar_opt);
		return -EINVAL;
	}
	clock_freq_mode = sar_get_clock_freq_mode(dev);
	val->raw_sar_val = clock_freq_mode;
	val->freq = pll_freq_tbl[clock_freq_mode][clock_type];
	return 0;
}

static int ap806_sar_dump(struct udevice *dev)
{
	u32 reg, val;
	struct sar_info *sar;
	struct dm_sar_pdata *priv = dev_get_priv(dev);

	reg = readl(priv->sar_base);
	printf("AP806 SAR register 0 [0x%08x]:\n", reg);
	printf("----------------------------------\n");
	sar = ap806_sar_0;
	while (sar->offset != -1) {
		val = (reg & sar->mask) >> sar->offset;
		printf("%s  0x%x\n", sar->name, val);
		sar++;
	}

	reg = readl(priv->sar_base + AP806_SAR_1_REG);
	printf("\nAP806 SAR register 1 [0x%08x]:\n", reg);
	printf("----------------------------------\n");
	sar = ap806_sar_1;
	while (sar->offset != -1) {
		val = (reg & sar->mask) >> sar->offset;
		printf("%s  0x%x\n", sar->name, val);
		sar++;
	}
	return 0;
}

int ap806_sar_init(struct udevice *dev)
{
	int ret, i;

	u32 sar_list[] = {
		SAR_CPU_FREQ,
		SAR_DDR_FREQ,
		SAR_AP_FABRIC_FREQ
	};

	for (i = 0; i < ARRAY_SIZE(sar_list); i++) {
		ret = mvebu_sar_id_register(dev, sar_list[i]);
		if (ret) {
			error("Failed to register SAR %d, for AP806.\n",
			      sar_list[i]);
			return ret;
		}
	}
	return 0;
}

static const struct sar_ops ap806_sar_ops = {
	.sar_init_func = ap806_sar_init,
	.sar_value_get_func = ap806_sar_value_get,
	.sar_dump_func = ap806_sar_dump,
};

U_BOOT_DRIVER(ap806_sar) = {
	.name = "ap806_sar",
	.id = UCLASS_SAR,
	.priv_auto_alloc_size = sizeof(struct dm_sar_pdata),
	.ops = &ap806_sar_ops,
};
