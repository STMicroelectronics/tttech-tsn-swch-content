// SPDX-License-Identifier: GPL-2.0-only
/*
 * dwmac-stm32.c - DWMAC Specific Glue layer for STM32 MCU
 *
 * Copyright (C) STMicroelectronics SA 2022
 * Author:  Alexandre Torgue <alexandre.torgue@foss.st.com> for STMicroelectronics.
 */
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define ETH1_DIRECT		0
#define ETHSW_CFG_IF_SEL_RGMII	BIT(1)
#define ETHSW_CLK_SEL_RCC	BIT(2)
#define ETHSW_REF_SEL_RCC	BIT(3)

/* CLOCK feed to PHY*/
#define ETHSW_CK_F_25M	25000000
#define ETHSW_CK_F_50M	50000000
#define ETHSW_CK_F_125M	125000000

struct stm32_deip {
	struct regmap *regmap;
	struct clk *ck_bus_ethsw;
	struct clk *ck_bus_ethsw_acmcfg;
	struct clk *ck_bus_ethsw_acmmsg;
	struct clk *ck_ker_ethsw;
	struct clk *ck_ker_ethswref;
	bool int_phyclk;
	bool int_125;
	bool rmmii_int_ref;
	u32 ethsw_cr;
	phy_interface_t phy_interface;
};

static int stm32_deip_init(struct device *dev, struct stm32_deip *deip)
{
	u32 val = ETH1_DIRECT, clk_rate;
	u32 ret;

	/*  enable bus clock */
	ret = clk_prepare_enable(deip->ck_bus_ethsw);
	if (ret) {
		dev_err(dev, "fail to clock DE-IP core\n");
		return ret;
	}

	if (deip->ck_bus_ethsw_acmcfg && deip->ck_bus_ethsw_acmmsg) {
		ret = clk_prepare_enable(deip->ck_bus_ethsw_acmcfg);
		ret |= clk_prepare_enable(deip->ck_bus_ethsw_acmmsg);
		if (ret) {
			dev_err(dev, "fail to clock ACM IP\n");
		return ret;
		}
	}

	ret = clk_prepare_enable(deip->ck_ker_ethsw);
	if (ret) {
		dev_err(dev, "fail to clock DE-IP\n");
		return ret;
	}

	/* Check if switch clock comes from RCC or PHY (through ETH1_CLK125 pad).
	 * EXT125 selection is not directly linked to phy interface.
	 */
	if (deip->int_125) {
		/* Clock from RCC */
		val |= ETHSW_CLK_SEL_RCC;
		clk_rate = clk_get_rate(deip->ck_ker_ethsw);
		if (clk_rate != ETHSW_CK_F_125M) {
			dev_err(dev, "ck_ker_ethsw not at the well rate\n");
			return -EINVAL;
		}
	}

	switch (deip->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/*  Get internal clock ck_ker_ethswref if no Crystal on PHY */
		if (deip->int_phyclk && deip->ck_ker_ethswref) {
			clk_rate = clk_get_rate(deip->ck_ker_ethswref);
			if (clk_rate != ETHSW_CK_F_25M) {
				dev_err(dev, "ck_ker_ethswref not at the well rate\n");
				return -EINVAL;
			};
		};
		val |= ETHSW_CFG_IF_SEL_RGMII;
		dev_dbg(dev, "ETHSW External ports: RGMII\n");
		break;
	case PHY_INTERFACE_MODE_RMII:
		/* Check if REF clock is provided by RCC or
		 * ETH1_REF_CLK / ETH3_REF_CLK pads.
		 */
		if (deip->rmmii_int_ref && deip->ck_ker_ethswref) {
			/*  Ref clock from RCC */
			val |= ETHSW_REF_SEL_RCC;
			clk_rate = clk_get_rate(deip->ck_ker_ethswref);
			if (clk_rate != ETHSW_CK_F_25M || clk_rate != ETHSW_CK_F_50M) {
				dev_err(dev, "ck_ker_ethswref not at the well rate\n");
				return -EINVAL;
			};
		};
		dev_dbg(dev, "ETHSW External ports: RMII\n");
		break;
	default:
		dev_err(dev, "ETHSW init :  Do not manage %d interface\n",
			deip->phy_interface);
		/* Do not manage others interfaces */
		return -EINVAL;
	}

	return	regmap_write(deip->regmap, deip->ethsw_cr, val);
}

