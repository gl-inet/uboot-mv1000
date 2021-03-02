/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <mach/clock.h>

DECLARE_GLOBAL_DATA_PTR;

/* Armada 7k/8k */
#define MVEBU_RFU_BASE			(MVEBU_REGISTER(0x6f0000))
#define RFU_GLOBAL_SW_RST		(MVEBU_RFU_BASE + 0x84)
#define RFU_SW_RESET_OFFSET		0
#define SZ_1M				0x00100000
#define SZ_256M				0x10000000
#define SZ_1G				0x40000000
#define SZ_4G				0x100000000

/*
 * The following table includes all memory regions for Armada 7k and
 * 8k SoCs. The Armada 7k is missing the CP110 slave regions here. Lets
 * define these regions at the beginning of the struct so that they
 * can be easier removed later dynamically if an Armada 7k device is detected.
 * For a detailed memory map, please see doc/mvebu/armada-8k-memory.txt
 */
#define ARMADA_7K8K_COMMON_REGIONS_START	2
static struct mm_region mvebu_mem_map[] = {
	/* Armada 80x0 memory regions include the CP1 (slave) units */
	{
		/* SRAM, MMIO regions - CP110 slave region */
		.phys = 0xf4000000UL,
		.virt = 0xf4000000UL,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* PCI CP1 regions */
		.phys = 0xfa000000UL,
		.virt = 0xfa000000UL,
		.size = 0x03000000UL,	/* 48MiB CP110 slave PCI space */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	/* Armada 80x0 and 70x0 common memory regions start here */
	{
		/* RAM */
		.phys = 0x0UL,
		.virt = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* SRAM, MMIO regions - AP806 region */
		.phys = 0xf0000000UL,
		.virt = 0xf0000000UL,
		.size = 0x01000000UL,	/* 16MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* SRAM, MMIO regions - CP110 master region */
		.phys = 0xf2000000UL,
		.virt = 0xf2000000UL,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* PCI CP0 regions */
		.phys = 0xf6000000UL,
		.virt = 0xf6000000UL,
		.size = 0x04000000UL,	/* 64MiB CP110 master PCI space */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* PCI CP1 IO regions or mochi indirect access space */
		.phys = 0xfd000000UL,
		.virt = 0xfd000000UL,
		.size = 0x00200000UL,	/* 2MiB */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* mochi indirect access space */
		.phys = 0xfe000000UL,
		.virt = 0xfe000000UL,
		.size = 0x00200000UL,	/* 2MiB */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

void enable_caches(void)
{
	/*
	 * Armada 7k is not equipped with the CP110 slave CP. In case this
	 * code runs on an Armada 7k device, lets remove the CP110 slave
	 * entries from the memory mapping by moving the start to the
	 * common regions.
	 */
	if (of_machine_is_compatible("marvell,armada7040"))
		mem_map = &mvebu_mem_map[ARMADA_7K8K_COMMON_REGIONS_START];

	icache_enable();
	dcache_enable();
}

void reset_cpu(ulong ignored)
{
	u32 reg;

	reg = readl(RFU_GLOBAL_SW_RST);
	reg &= ~(1 << RFU_SW_RESET_OFFSET);
	writel(reg, RFU_GLOBAL_SW_RST);
}

#ifdef CONFIG_NAND_PXA3XX
/* Return NAND clock in Hz */
u32 mvebu_get_nand_clock(void)
{
	unsigned long NAND_FLASH_CLK_CTRL = 0xF2440700UL;
	unsigned long NF_CLOCK_SEL_MASK = 0x1;
	u32 reg;

	reg = readl(NAND_FLASH_CLK_CTRL);
	if (reg & NF_CLOCK_SEL_MASK)
		return 400 * 1000000;
	else
		return 250 * 1000000;
}

/* Select NAND in the device bus multiplexer */
void mvebu_nand_select(void)
{
	unsigned long SOC_DEV_MULTIPLEX_REG = 0xf2440208;
	unsigned long SOC_MUX_NAND_EN_MASK = 0x1;

	setbits_le32(SOC_DEV_MULTIPLEX_REG, SOC_MUX_NAND_EN_MASK);
}
#endif

int mvebu_dram_init(void)
{
	u32 cs;
	gd->ram_size = 0;
	for (cs = 0; cs < 4; cs++)
		if (get_info(DRAM_CS0 + cs))
			gd->ram_size += get_info(DRAM_CS0_SIZE + cs);

	gd->ram_size *= SZ_1M;
	/* if DRAM size == 0, print error message */
	if (gd->ram_size == 0) {
		error("DRAM size not initialized - check DRAM configuration\n");
		printf("\n Using temporary DRAM size of 256MB.\n\n");
		gd->ram_size = SZ_256M;
	}

	return 0;
}

void mvebu_dram_init_banksize(void)
{
	/*
	 * Config 2 DRAM banks:
	 * Bank 0 - max size 4G - 1G
	 * Bank 1 - ram size - 4G + 1G
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size <= SZ_4G - SZ_1G) {
		gd->bd->bi_dram[0].size = min(gd->ram_size,
					      (phys_size_t)(SZ_4G - SZ_1G));
		return;
	}

	gd->bd->bi_dram[0].size = SZ_4G - SZ_1G;
	gd->bd->bi_dram[1].start = SZ_4G;
	gd->bd->bi_dram[1].size = gd->ram_size - SZ_4G + SZ_1G;
}


#if defined(CONFIG_DISPLAY_BOARDINFO)
int print_cpuinfo(void)
{
	soc_print_clock_info();
	return 0;
}
#endif
