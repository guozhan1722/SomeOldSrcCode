#include <linux/module.h>
#include <net/devmdf.h>

MODULE_AUTHOR("Luis R. Rodriguez");
MODULE_DESCRIPTION("Kernel compatibility module");
MODULE_LICENSE("GPL");

#ifndef COMPAT_BASE_TREE
#error "You need a COMPAT_BASE_TREE"
#endif

#ifndef COMPAT_BASE_TREE_VERSION
#error "You need a COMPAT_BASE_TREE_VERSION"
#endif

#ifndef COMPAT_VERSION
#error "You need a COMPAT_VERSION"
#endif

static char *compat_base_tree = COMPAT_BASE_TREE;
static char *compat_base_tree_version = COMPAT_BASE_TREE_VERSION;
static char *compat_version = COMPAT_VERSION;

module_param(compat_base_tree, charp, 0400);
MODULE_PARM_DESC(compat_base_tree,
		 "The upstream tree used as base for this backport");

module_param(compat_base_tree_version, charp, 0400);
MODULE_PARM_DESC(compat_base_tree_version,
		 "The git-describe of the upstream base tree");

module_param(compat_version, charp, 0400);
MODULE_PARM_DESC(compat_version,
		 "Version of the kernel compat backport work");

#include "devmdf.c"

static u8 MeshMacBlack[MESHMACSIZE][ETH_ALEN] = {0};
static int McastRate = -1;

void SetMcastRate(int rate)
{
    McastRate = rate;
}
EXPORT_SYMBOL(SetMcastRate);

int GetMcastRate(void)
{
    return McastRate;
}
EXPORT_SYMBOL(GetMcastRate);

void AddMeshMacBlack(char *Mac)
{
    int i=0;
    char empty[ETH_ALEN] = {0};

    for(i=0; i<MESHMACSIZE; i++ ) {
        if(memcmp(empty, MeshMacBlack[i], ETH_ALEN) == 0) break;   
        if(memcmp(Mac, MeshMacBlack[i], ETH_ALEN) == 0) return;   
    }
    if(i != MESHMACSIZE) memcpy(MeshMacBlack[i], Mac, ETH_ALEN); 
    else  printk(KERN_INFO "MeshMacBlack full!\n");
}

EXPORT_SYMBOL(AddMeshMacBlack);

int LookupMeshMacBlack(char *Mac)
{
    int i=0;
    char empty[ETH_ALEN] = {0};

    for(i=0; i<MESHMACSIZE; i++ ) {
        if(memcmp(MeshMacBlack[i], empty, ETH_ALEN) == 0) return -1; 
        if(memcmp(MeshMacBlack[i], Mac, ETH_ALEN) == 0) return 0;   // mach
    }
    return -1;
}
EXPORT_SYMBOL(LookupMeshMacBlack);


static int __init compat_init(void)
{
	/* pm-qos for kernels <= 2.6.24, this is a no-op on newer kernels */

    mdfoffset_init();
	memset(MeshMacBlack, 0x00, sizeof(MeshMacBlack));
    compat_pm_qos_power_init();

	printk(KERN_INFO
	       COMPAT_PROJECT " backport release: "
	       COMPAT_VERSION
	       "\n");
	printk(KERN_INFO "Backport based on "
	       COMPAT_BASE_TREE " " COMPAT_BASE_TREE_VERSION
	       "\n");

        return 0;
}
module_init(compat_init);

static void __exit compat_exit(void)
{
    mdfoffset_cleanup();
	compat_pm_qos_power_deinit();
    return;
}
module_exit(compat_exit);

