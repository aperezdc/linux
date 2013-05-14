/** @file uap_proc.c
  * @brief This file contains functions for proc file.
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
#ifdef CONFIG_PROC_FS
#include "uap_headers.h"
#include <linux/fs.h>


static int uap_hwstatus_write(struct file *file, const char __user *buffer,
			      size_t count, loff_t *pos)
{
	char databuf[11];
	int length = 0;

	if (count > (sizeof(databuf) - 1))
		return -1;

	if (copy_from_user(&databuf, buffer, count))
		return -1;

	databuf[count] = '\0';
	length = strlen(databuf);
	if (databuf[length - 1] == '\n')
		databuf[--length] = '\0';

	if (string_to_number(databuf) == HWReset) {
		uap_private *priv = (uap_private*) PDE_DATA(file_inode(file));
		PRINTM(MSG, "reset hw\n");
		uap_soft_reset(priv);
		priv->adapter->HardwareStatus = HWReset;
		return count;
	}

	return -EINVAL;
}

static int uap_show_hwstatus(struct seq_file *m, void *v)
{
	struct net_device *netdev = m->private;
	uap_private *priv = (uap_private*) netdev_priv(netdev);
	seq_printf(m, "%d\n", priv->adapter->HardwareStatus);
	return 0;
}

static int uap_show_info(struct seq_file *m, void *v)
{
	struct net_device *netdev = m->private;
	uap_private *priv = (uap_private*) netdev_priv(netdev);
	struct netdev_hw_addr *ha;
	uint32_t i;

	seq_printf(m, "driver_name = \"uap\"\n");
	seq_printf(m, "driver_version = %s-(FP%s)\n", DRIVER_VERSION, FPNUM);
	seq_printf(m, "InterfaceName=\"%s\"\n", netdev->name);
	seq_printf(m, "State=\"%s\"\n", ((priv->MediaConnected == FALSE) ? "Disconnected" : "Connected"));
	seq_printf(m, "MACAddress=\"%02x:%02x:%02x:%02x:%02x:%02x\"\n",
		   netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2],
		   netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5]);

	i = 0;
	netdev_for_each_mc_addr(ha, netdev) {
		i++;
	}
	seq_printf(m, "MCCount=\"%d\"\n", i);

	i = 0;
	netdev_for_each_mc_addr(ha, netdev) {
		seq_printf(m, "MCAddr[%d]=\"%02x:%02x:%02x:%02x:%02x:%02x\"\n", i++,
			   ha->addr[0], ha->addr[1], ha->addr[2],
			   ha->addr[3], ha->addr[4], ha->addr[5]);
	}

	seq_printf(m, "num_tx_bytes = %lu\n", priv->stats.tx_bytes);
	seq_printf(m, "num_rx_bytes = %lu\n", priv->stats.rx_bytes);
	seq_printf(m, "num_tx_pkts = %lu\n", priv->stats.tx_packets);
	seq_printf(m, "num_rx_pkts = %lu\n", priv->stats.rx_packets);
	seq_printf(m, "num_tx_pkts_dropped = %lu\n", priv->stats.tx_dropped);
	seq_printf(m, "num_rx_pkts_dropped = %lu\n", priv->stats.rx_dropped);
	seq_printf(m, "num_tx_pkts_err = %lu\n", priv->stats.tx_errors);
	seq_printf(m, "num_rx_pkts_err = %lu\n", priv->stats.rx_errors);
	seq_printf(m, "num_tx_timeout = %u\n", priv->num_tx_timeout);
	seq_printf(m, "carrier %s\n",
		   ((netif_carrier_ok(priv->uap_dev.netdev)) ? "on" : "off"));
	seq_printf(m, "tx queue %s\n",
		   ((netif_queue_stopped(priv->uap_dev.netdev)) ? "stopped" :
		    "started"));
	return 0;
}

static int uap_hwstatus_open(struct inode *inode, struct file *file)
{
	return single_open(file, uap_show_hwstatus, PDE_DATA(inode));
}

static int uap_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, uap_show_info, PDE_DATA(inode));
}

static const struct file_operations info_fops = {
	.open		= uap_info_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations hwstatus_fops = {
	.open		= uap_hwstatus_open,
	.read		= seq_read,
	.write		= uap_hwstatus_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};


/**
 * @brief create uap proc file
 *
 * @param priv	pointer uap_private
 * @param dev	pointer net_device
 */
void
uap_proc_entry(uap_private *priv, struct net_device *dev)
{
	PRINTM(INFO, "Creating Proc Interface\n");
	/* Check if uap directory already exists */
	if (!priv->proc_uap) {
		if (!(priv->proc_uap = proc_mkdir("uap", NULL))) {
			PRINTM(MSG, "Failed to create /proc/<dev>/");
			return;
		}
		if ((priv->proc_entry = proc_mkdir(dev->name, priv->proc_uap))) {
			if (!proc_create_data("info", 0640, priv->proc_entry, &info_fops, dev))
				PRINTM(MSG, "Failed to create /proc/<dev>/info");
			if (!proc_create_data("hwstatus", 0640, priv->proc_entry, &hwstatus_fops, dev))
				PRINTM(MSG, "Failed to create /proc/<dev>/hwstatus");
		}
	}
}

/**
 * @brief remove proc file
 *
 * @param priv	pointer uap_private
 */
void
uap_proc_remove(uap_private *priv)
{
	if (priv->proc_uap) {
		if (priv->proc_entry) {
			remove_proc_entry("info", priv->proc_entry);
			remove_proc_entry("hwstatus", priv->proc_entry);
		}
		remove_proc_entry(priv->uap_dev.netdev->name, priv->proc_uap);
		remove_proc_entry("uap", NULL);
	}
}

/**
 * @brief convert string to number
 *
 * @param s	pointer to numbered string
 * @return	converted number from string s
 */
int
string_to_number(char *s)
{
    int r = 0;
    int base = 0;
    int pn = 1;

    if (strncmp(s, "-", 1) == 0) {
        pn = -1;
        s++;
    }
    if ((strncmp(s, "0x", 2) == 0) || (strncmp(s, "0X", 2) == 0)) {
        base = 16;
        s += 2;
    } else
        base = 10;

    for (s = s; *s != 0; s++) {
        if ((*s >= '0') && (*s <= '9'))
            r = (r * base) + (*s - '0');
        else if ((*s >= 'A') && (*s <= 'F'))
            r = (r * base) + (*s - 'A' + 10);
        else if ((*s >= 'a') && (*s <= 'f'))
            r = (r * base) + (*s - 'a' + 10);
        else
            break;
    }

    return (r * pn);
}

#endif
