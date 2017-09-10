#ifndef LINUX_26_29_COMPAT_H
#define LINUX_26_29_COMPAT_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/usb.h>

/**
 *	skb_queue_is_first - check if skb is the first entry in the queue
 *	@list: queue head
 *	@skb: buffer
 *
 *	Returns true if @skb is the first buffer on the list.
 */
static inline bool skb_queue_is_first(const struct sk_buff_head *list,
				      const struct sk_buff *skb)
{
	return (skb->prev == (struct sk_buff *) list);
}

/**
 *	skb_queue_prev - return the prev packet in the queue
 *	@list: queue head
 *	@skb: current buffer
 *
 *	Return the prev packet in @list before @skb.  It is only valid to
 *	call this if skb_queue_is_first() evaluates to false.
 */
static inline struct sk_buff *skb_queue_prev(const struct sk_buff_head *list,
					     const struct sk_buff *skb)
{
	/* This BUG_ON may seem severe, but if we just return then we
	 * are going to dereference garbage.
	 */
	BUG_ON(skb_queue_is_first(list, skb));
	return skb->prev;
}


static inline struct net_device_stats *dev_get_stats(struct net_device *dev)
{
	return dev->get_stats(dev);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
#if defined(CONFIG_USB) || defined(CONFIG_USB_MODULE)
extern void usb_unpoison_anchored_urbs(struct usb_anchor *anchor);
#endif /* CONFIG_USB */
#endif

#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(divisor) __divisor = divisor;		\
	(((x) + ((__divisor) / 2)) / (__divisor));	\
}							\
)

extern int eth_mac_addr(struct net_device *dev, void *p);

#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)) */

#endif /*  LINUX_26_29_COMPAT_H */
