// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 *
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>
#include <linux/err.h>
#include <linux/delay.h>

#define msleep(a) udelay(a * 1000)

struct st7785_panel_priv {
	struct gpio_desc reset;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
	unsigned int sleep_delay;
};

static const struct display_timing default_timing = {
	.pixelclock.typ		= 7730000,
	.hactive.typ		= 240,
	.hfront_porch.typ	= 66,
	.hback_porch.typ	= 50,
	.hsync_len.typ		= 10,
	.vactive.typ		= 320,
	.vfront_porch.typ	= 22,
	.vback_porch.typ	= 10,
	.vsync_len.typ		= 10,
};

static int st7785_enable(struct udevice *dev)
{
	struct st7785_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	msleep(350);
	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_write(dsi, MIPI_DCS_EXIT_SLEEP_MODE, NULL, 0);
	if (ret < 0) {
		dev_err(dev, "exit_sleep_mode cmd failed ret = %d\n", ret);
		goto power_off;
	}
/*	
	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0x36, 0x00 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 2 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0x3A, 0x66 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 3 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xB2,0x0C,0x0C,0x00,0x33,0x33 }, 6);
	if (ret < 0) {
		dev_err(dev, "cmd 4 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xb7, 0x75 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 5 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xbb, 0x2d }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 6 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc0, 0x2c }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 7 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc2, 0x01 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 8 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc3, 0x13 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 9 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc4, 0x20 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 10 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc5, 0x1c }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 11 = %d\n", ret);
		goto power_off;
	}

	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xc6, 0x0f }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 12 = %d\n", ret);
		goto power_off;
	}


	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xd0, 0xa7 }, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 13 = %d\n", ret);
		goto power_off;
	}
*/
	
	ret = mipi_dsi_dcs_write_buffer(dsi, (u8[]) { 0xB0, 0x11}, 2);
	if (ret < 0) {
		dev_err(dev, "cmd 2 = %d\n", ret);
		goto power_off;
	}



	ret = mipi_dsi_dcs_write(dsi, MIPI_DCS_SET_DISPLAY_ON, NULL, 0);
	if (ret < 0) {
		dev_err(dev, "exit_sleep_mode cmd failed ret = %d\n", ret);
		goto power_off;
	}
	ret = mipi_dsi_dcs_write(dsi, MIPI_DCS_ENTER_INVERT_MODE, NULL, 0);

	if (ret < 0) {
		dev_err(dev, "exit_sleep_mode cmd failed ret = %d\n", ret);
		goto power_off;
	}

	msleep(priv->sleep_delay);

power_off:
	return 0;
}

static int st7785_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	return st7785_enable(dev);
}

static int st7785_panel_get_display_timing(struct udevice *dev,
					    struct display_timing *timings)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	struct st7785_panel_priv *priv = dev_get_priv(dev);

	memcpy(timings, &default_timing, sizeof(*timings));

	/* fill characteristics of DSI data link */
	if (device) {
		device->lanes = priv->lanes;
		device->format = priv->format;
		device->format = MIPI_DSI_FMT_RGB666;
		device->mode_flags = priv->mode_flags;
	}

	return 0;
}

static int st7785_panel_probe(struct udevice *dev)
{
	struct st7785_panel_priv *priv = dev_get_priv(dev);
	int ret;
	u32 video_mode;

	priv->format = MIPI_DSI_FMT_RGB888;
	priv->mode_flags = MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_VIDEO;

	ret = dev_read_u32(dev, "video-mode", &video_mode);
	if (!ret) {
		switch (video_mode) {
		case 0:
			/* burst mode */
			priv->mode_flags |= MIPI_DSI_MODE_VIDEO_BURST;
			break;
		case 1:
			/* non-burst mode with sync event */
			break;
		case 2:
			/* non-burst mode with sync pulse */
			priv->mode_flags |= MIPI_DSI_MODE_VIDEO_SYNC_PULSE;
			break;
		default:
			dev_warn(dev, "invalid video mode %d\n", video_mode);
			break;
		}
	}

	ret = dev_read_u32(dev, "dsi-lanes", &priv->lanes);
	if (ret) {
		printf("Failed to get dsi-lanes property (%d)\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpio", 0, &priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		printf("Warning: cannot get reset GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	/* reset panel */
	ret = dm_gpio_set_value(&priv->reset, true);
	if (ret)
		printf("reset gpio fails to set true\n");
	mdelay(10);
	ret = dm_gpio_set_value(&priv->reset, false);
	if (ret)
		printf("reset gpio fails to set true\n");
	mdelay(10);

	return 0;
}

static int st7785_panel_disable(struct udevice *dev)
{
	struct st7785_panel_priv *priv = dev_get_priv(dev);

	dm_gpio_set_value(&priv->reset, true);

	return 0;
}

static const struct panel_ops st7785_panel_ops = {
	.enable_backlight = st7785_panel_enable_backlight,
	.get_display_timing = st7785_panel_get_display_timing,
};

static const struct udevice_id st7785_panel_ids[] = {
	{ .compatible = "et0240h6dma,st7785" },
	{ }
};

U_BOOT_DRIVER(st7785_panel) = {
	.name			  = "st7785_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = st7785_panel_ids,
	.ops			  = &st7785_panel_ops,
	.probe			  = st7785_panel_probe,
	.remove			  = st7785_panel_disable,
	.plat_auto = sizeof(struct mipi_dsi_panel_plat),
	.priv_auto = sizeof(struct st7785_panel_priv),
};
