/*
 * Copyright (C) 2012-2014 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/crm_regs.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/imx-common/video.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#include <i2c.h>

#ifdef CONFIG_CMD_SATA
#include <asm/imx-common/sata.h>
#endif
#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/

#ifdef CONFIG_MAX7310_IOEXP
#include <gpio_exp.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define I2C_EXP_RST IMX_GPIO_NR(1, 15)
#define I2C3_STEER  IMX_GPIO_NR(5, 4)
#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

/*Need more drive strength for SD1 slot on base board*/
#define USDHC1_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)


#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);

	return 0;
}

iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_KEY_COL1__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_KEY_COL2__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

/* I2C2 PMIC, iPod, Tuner, Codec, Touch, HDMI EDID, MIPI CSI2 card */
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

/*
 * I2C3 MLB, Port Expanders (A, B, C), Video ADC, Light Sensor,
 * Compass Sensor, Accelerometer, Res Touch
 */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};

iomux_v3_cfg_t const i2c3_pads[] = {
	MX6_PAD_EIM_A24__GPIO5_IO04		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const port_exp[] = {
	MX6_PAD_SD2_DAT0__GPIO1_IO15		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D6__GPIO2_IO06  | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

#ifdef CONFIG_SYS_I2C_MXC
/* set all switches APS in normal and PFM mode in standby */
static int setup_pmic_mode(int chip)
{
	unsigned char offset, i, switch_num, value;

	if (!chip) {
		/* pfuze100 */
		switch_num = 6;
		offset = 0x31;
	} else {
		/* pfuze200 */
		switch_num = 4;
		offset = 0x38;
	}

	value = 0xc;
	if (i2c_write(0x8, 0x23, 1, &value, 1)) {
		printf("Set SW1AB mode error!\n");
		return -1;
	}

	for (i = 0; i < switch_num - 1; i++) {
		if (i2c_write(0x8, offset + i * 7, 1, &value, 1)) {
			printf("Set switch%x mode error!\n", offset);
			return -1;
		}
	}

	return 0;
}

static int setup_pmic_voltages(void)
{
	printf("  %s: *** was called\n", __FUNCTION__);
	unsigned char value, rev_id = 0 ;
	printf("  %s: *** i2c_set_bus_num(1)\n", __FUNCTION__);
	i2c_set_bus_num(1);
	printf("  %s: *** i2c_probe(0x8)\n", __FUNCTION__);
	if (!i2c_probe(0x8)) {
		printf("  %s: *** i2c_read(0x8, 0, 1, &value, 1)\n", __FUNCTION__);
		if (i2c_read(0x8, 0, 1, &value, 1)) {
			printf("Read device ID error!\n");
			return -1;
		}
		if (i2c_read(0x8, 3, 1, &rev_id, 1)) {
			printf("Read Rev ID error!\n");
			return -1;
		}
		printf("Found PFUZE100! deviceid=%x,revid=%x\n", value, rev_id);

		if (setup_pmic_mode(value & 0xf)) {
			printf("setup pmic mode error!\n");
			return -1;
		}
		/* set SW1AB staby volatage 0.975V*/
		if (i2c_read(0x8, 0x21, 1, &value, 1)) {
			printf("Read SW1ABSTBY error!\n");
			return -1;
		}
		value &= ~0x3f;
		value |= 0x1b;
		if (i2c_write(0x8, 0x21, 1, &value, 1)) {
			printf("Set SW1ABSTBY error!\n");
			return -1;
		}
		/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
		if (i2c_read(0x8, 0x24, 1, &value, 1)) {
			printf("Read SW1ABSTBY error!\n");
			return -1;
		}
		value &= ~0xc0;
		value |= 0x40;
		if (i2c_write(0x8, 0x24, 1, &value, 1)) {
			printf("Set SW1ABSTBY error!\n");
			return -1;
		}

		/* set SW1C staby volatage 0.975V*/
		if (i2c_read(0x8, 0x2f, 1, &value, 1)) {
			printf("Read SW1CSTBY error!\n");
			return -1;
		}
		value &= ~0x3f;
		value |= 0x1b;
		if (i2c_write(0x8, 0x2f, 1, &value, 1)) {
			printf("Set SW1CSTBY error!\n");
			return -1;
		}

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		if (i2c_read(0x8, 0x32, 1, &value, 1)) {
			printf("Read SW1ABSTBY error!\n");
			return -1;
		}
		value &= ~0xc0;
		value |= 0x40;
		if (i2c_write(0x8, 0x32, 1, &value, 1)) {
			printf("Set SW1ABSTBY error!\n");
			return -1;
		}
	}

	printf("  %s: was left\n", __FUNCTION__);
	return 0;
}

//#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned char value;
	/* increase VDDARM/VDDSOC to support 1.2G chip */
	if (check_1_2G()) {
		ldo_bypass = 0;	/* ldo_enable on 1.2G chip */
		printf("1.2G chip, increase VDDARM_IN/VDDSOC_IN\n");
		/* increase VDDARM to 1.425V */
		if (i2c_read(0x8, 0x20, 1, &value, 1)) {
			printf("Read SW1AB error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x2d;
		if (i2c_write(0x8, 0x20, 1, &value, 1)) {
			printf("Set SW1AB error!\n");
			return;
		}
		/* increase VDDSOC to 1.425V */
		if (i2c_read(0x8, 0x2e, 1, &value, 1)) {
			printf("Read SW1C error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x2d;
		if (i2c_write(0x8, 0x2e, 1, &value, 1)) {
			printf("Set SW1C error!\n");
			return;
		}
	}
}
//#endif
#endif

static void setup_iomux_uart(void)
{
	printf("  setup_iomux_uart: was called\n");
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
	printf("  setup_iomux_uart: was left\n");
}

#ifdef CONFIG_FSL_ESDHC

struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

int mmc_get_env_devno(void)
{
	u32 soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	u32 dev_no;
	u32 bootsel;

	bootsel = (soc_sbmr & 0x000000FF) >> 6 ;

	/* If not boot from sd/mmc, use default value */
	if (bootsel != 1)
		return CONFIG_SYS_MMC_ENV_DEV;

	/* BOOT_CFG2[3] and BOOT_CFG2[4] */
	dev_no = (soc_sbmr & 0x00001800) >> 11;

	/* need ubstract 1 to map to the mmc device id
	 * see the comments in board_mmc_init function
	 */

	dev_no -= 2;

	return dev_no;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no + 2;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR) {
		gpio_direction_input(IMX_GPIO_NR(2, 6));
		ret = !gpio_get_value(IMX_GPIO_NR(2, 6));
	} else /* Don't have the CD GPIO pin on board */
		ret = 1;

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return status;
		}

		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}

	return status;
}

int check_mmc_autodetect(void)
{
	char *autodetect_str = getenv("mmcautodetect");

	if ((autodetect_str != NULL) &&
		(strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

#ifdef CONFIG_ENV_IS_IN_MMC
void board_late_mmc_env_init_notused(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_devno();

	if (!check_mmc_autodetect())
		return;

	setenv_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	setenv("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
} 
#endif // CONFIG_ENV_IS_IN_MMC

#endif

#ifdef CONFIG_SYS_USE_SPINOR
iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D19__GPIO3_IO19  | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Steer logic */
	MX6_PAD_EIM_A24__GPIO5_IO04  | MUX_PAD_CTRL(NO_PAD_CTRL),
};

void setup_spinor(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
					 ARRAY_SIZE(ecspi1_pads));
	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);
	gpio_direction_output(IMX_GPIO_NR(3, 19), 0);
}
#endif

#ifdef CONFIG_SYS_USE_EIMNOR
iomux_v3_cfg_t eimnor_pads[] = {
	MX6_PAD_EIM_D16__EIM_DATA16      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D17__EIM_DATA17      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D18__EIM_DATA18      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D19__EIM_DATA19      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D20__EIM_DATA20      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D21__EIM_DATA21      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D22__EIM_DATA22      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D23__EIM_DATA23      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D24__EIM_DATA24      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D25__EIM_DATA25      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D26__EIM_DATA26      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D27__EIM_DATA27      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D28__EIM_DATA28      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D29__EIM_DATA29      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D30__EIM_DATA30      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D31__EIM_DATA31      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA0__EIM_AD00    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA1__EIM_AD01    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA2__EIM_AD02    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA3__EIM_AD03    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA4__EIM_AD04    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA5__EIM_AD05    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA6__EIM_AD06    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA7__EIM_AD07    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA8__EIM_AD08    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA9__EIM_AD09    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA10__EIM_AD10   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA11__EIM_AD11   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL) ,
	MX6_PAD_EIM_DA12__EIM_AD12   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA13__EIM_AD13   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA14__EIM_AD14   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA15__EIM_AD15   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A16__EIM_ADDR16      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A17__EIM_ADDR17      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A18__EIM_ADDR18      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A19__EIM_ADDR19      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A20__EIM_ADDR20      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A21__EIM_ADDR21      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A22__EIM_ADDR22      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A23__EIM_ADDR23      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_OE__EIM_OE_B         | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_RW__EIM_RW           | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_CS0__EIM_CS0_B       | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Steer logic */
	MX6_PAD_EIM_A24__GPIO5_IO04      | MUX_PAD_CTRL(NO_PAD_CTRL),
};
static void eimnor_cs_setup(void)
{
	writel(0x00000120, WEIM_BASE_ADDR + 0x090);
	writel(0x00020181, WEIM_BASE_ADDR + 0x000);
	writel(0x00000001, WEIM_BASE_ADDR + 0x004);
	writel(0x0a020000, WEIM_BASE_ADDR + 0x008);
	writel(0x0000c000, WEIM_BASE_ADDR + 0x00c);
	writel(0x0804a240, WEIM_BASE_ADDR + 0x010);
}

static void setup_eimnor(void)
{
	imx_iomux_v3_setup_multiple_pads(eimnor_pads,
			ARRAY_SIZE(eimnor_pads));

	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);

	eimnor_cs_setup();
}
#endif

#ifdef CONFIG_SYS_USE_NAND
iomux_v3_cfg_t gpmi_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_ALE__NAND_ALE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL0),
	MX6_PAD_NANDF_CS0__NAND_CE0_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_CMD__NAND_RE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_CLK__NAND_WE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D0__NAND_DATA00	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D1__NAND_DATA01	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D2__NAND_DATA02	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D3__NAND_DATA03	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D4__NAND_DATA04	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D5__NAND_DATA05	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D6__NAND_DATA06	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D7__NAND_DATA07	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_DAT0__NAND_DQS		| MUX_PAD_CTRL(GPMI_PAD_CTRL1),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));

	/* gate ENFC_CLK_ROOT clock first,before clk source switch */
	clrbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable ENFC_CLK_ROOT clock */
	setbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

