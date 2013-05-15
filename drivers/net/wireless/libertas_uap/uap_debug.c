/** @file uap_debug.c
 * @brief This file contains functions for debug proc file.
 *
 * Copyright (C) 2008-2009, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available along with the File in the gpl.txt file or by writing to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 *
 */
#ifdef CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include "uap_drv.h"

enum debug_location {
	LOCATION_UAP_DEV,
	LOCATION_ADAPTER,
};

#define debugfs_uap_dev_member(n)		\
	(sizeof((uap_dev_t*) 0)->n),		\
	((ptrdiff_t) &((uap_dev_t*) 0)->n),	\
	LOCATION_UAP_DEV

#define debugfs_adapter_member(n)		\
	(sizeof((uap_adapter*) 0)->n),		\
	((ptrdiff_t) &((uap_adapter*) 0)->n),	\
	LOCATION_ADAPTER

static const struct {
	const char	   *name;
	size_t		    size;
	ptrdiff_t	    offset;
	enum debug_location location;
} debugfs_items[] = {
	{ "cmd_sent",       debugfs_uap_dev_member(cmd_sent)                         },
	{ "data_sent",      debugfs_uap_dev_member(data_sent)                        },
	{ "int_count",      debugfs_adapter_member(IntCounter)                       },
	{ "cmd_pending",    debugfs_adapter_member(cmd_pending)                      },
	{ "ps_mode",        debugfs_adapter_member(psmode)                           },
	{ "ps_state",       debugfs_adapter_member(ps_state)                         },
	{ "cmd_h2c_failed", debugfs_adapter_member(dbg.num_cmd_host_to_card_failure) },
	{ "tx_h2c_failed",  debugfs_adapter_member(dbg.num_tx_host_to_card_failure)  },
};


static struct dentry *debugfs_module_dir = NULL;


void uap_debugfs_add_dev(uap_private *priv)
{
	unsigned i;

	if (!debugfs_module_dir)
		return;

	if (priv->debugfs_dir)
		return;

	if ((priv->debugfs_dir = debugfs_create_dir(netdev_name(priv->uap_dev.netdev), debugfs_module_dir))) {
		for (i = 0; i < sizeof(debugfs_items) / sizeof(debugfs_items[0]); i++) {
			char* address = NULL;

			switch (debugfs_items[i].location) {
				case LOCATION_UAP_DEV: address = (char*) &priv->uap_dev; break;
				case LOCATION_ADAPTER: address = (char*)  priv->adapter; break;
				default: BUG();
			}

			address += debugfs_items[i].offset;

			switch (debugfs_items[i].size) {
				case 1: debugfs_create_x8 (debugfs_items[i].name, 0600, priv->debugfs_dir, (u8 *) address); break;
				case 2: debugfs_create_x16(debugfs_items[i].name, 0600, priv->debugfs_dir, (u16*) address); break;
				case 4: debugfs_create_x32(debugfs_items[i].name, 0600, priv->debugfs_dir, (u32*) address); break;
				default: BUG();
			}
		}
	}
}

void uap_debugfs_remove_dev(uap_private *priv)
{
	if (priv->debugfs_dir) {
		debugfs_remove_recursive(priv->debugfs_dir);
	}
}

void __init uap_debugfs_init(void)
{
	debugfs_module_dir = debugfs_create_dir(KBUILD_MODNAME, NULL);
}

void __exit uap_debugfs_exit(void)
{
	if (debugfs_module_dir)
		debugfs_remove_recursive(debugfs_module_dir);
}

#endif /* CONFIG_DEBUG_FS */
