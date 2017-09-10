#include <linux/module.h>

MODULE_AUTHOR("Luis R. Rodriguez");
MODULE_DESCRIPTION("Kernel compatibility module");
MODULE_LICENSE("GPL");

static int __init compat_init(void)
{
	/* pm-qos for kernels <= 2.6.24, this is a no-op on newer kernels */
	compat_pm_qos_power_init();
	printk(KERN_INFO "Generic kernel compatibility enabled based on "
	       "linux-next next-20100113\n");

        return 0;
}
module_init(compat_init);

static void __exit compat_exit(void)
{
	compat_pm_qos_power_deinit();

        return;
}
module_exit(compat_exit);

#ifdef CONFIG_TARGET_ixp4xx_generic_VIP421EXP49G
xxx
#endif

dddddd