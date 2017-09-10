/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include "ag71xx.h"

static void ag71xx_phy_link_adjust(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct phy_device *phydev = ag->phy_dev;
	unsigned long flags;
	int status_change = 0;

	spin_lock_irqsave(&ag->lock, flags);

	if (phydev->link) {
		if (ag->duplex != phydev->duplex
		    || ag->speed != phydev->speed) {
			status_change = 1;
		}
	}

	if (phydev->link != ag->link)
		status_change = 1;

	ag->link = phydev->link;
	ag->duplex = phydev->duplex;
	ag->speed = phydev->speed;

	if (status_change)
		ag71xx_link_adjust(ag);

	spin_unlock_irqrestore(&ag->lock, flags);
}

void ag71xx_phy_start(struct ag71xx *ag)
{
	if (ag->phy_dev) {
		phy_start(ag->phy_dev);
	} else {
		struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);

		ag->duplex = pdata->duplex;
		ag->speed = pdata->speed;
		ag->link = 1;
		ag71xx_link_adjust(ag);
	}
}

void ag71xx_phy_stop(struct ag71xx *ag)
{
	if (ag->phy_dev) {
		phy_stop(ag->phy_dev);
	} else {
		ag->duplex = -1;
		ag->link = 0;
		ag->speed = 0;
		ag71xx_link_adjust(ag);
	}
}

static int ag71xx_phy_connect_fixed(struct ag71xx *ag)
{
	struct net_device *dev = ag->dev;
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	int ret = 0;

	/* use fixed settings */
	switch (pdata->speed) {
	case SPEED_10:
	case SPEED_100:
	case SPEED_1000:
		break;
	default:
		printk(KERN_ERR "%s: invalid speed specified\n",
			dev->name);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ag71xx_phy_connect_multi(struct ag71xx *ag)
{
	struct net_device *dev = ag->dev;
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	struct phy_device *phydev = NULL;
	int phy_count = 0;
	int phy_addr;
	int ret = 0;

	for (phy_addr = 0; phy_addr < PHY_MAX_ADDR; phy_addr++) {
		if (!(pdata->phy_mask & (1 << phy_addr)))
			continue;

		if (ag->mii_bus->phy_map[phy_addr] == NULL)
			continue;

		DBG("%s: PHY found at %s, uid=%08x\n",
			dev->name,
			dev_name(&ag->mii_bus->phy_map[phy_addr]->dev),
			ag->mii_bus->phy_map[phy_addr]->phy_id);

		if (phydev == NULL)
			phydev = ag->mii_bus->phy_map[phy_addr];

		phy_count++;
	}

	switch (phy_count) {
	case 0:
		printk(KERN_ERR "%s: no PHY found with phy_mask=%08x\n",
			dev->name, pdata->phy_mask);
		ret = -ENODEV;
		break;
	case 1:
		ag->phy_dev = phy_connect(dev, dev_name(&phydev->dev),
			&ag71xx_phy_link_adjust, 0, pdata->phy_if_mode);

		if (IS_ERR(ag->phy_dev)) {
			printk(KERN_ERR "%s: could not connect to PHY at %s\n",
				dev->name, dev_name(&phydev->dev));
			return PTR_ERR(ag->phy_dev);
		}

		/* mask with MAC supported features */
		if (pdata->has_gbit)
			phydev->supported &= PHY_GBIT_FEATURES;
		else
			phydev->supported &= PHY_BASIC_FEATURES;

		phydev->advertising = phydev->supported;

		printk(KERN_DEBUG "%s: connected to PHY at %s "
			"[uid=%08x, driver=%s]\n",
			dev->name, dev_name(&phydev->dev),
			phydev->phy_id, phydev->drv->name);

		ag->link = 0;
		ag->speed = 0;
		ag->duplex = -1;
		break;

	default:
		printk(KERN_DEBUG "%s: connected to %d PHYs\n",
			dev->name, phy_count);
		ret = ag71xx_phy_connect_fixed(ag);
		break;
	}

	return ret;
}

static int dev_is_class(struct device *dev, void *class)
{
	if (dev->class != NULL && !strcmp(dev->class->name, class))
		return 1;

	return 0;
}

static struct device *dev_find_class(struct device *parent, char *class)
{
	if (dev_is_class(parent, class)) {
		get_device(parent);
		return parent;
	}

	return device_find_child(parent, class, dev_is_class);
}

static struct mii_bus *dev_to_mii_bus(struct device *dev)
{
	struct device *d;

	d = dev_find_class(dev, "mdio_bus");
	if (d != NULL) {
		struct mii_bus *bus;

		bus = to_mii_bus(d);
		put_device(d);

		return bus;
	}

	return NULL;
}

int ag71xx_phy_connect(struct ag71xx *ag)
{
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);

	ag->mii_bus = dev_to_mii_bus(pdata->mii_bus_dev);
	if (ag->mii_bus == NULL) {
		printk(KERN_ERR "%s: unable to find MII bus on device '%s'\n",
			ag->dev->name, dev_name(pdata->mii_bus_dev));
		return -ENODEV;
	}

	/* Reset the mdio bus explicitly */
	if (ag->mii_bus->reset) {
		mutex_lock(&ag->mii_bus->mdio_lock);
		ag->mii_bus->reset(ag->mii_bus);
		mutex_unlock(&ag->mii_bus->mdio_lock);
	}

	if (pdata->phy_mask)
		return ag71xx_phy_connect_multi(ag);

	return ag71xx_phy_connect_fixed(ag);
}

void ag71xx_phy_disconnect(struct ag71xx *ag)
{
	if (ag->phy_dev)
		phy_disconnect(ag->phy_dev);
}
