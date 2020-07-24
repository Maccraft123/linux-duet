// SPDX-License-Identifier: GPL-2.0
//
// Copyright (c) 2020 MediaTek Inc.

#include <linux/device.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/soc/mediatek/mtk-mmsys.h>

#define DISP_OVL0_MOUT_EN		0xf00
#define DISP_OVL0_2L_MOUT_EN		0xf04
#define DISP_OVL1_2L_MOUT_EN		0xf08
#define DISP_DITHER0_MOUT_EN		0xf0c
#define DISP_PATH0_SEL_IN		0xf24
#define DISP_DSI0_SEL_IN		0xf2c
#define DISP_DPI0_SEL_IN		0xf30
#define DISP_RDMA0_SOUT_SEL_IN		0xf50
#define DISP_RDMA1_SOUT_SEL_IN		0xf54

#define OVL0_MOUT_EN_OVL0_2L			BIT(4)
#define OVL0_2L_MOUT_EN_DISP_PATH0		BIT(0)
#define OVL1_2L_MOUT_EN_RDMA1			BIT(4)
#define DITHER0_MOUT_IN_DSI0			BIT(0)
#define DISP_PATH0_SEL_IN_OVL0_2L		0x1
#define DSI0_SEL_IN_RDMA0			0x1
#define DSI0_SEL_IN_RDMA1			0x3
#define DPI0_SEL_IN_RDMA0			0x1
#define DPI0_SEL_IN_RDMA1			0x2
#define RDMA0_SOUT_COLOR0			0x1
#define RDMA1_SOUT_DSI0				0x1

struct mmsys_path_sel {
	enum mtk_ddp_comp_id cur;
	enum mtk_ddp_comp_id next;
	u32 addr;
	u32 val;
};

static struct mmsys_path_sel mmsys_mout_en[] = {
	{
		DDP_COMPONENT_OVL0, DDP_COMPONENT_OVL_2L0,
		DISP_OVL0_MOUT_EN, OVL0_MOUT_EN_OVL0_2L,
	},
	{
		DDP_COMPONENT_OVL_2L0, DDP_COMPONENT_RDMA0,
		DISP_OVL0_2L_MOUT_EN, OVL0_2L_MOUT_EN_DISP_PATH0,
	},
	{
		DDP_COMPONENT_OVL_2L1, DDP_COMPONENT_RDMA1,
		DISP_OVL1_2L_MOUT_EN, OVL1_2L_MOUT_EN_RDMA1,
	},
	{
		DDP_COMPONENT_DITHER, DDP_COMPONENT_DSI0,
		DISP_DITHER0_MOUT_EN, DITHER0_MOUT_IN_DSI0,
	},
};

static struct mmsys_path_sel mmsys_sel_in[] = {
	{
		DDP_COMPONENT_OVL_2L0, DDP_COMPONENT_RDMA0,
		DISP_PATH0_SEL_IN, DISP_PATH0_SEL_IN_OVL0_2L,
	},
	{
		DDP_COMPONENT_RDMA1, DDP_COMPONENT_DPI0,
		DISP_DPI0_SEL_IN, DPI0_SEL_IN_RDMA1,
	},
};

static struct mmsys_path_sel mmsys_sout_sel[] = {
	{
		DDP_COMPONENT_RDMA0, DDP_COMPONENT_COLOR0,
		DISP_RDMA0_SOUT_SEL_IN, RDMA0_SOUT_COLOR0,
	},
};

static unsigned int mtk_mmsys_ddp_mout_en(enum mtk_ddp_comp_id cur,
					  enum mtk_ddp_comp_id next,
					  unsigned int *addr)
{
	u32 i;
	u32 val = 0;
	struct mmsys_path_sel *path;

	for (i = 0; i < ARRAY_SIZE(mmsys_mout_en); i++) {
		path = &mmsys_mout_en[i];
		if (cur == path->cur && next == path->next) {
			*addr = path->addr;
			val = path->val;
			break;
		}
	}

	return val;
}

static unsigned int mtk_mmsys_ddp_sel_in(enum mtk_ddp_comp_id cur,
					 enum mtk_ddp_comp_id next,
					 unsigned int *addr)
{
	u32 i;
	u32 val = 0;
	struct mmsys_path_sel *path;

	for (i = 0; i < ARRAY_SIZE(mmsys_sel_in); i++) {
		path = &mmsys_sel_in[i];
		if (cur == path->cur && next == path->next) {
			*addr = path->addr;
			val = path->val;
			break;
		}
	}

	return val;
}

static void mtk_mmsys_ddp_sout_sel(void __iomem *config_regs,
				   enum mtk_ddp_comp_id cur,
				   enum mtk_ddp_comp_id next)
{
	u32 i;
	u32 val = 0;
	u32 addr = 0;
	struct mmsys_path_sel *path;

	for (i = 0; i < ARRAY_SIZE(mmsys_sout_sel); i++) {
		path = &mmsys_sout_sel[i];
		if (cur == path->cur && next == path->next) {
			addr = path->addr;
			val = path->val;
			break;
		}
	}

	if (val)
		writel_relaxed(val, config_regs + addr);
}

static struct mtk_mmsys_conn_funcs mmsys_funcs = {
	.mout_en = mtk_mmsys_ddp_mout_en,
	.sel_in = mtk_mmsys_ddp_sel_in,
	.sout_sel = mtk_mmsys_ddp_sout_sel,
};

static int mmsys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	mtk_mmsys_register_conn_funcs(dev->parent, &mmsys_funcs);

	return 0;
}

static struct platform_driver mmsys_drv = {
	.probe = mmsys_probe,
	.driver = {
		.name = "mt8183-mmsys",
	},
};

builtin_platform_driver(mmsys_drv);