#ifdef CONFIG_MAX7310_IOEXP
void reset_max7310(void)
{
	printf("%s: was called\n", __func__);
	gpio_direction_output(I2C_EXP_RST, 1);
	imx_iomux_v3_setup_multiple_pads(port_exp,
					ARRAY_SIZE(port_exp));
	printf("%s: was left\n", __func__);
}

int setup_max7310(void)
{
#ifdef CONFIG_SYS_I2C_MXC
	/* set steering config to i2c,
	  * note: this causes pin conflicts with eimnor and spinor
	  */
	printf("%s: was called\n", __func__);
    gpio_direction_output(IMX_GPIO_NR(5, 4), 1);
	imx_iomux_v3_setup_multiple_pads(i2c3_pads,
					ARRAY_SIZE(i2c3_pads));

	/*setup i2c info 2*/
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

#ifdef CONFIG_MAX7310_IOEXP
	gpio_exp_setup_port(1, 2, 0x30);
	gpio_exp_setup_port(2, 2, 0x32);
	gpio_exp_setup_port(3, 2, 0x34);
#endif

	printf("%s: was left 0\n", __func__);
	return 0;
#else

	printf("%s: was left -EPERM\n", __func__);
	return -EPERM;
#endif
}
#endif
int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	int reg = readl(&iomux->gpr[2]);

	reg &= ~(IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		 IOMUXC_GPR2_LVDS_CH1_MODE_MASK);

	writel(reg, &iomux->gpr[2]);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