static int stm32_deip_parse_data(struct device *dev, struct stm32_deip *deip)
{
	struct device_node *np = dev->of_node;
	int err;

	/* Get mode register */
	deip->regmap = syscon_regmap_lookup_by_phandle(np, "st,syscon");
	if (IS_ERR(deip->regmap))
		return PTR_ERR(deip->regmap);

	err = of_property_read_u32_index(np, "st,syscon", 1, &deip->ethsw_cr);
	if (err) {
		dev_err(dev, "Can't get sysconfig register (%d)\n", err);
		return err;
	}

	/*  Get ETHSW Bus clock */
	deip->ck_bus_ethsw = devm_clk_get(dev, "ethsw-bus-clk");
	if (IS_ERR(deip->ck_bus_ethsw)) {
		dev_err(dev, "No switch bus clock provided...\n");
		return PTR_ERR(deip->ck_bus_ethsw);
	}

	/*  Get ETHSW ACM CFG Bus clock */
	deip->ck_bus_ethsw_acmcfg = devm_clk_get(dev, "ethswacmcfg-bus-clk");
	if (IS_ERR(deip->ck_bus_ethsw_acmcfg)) {
		deip->ck_bus_ethsw_acmcfg = NULL;
		dev_info(dev, "No acmcfg bus clock provided...\n");
	}

	/*  Get ETHSW ACM MSG Bus clock */
	deip->ck_bus_ethsw_acmmsg = devm_clk_get(dev, "ethswacmmsg-bus-clk");
	if (IS_ERR(deip->ck_bus_ethsw_acmmsg)) {
		deip->ck_bus_ethsw_acmmsg = NULL;
		dev_info(dev, "No acmmsg bus clock provided...\n");
	}

	/* This clock is mandatory to synchronize de-ip reset */
	deip->ck_ker_ethsw = devm_clk_get(dev, "ethsw-clk");
	if (IS_ERR(deip->ck_ker_ethsw)) {
		dev_err(dev, "No ck_ker_ethsw clock provided...\n");
		return PTR_ERR(deip->ck_bus_ethsw);
	}

	/*  Get ETHSW_REF_CLK clocks used for RMII ref clock and/or for PHY
	 *  external clock (when a crystal is not used.
	 */
	deip->ck_ker_ethswref = devm_clk_get(dev, "ethsw-ref-clk");
	if (IS_ERR(deip->ck_ker_ethswref)) {
		dev_info(dev, "No phy clock provided...\n");
		deip->ck_ker_ethswref = NULL;
	}

	/* Ethernet PHY have no crystal */
	deip->int_phyclk = of_property_read_bool(np, "st,ethsw-internal-phyclk");

	/* Switch 125MHz clock selection. */
	deip->int_125 = of_property_read_bool(np, "st,ethsw-internal-125");

	/* RMII clock ref selection. */
	deip->rmmii_int_ref = of_property_read_bool(np, "st,ethsw-rmii-internal-refclk");

	/* Get external ports PHY interfaces */
	deip->phy_interface = device_get_phy_mode(dev);
	if (deip->phy_interface < 0) {
		dev_err(dev, "No phy interfaces provided!\n");
		return deip->phy_interface;
	}

	return err;
}

static int stm32_deip_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct stm32_deip *deip;
	int ret;

	deip = devm_kzalloc(&pdev->dev, sizeof(*deip), GFP_KERNEL);
	if (!deip) {
		ret = -ENOMEM;
		return ret;
	}

	/*  Get glue resources from DT */
	ret = stm32_deip_parse_data(&pdev->dev, deip);
	if (ret)
		return ret;

	ret = stm32_deip_init(&pdev->dev, deip);
	if (ret)
		return ret;

	/* Allocate and initialize the de-ip and acm devices */
	ret = devm_of_platform_populate(dev);
	if (ret)
		return dev_err_probe(dev, ret, "failed to add switch devices\n");

	return ret;
}

static int stm32_deip_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id stm32_deip_match[] = {
	{ .compatible = "st,stm32-deip"},
	{ }
};

MODULE_DEVICE_TABLE(of, stm32_deip_match);

static struct platform_driver stm32_deip_driver = {
	.probe  = stm32_deip_probe,
	.remove = stm32_deip_remove,
	.driver = {
		.name           = "stm32-deip",
		.of_match_table = stm32_deip_match,
	},
};
module_platform_driver(stm32_deip_driver);

MODULE_AUTHOR("Alexandre Torgue <alexandre.torgue@foss.st.com>");
MODULE_DESCRIPTION("STMicroelectronics STM32 TTECH DE-IP Specific Glue layer");
MODULE_LICENSE("GPL v2");