struct display_info_t const displays[] =
{
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB666,
		.detect	= NULL,
		.enable	= NULL,
		.mode	=
		{
			.name           = "LGXGA15",
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 768,
			.pixclock       = 15384,
			.left_margin    = 100,
			.right_margin   = 100,
			.upper_margin   = 10,
			.lower_margin   = 15,
			.hsync_len      = 50,
			.vsync_len      = 13,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},	
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB666,
		.detect	= NULL,
		.enable	= NULL,
		.mode	=
		{
			.name           = "LGWXGA13",
			.refresh        = 60,
			.xres           = 1366,
			.yres           = 768,
			.pixclock       = 14430,
			.left_margin    = 10,
			.right_margin   = 48,
			.upper_margin   = 16,
			.lower_margin   = 3,
			.hsync_len      = 32,
			.vsync_len      = 6,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= do_enable_hdmi,
		.mode	=
		{
			.name           = "HDMI",
			.refresh        = 60,
			.xres           = 640,
			.yres           = 480,
			.pixclock       = 39721,
			.left_margin    = 48,
			.right_margin   = 16,
			.upper_margin   = 33,
			.lower_margin   = 10,
			.hsync_len      = 96,
			.vsync_len      = 2,
			.sync           = 0,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
};
size_t display_count = ARRAY_SIZE(displays);

iomux_v3_cfg_t const lvds_pads[] = {
	MX6_PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_GPIO_2__GPIO1_IO02 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(ENET_PAD_CTRL),


};

static void setup_iomux_lvds(void)
{
	gpio_direction_output(IMX_GPIO_NR(7, 12), 0);
	gpio_direction_output(IMX_GPIO_NR(5, 15), 1);
	gpio_direction_output(IMX_GPIO_NR(1, 2), 1);
	gpio_direction_output(IMX_GPIO_NR(5, 14), 1);

	imx_iomux_v3_setup_multiple_pads(lvds_pads,
					 ARRAY_SIZE(lvds_pads));
}


iomux_v3_cfg_t const backlight_pads[] = {
	MX6_PAD_SD1_DAT3__GPIO1_IO21 | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_backlight(void)
{
	gpio_direction_output(IMX_GPIO_NR(1, 21), 1);
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
}

static void setup_display(void)
{
	printf("setup_display: was called\n");
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	setup_iomux_lvds();
	setup_iomux_backlight();
	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB_DI0 and LDB_DI1 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* Set LDB_DI0 and LDB_DI1 clk select to 3b'011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
		 MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
	       (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0 <<
		MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0 <<
		MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT |
	      IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
	      IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0 |
	      IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg &= ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
		 IOMUXC_GPR3_HDMI_MUX_CTL_MASK);
	reg |= (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET) |
	       (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	gpio_direction_output(IMX_GPIO_NR(7, 12), 1);
	printf("setup_display: was left\n");
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

	return cpu_eth_init(bis);
}

#define BOARD_REV_B  0x200
#define BOARD_REV_A  0x100

static int mx6sabre_rev(void)
{
	/*
	 * Get Board ID information from OCOTP_GP1[15:8]
	 * i.MX6Q ARD RevA: 0x01
	 * i.MX6Q ARD RevB: 0x02
	 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
			(struct fuse_bank4_regs *)bank->fuse_regs;
	int reg = readl(&fuse->gp1);
	int ret;

	switch (reg >> 8 & 0x0F) {
	case 0x02:
		ret = BOARD_REV_B;
		break;
	case 0x01:
	default:
		ret = BOARD_REV_A;
		break;
	}

	return ret;
}

u32 get_board_rev(void)
{
	int rev = mx6sabre_rev();

	return (get_cpu_rev() & ~(0xF << 8)) | rev;
}

iomux_v3_cfg_t const pwr_key_pads[] = {
	MX6_PAD_EIM_LBA__GPIO2_IO27  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D4__GPIO2_IO04  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_COL2__GPIO4_IO10  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_COL4__GPIO4_IO14  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW4__GPIO4_IO15  | MUX_PAD_CTRL(NO_PAD_CTRL),
};


//#define __DELAY_POWER_KEY_SIGNAL__
#define DEBOUNCE_TIME_MS 100
static int get_power_key_signal(void)
{
	u32 reset_cause;

	gpio_direction_input(IMX_GPIO_NR(2, 27));
	reset_cause = readl(SRC_BASE_ADDR + 0x8);
	writel(reset_cause, SRC_BASE_ADDR + 0x8);

#ifdef __DELAY_POWER_KEY_SIGNAL__ 
	do
	{
		if(0x0001 != reset_cause)
		{
			//This state is not normal power on, it may be caused by reset or watchdog
			return 1;
		}
		else
		{
			if(gpio_get_value(IMX_GPIO_NR(2, 27)))
			{
			//This state is normal power on
				return 1;
			}
		}
	}while(1);
#else
	{
		if(0x0001 == reset_cause) {
			mdelay(DEBOUNCE_TIME_MS); 
			if(gpio_get_value(IMX_GPIO_NR(2, 27)))
				return 1;
			else
				return 0;
		}else {
			return 1;
		}


	}
#endif
	return 0;
}

#define TF9300_CMD_SOC          0x2C
int battery_level(void)
{
	u8 buf[4];

	if (!i2c_probe(0x55)) {
		if (i2c_read(0x55, (uint)TF9300_CMD_SOC, 1, &buf[0], 2)) {
			printf("%s:i2c_write:error\n", __func__);
			return -1;
		}
       		return ((int)(buf[1]<<8)+buf[0]);	
       }
	return -1;
}

int ac_off_detect(void)
{
	gpio_direction_input(IMX_GPIO_NR(4, 10));
	return (gpio_get_value(IMX_GPIO_NR(4, 10)) & (1 << 10));
}

void power_off(void){
	gpio_set_value(IMX_GPIO_NR(2, 4), 0);
}

int board_early_init_f(void)
{
	setup_iomux_uart();

#ifdef CONFIG_MAX7310_IOEXP
	/*Reset gpio expander at early stage*/
	reset_max7310();
#endif

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif

#ifdef CONFIG_SYS_USE_SPINOR
	setup_spinor();
#endif

#ifdef CONFIG_SYS_USE_EIMNOR
	setup_eimnor();
#endif

#ifdef CONFIG_SYS_USE_NAND
	setup_gpmi_nand();
#endif

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif
	return 0;
}

int board_init(void)
{
	imx_iomux_v3_setup_multiple_pads(pwr_key_pads,
					 ARRAY_SIZE(pwr_key_pads));
	if(get_power_key_signal())
		//Can's requirement for pull up SYS_ON_CTL
		gpio_direction_output(IMX_GPIO_NR(2, 4), 1);
	else
		gpio_direction_output(IMX_GPIO_NR(2, 4), 0);
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{NULL,   0},
};
#endif

int board_late_init(void)
{
	int ret;
#ifdef CONFIG_CMD_BMODE
	printf("  %s: add_board_boot_modes was called\n", __FUNCTION__);
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_SYS_I2C_MXC
	printf("  %s: setup_i2c was called Speed:%u\n", 
		__FUNCTION__, CONFIG_SYS_I2C_SPEED);

	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	printf("  %s: setup_pmic_voltages was called\n", __FUNCTION__);
	ret = setup_pmic_voltages();
	if (ret)
		return -1;
#endif

#ifdef CONFIG_MAX7310_IOEXP
	printf("  %s: setup_max7310 was called\n", __FUNCTION__);
	setup_max7310();
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	printf("  %s: board_late_mmc_env_init was called\n", __FUNCTION__);
	board_late_mmc_env_init();
#endif

	printf("  %s: gpio_direction_output was called\n", __FUNCTION__);
	gpio_direction_output(IMX_GPIO_NR(4, 14), 1);
	gpio_direction_output(IMX_GPIO_NR(4, 15), 1);
	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY

#define GPIO_VOL_DN_KEY IMX_GPIO_NR(5, 14)
iomux_v3_cfg_t const recovery_key_pads[] = {
	(MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

int is_recovery_key_pressing(void)
{
	int button_pressed = 0;

	/* Check Recovery Combo Button press or not. */
	imx_iomux_v3_setup_multiple_pads(recovery_key_pads,
		ARRAY_SIZE(recovery_key_pads));

	gpio_request(GPIO_VOL_DN_KEY, "volume_dn_key");
	gpio_direction_input(GPIO_VOL_DN_KEY);

	if (gpio_get_value(GPIO_VOL_DN_KEY) == 0) { /* VOL_DN key is low assert */
		button_pressed = 1;
		printf("Recovery key pressed\n");
	}

	return  button_pressed;
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/

void board_fastboot_setup(void)
{
	switch (get_boot_device()) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "sata");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "boota sata");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)

	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc0");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc0");
		break;
	case SD4_BOOT:
	case MMC4_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc1");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc1");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
#if defined(CONFIG_FASTBOOT_STORAGE_NAND)
	case NAND_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "nand");
		if (!getenv("fbparts"))
			setenv("fbparts",
			 "16m@16m(boot) 16m@32m(recovery) 810m@48m(android_root)ubifs");
		if (!getenv("bootcmd"))
			setenv("bootcmd",
				"nand read ${loadaddr} ${boot_nand_offset} "
				"${boot_nand_size};boota ${loadaddr}");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_NAND*/
	default:
		printf("unsupported boot devices\n");
		break;
	}
}

#ifdef CONFIG_ANDROID_RECOVERY

int check_recovery_cmd_file(void)
{
	int button_pressed = 0;
	int recovery_mode = 0;

	recovery_mode = recovery_check_and_clean_flag();

	/* Check Recovery Combo Button press or not. */
	imx_iomux_v3_setup_multiple_pads(recovery_key_pads,
		ARRAY_SIZE(recovery_key_pads));

	gpio_direction_input(GPIO_VOL_DN_KEY);

	if (gpio_get_value(GPIO_VOL_DN_KEY) == 0) { /* VOL_DN key is low assert */
		button_pressed = 1;
		printf("Recovery key pressed\n");
	}

	return recovery_mode || button_pressed;
}

void board_recovery_setup_mf0300(void)
{
	int bootdev = get_boot_device();

	switch (bootdev) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery", "boota sata recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD1_BOOT:
	case MMC1_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery", "boota mmc0 recovery");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery", "boota mmc1 recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
#if defined(CONFIG_FASTBOOT_STORAGE_NAND)
	case NAND_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"nand read ${loadaddr} ${recovery_nand_offset} "
				"${recovery_nand_size};boota ${loadaddr}");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_NAND*/
	case 13:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery", "boota mmc1 recovery");
		break;
	default:
		printf("Unsupported bootup device for recovery: dev: %d\n",
			bootdev);
		return;
	}

	printf("setup env for recovery..\n");
	setenv("bootcmd", "run bootcmd_android_recovery");
}
#endif /*CONFIG_ANDROID_RECOVERY*/

#endif /*CONFIG_FASTBOOT*/

int checkboard(void)
{
	int rev = mx6sabre_rev();
	char *revname;

	switch (rev) {
	case BOARD_REV_B:
		revname = "B";
		break;
	case BOARD_REV_A:
	default:
		revname = "A";
		break;
	}

	printf("Board: MX6-MF0300 rev%s\n", revname);
	return 0;
}

#ifdef CONFIG_IMX_UDC
iomux_v3_cfg_t const otg_udc_pads[] = {
	(MX6_PAD_ENET_RX_ER__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL)),
};
void udc_pins_setting(void)
{
	imx_iomux_v3_setup_multiple_pads(otg_udc_pads,
		ARRAY_SIZE(otg_udc_pads));

	/*set daisy chain for otg_pin_id on 6q. for 6dl, this bit is reserved*/
    imx_iomux_set_gpr_register(1, 13, 1, 0);
}

#endif /*CONFIG_IMX_UDC*/

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTG_PWR       IOEXP_GPIO_NR(3, 1)
#define USB_OTG_FLOGC       IOEXP_GPIO_NR(5, 22)

iomux_v3_cfg_t const usb_otg_pads[] = {
	MX6_PAD_ENET_RX_ER__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
			ARRAY_SIZE(usb_otg_pads));

		/*set daisy chain for otg_pin_id on 6q. for 6dl, this bit is reserved*/
                imx_iomux_set_gpr_register(1, 13, 1, 0);
		break;
	case 1:
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return 1;
	}
	return 0;
}

int board_ehci_power(int port, int on)
{

#ifdef CONFIG_SYS_I2C_MXC
	switch (port) {
	case 0:
		if (on){
			gpio_exp_direction_output(USB_OTG_PWR, 1);
			gpio_exp_direction_output(USB_OTG_FLOGC, 1);
		}
		else{
			gpio_exp_direction_output(USB_OTG_PWR, 0);
			gpio_exp_direction_output(USB_OTG_FLOGC, 0);
		}
		break;
	case 1:
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return 1;
	}
#endif // CONFIG_SYS_I2C_MXC
	
	return 0;
}
#endif
