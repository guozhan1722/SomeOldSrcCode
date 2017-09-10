#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
                     
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "testmode.h"

/* copied from the kernel -- keep in sync */

enum ath5k_testmode_attr {

	__ATH5K_TM_ATTR_INVALID = 0,
	ATH5K_TM_ATTR_CMD = 1,
	ATH5K_TM_ATTR_BWMODE = 2,
    ATH5K_TM_ATTR_ADF = 3,
    ATH5K_TM_ATTR_MCAST = 4,

	/* keep last */

	__ATH5K_TM_ATTR_AFTER_LAST,
	ATH5K_TM_ATTR_MAX = __ATH5K_TM_ATTR_AFTER_LAST - 1

};

enum ath5k_testmode_cmd {
	ATH5K_TM_CMD_SET_BWMODE = 0,
	ATH5K_TM_CMD_GET_BWMODE = 1,
    ATH5K_TM_CMD_SET_ADF = 2,
	ATH5K_TM_CMD_GET_ADF = 3,
    ATH5K_TM_CMD_SET_MCAST = 4,
	ATH5K_TM_CMD_GET_MCAST = 5,
};

/* for ath9k */
enum ath9k_testmode_attr {

	ATH9K_TM_ATTR_INVALID = 0,
	ATH9K_TM_ATTR_CMD = 1,
	ATH9K_TM_ATTR_BWMODE = 2,	/* not implemented */
	ATH9K_TM_ATTR_ADF = 3,		/* not implemented */
	ATH9K_TM_ATTR_MCAST = 4,	/* not implemented */
	ATH9K_TM_ATTR_TXAGGR = 5,	
	ATH9K_TM_ATTR_RXAGGR = 6,
	ATH9K_TM_ATTR_SGI = 7,

	/* keep last */

	__ATH9K_TM_ATTR_AFTER_LAST,
	ATH9K_TM_ATTR_MAX = __ATH9K_TM_ATTR_AFTER_LAST - 1

};

enum ath9k_testmode_cmd {
	ATH9K_TM_CMD_SET_BWMODE = 0,	/* not implemented */
	ATH9K_TM_CMD_GET_BWMODE = 1,	/* not implemented */
	ATH9K_TM_CMD_SET_ADF = 2,	/* not implemented */
	ATH9K_TM_CMD_GET_ADF = 3,	/* not implemented */
	ATH9K_TM_CMD_SET_MCAST = 4,	/* not implemented */
	ATH9K_TM_CMD_GET_MCAST = 5,	/* not implemented */
	ATH9K_TM_CMD_TXAGGR = 6,
	ATH9K_TM_CMD_RXAGGR = 7,
	ATH9K_TM_CMD_SGI = 8,
};


static int print_bwmode(struct nl_msg *msg, void *arg)
{

	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	struct nlattr *td[ATH5K_TM_ATTR_MAX + 1];

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_TESTDATA] || !tb[NL80211_ATTR_WIPHY]) {

		printf("no data!\n");
		return NL_SKIP;
	}

	nla_parse(td, ATH5K_TM_ATTR_MAX, nla_data(tb[NL80211_ATTR_TESTDATA]),
			nla_len(tb[NL80211_ATTR_TESTDATA]), NULL);

	if (!td[ATH5K_TM_ATTR_BWMODE]) {

		printf("no BWMODE info\n");
		return NL_SKIP;
	}

	printf("phy#%d bwmode %d\n",
		nla_get_u32(tb[NL80211_ATTR_WIPHY]),
		nla_get_u32(td[ATH5K_TM_ATTR_BWMODE]));

	return NL_SKIP;
}

static int handle_bwmode(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc >= 2)	return 1;

	if (argc == 0) {
		printf("retrieving bwmode\n");

		NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_GET_BWMODE);
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_bwmode, NULL);

		return 0;
	}

	printf("setting bwmode %d\n", atoi(*argv));

	NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_SET_BWMODE);

	NLA_PUT_U32(msg, ATH5K_TM_ATTR_BWMODE, atoi(*argv));

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

static int print_adf(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *td[ATH5K_TM_ATTR_MAX + 1];

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_TESTDATA] || !tb[NL80211_ATTR_WIPHY]) {
		printf("no data!\n");
		return NL_SKIP;
	}

	nla_parse(td, ATH5K_TM_ATTR_MAX, nla_data(tb[NL80211_ATTR_TESTDATA]),
			nla_len(tb[NL80211_ATTR_TESTDATA]), NULL);

	if (!td[ATH5K_TM_ATTR_ADF]) {
		printf("no ADF info\n");
		return NL_SKIP;
	}

	printf("Got phy#%d adf frequency %lldHz\n",
		nla_get_u32(tb[NL80211_ATTR_WIPHY]),
		nla_get_u64(td[ATH5K_TM_ATTR_ADF]));

	return NL_SKIP;

}

static unsigned int decide_scale(char *c)
{
    char i;

    for(i=0; *(c+i); i++) {
        if((*(c+i) == 'K') || (*(c+i) == 'k')) {
            return 1000;
        } else if ((*(c+i) == 'M') || (*(c+i) == 'm')) {
            return 1000000;
        } else if ((*(c+i) == 'G') || (*(c+i) == 'g')) {
            return 1000000000;
        } 
    }
    return 1;
}

static int handle_adfset(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
    unsigned long long temp1;
    unsigned int temp2, scale, tmp;
    char *c, cs;
    

	if (argc != 2)	return 1;
    cs = *(*argv) - '0';
    if(cs == 1) cs =0;
    else if(cs != 1) cs =1;

    c = strstr(*(argv+1), ".");
    if(c) {
        temp1 = atoll(*(argv+1));
        temp2 = atoi(c+1);
        scale = decide_scale(c+1);

        for(tmp=1;;) {
            tmp *= 10;
            if (tmp >temp2) break;
        }
        if (scale>=tmp) {
            temp2 = temp2*(scale/tmp);
        } else {
            temp2 = temp2/(tmp/scale);
        }
        temp1 = temp1*scale + temp2;
 
     } else {
        temp1 = atoll(*(argv+1));
        scale = decide_scale(*(argv+1));
        temp1 = cs*0x200000000 + temp1*scale;           
     }
     printf("Setting frequency for ADF chip%d @%lldHz\n", cs+1, temp1);       
     temp1 += cs*0x200000000;    	

	NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_SET_ADF);
	NLA_PUT_U64(msg, ATH5K_TM_ATTR_ADF, temp1);

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

static int handle_adfget(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc >= 2)	return 1;

	if (argc == 0) {
		printf("retrieving adf frequency\n");
		NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_GET_ADF);
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_adf, NULL);
		return 0;
	}

nla_put_failure:
	return -ENOBUFS;
}

int adf_commands(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc <= 0)
		return 3;

	if (strcmp(*argv, "set") == 0)
		return handle_adfset(cb, msg, argc - 1, argv + 1);
    else if (strcmp(*argv, "get") == 0)
		return handle_adfget(cb, msg, argc - 1, argv + 1);
	else return 1;
}


static int handle_mcastset(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
    unsigned long long temp;
	if (argc >= 2)	return 1;

	printf("Setting mcast rate %d\n", atol(*argv));

	NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_SET_MCAST);
	NLA_PUT_U32(msg, ATH5K_TM_ATTR_MCAST, atol(*argv));

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

static int print_mcast(struct nl_msg *msg, void *arg)
{

	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	struct nlattr *td[ATH5K_TM_ATTR_MAX + 1];

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_TESTDATA] || !tb[NL80211_ATTR_WIPHY]) {
		printf("no data!\n");
		return NL_SKIP;
	}

	nla_parse(td, ATH5K_TM_ATTR_MAX, nla_data(tb[NL80211_ATTR_TESTDATA]),
			nla_len(tb[NL80211_ATTR_TESTDATA]), NULL);

	if (!td[ATH5K_TM_ATTR_MCAST]) {
		printf("no mcast info\n");
		return NL_SKIP;
	}

	printf("Got phy#%d mcast rate %d\n",
		nla_get_u32(tb[NL80211_ATTR_WIPHY]),
		nla_get_u32(td[ATH5K_TM_ATTR_MCAST]));

	return NL_SKIP;
}

static int handle_mcastget(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc >= 2)	return 1;

	if (argc == 0) {
		printf("retrieving mcast rate\n");
		NLA_PUT_U32(msg, ATH5K_TM_ATTR_CMD, ATH5K_TM_CMD_GET_MCAST);
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_mcast, NULL);
		return 0;
	}

nla_put_failure:
	return -ENOBUFS;
}

int mcast_commands(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc <= 0)
		return 3;

	if (strcmp(*argv, "set") == 0)
		return handle_mcastset(cb, msg, argc - 1, argv + 1);
    else if (strcmp(*argv, "get") == 0)
		return handle_mcastget(cb, msg, argc - 1, argv + 1);
    else return 1;
}

/* enable/disable tx aggr */
static int handle_txaggr(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (!(argc == 1))	return 1;

	if (atoi(*argv) == 0) {
		printf("disable txaggr %d\n", atoi(*argv));
	}
	else if(atoi(*argv) == 1) {
		printf("enable txaggr %d\n", atoi(*argv));
	}

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_CMD, ATH9K_TM_CMD_TXAGGR);

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_TXAGGR, atoi(*argv));

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

/* enable/disable rx aggr */
static int handle_rxaggr(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (!(argc == 1))	return 1;

	if (atoi(*argv) == 0) {
		printf("disable rxaggr %d\n", atoi(*argv));
	}
	else if(atoi(*argv) == 1) {
		printf("enable rxaggr %d\n", atoi(*argv));
	}

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_CMD, ATH9K_TM_CMD_RXAGGR);

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_RXAGGR, atoi(*argv));

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

/* enable/disable short GI */
static int handle_sgi(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (!(argc == 1))	return 1;

	if (atoi(*argv) == 0) {
		printf("disable sgi %d\n", atoi(*argv));
	}
	else if(atoi(*argv) == 1) {
		printf("enable sgi %d\n", atoi(*argv));
	}

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_CMD, ATH9K_TM_CMD_SGI);

	NLA_PUT_U32(msg, ATH9K_TM_ATTR_SGI, atoi(*argv));

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

int do_commands(struct nl_cb *cb, struct nl_msg *msg, int argc, char **argv)
{
	if (argc <= 0) return 3;

	if (strcmp(*argv, "bwmode") == 0)
		return handle_bwmode(cb, msg, argc - 1, argv + 1);
    	else if (strcmp(*argv, "adf") == 0)
		return adf_commands(cb, msg, argc - 1, argv + 1);
	else if (strcmp(*argv, "mcast") == 0)
		return mcast_commands(cb, msg, argc - 1, argv + 1);
    	else if (strcmp(*argv, "txaggr") == 0) /* enable/disable tx aggr */
		return handle_txaggr(cb, msg, argc - 1, argv + 1);
    	else if (strcmp(*argv, "rxaggr") == 0) /* enable/disable rx aggr */
		return handle_rxaggr(cb, msg, argc - 1, argv + 1);
    	else if (strcmp(*argv, "sgi") == 0) /* enable/disable sgi */
		return handle_sgi(cb, msg, argc - 1, argv + 1);
	else return 1;
}

#ifndef CONFIG_LIBNL20
/* libnl 2.0 compatibility code */

static inline struct nl_handle *nl_socket_alloc(void)
{
	return nl_handle_alloc();
}

static inline void nl_socket_free(struct nl_sock *h)
{
	nl_handle_destroy(h);
}

static inline int __genl_ctrl_alloc_cache(struct nl_sock *h, struct nl_cache **cache)
{
	struct nl_cache *tmp = genl_ctrl_alloc_cache(h);
	if (!tmp)
		return -ENOMEM;
	*cache = tmp;
	return 0;
}
#define genl_ctrl_alloc_cache __genl_ctrl_alloc_cache
#endif /* CONFIG_LIBNL20 */

static int nl80211_init(struct nl80211_state *state)
{
	int err;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	if (genl_ctrl_alloc_cache(state->nl_sock, &state->nl_cache)) {
		fprintf(stderr, "Failed to allocate generic netlink cache.\n");
		err = -ENOMEM;
		goto out_handle_destroy;
	}

	state->nl80211 = genl_ctrl_search_by_name(state->nl_cache, "nl80211");
	if (!state->nl80211) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_cache_free;
	}

	/*
	 * Enable peek mode so drivers can send large amounts
	 * of data in blobs without problems.
	 */
	nl_socket_enable_msg_peek(state->nl_sock);

	return 0;

 out_cache_free:
	nl_cache_free(state->nl_cache);
 out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

static void nl80211_cleanup(struct nl80211_state *state)
{
	genl_family_put(state->nl80211);
	nl_cache_free(state->nl_cache);
	nl_socket_free(state->nl_sock);
}

static int phy_lookup(char *name)
{
	char buf[200];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0)
		return -1;
	buf[pos] = '\0';
	return atoi(buf);
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static int handle(struct nl80211_state *state, int argc, char **argv)
{
	struct nl_cb *cb;
	struct nl_msg *msg;
	struct nlattr *nest;
	int devidx = 0;
	int err;

	if (!argc) {
		printf("no dev/phy given\n");
		return 1;
	}

	/* CHANGE HERE: you may need to allocate larger messages! */
	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return 2;
	}

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(state->nl80211), 0,
		    0, NL80211_CMD_TESTMODE, 0);

	devidx = if_nametoindex(*argv);
	if (devidx) {
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	} else {
		devidx = phy_lookup(*argv);
		if (devidx < 0) {
			printf("Device not found\n");
			return 1;
		}
		NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, devidx);
	}
	argc--;
	argv++;

	nest = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
	if (!nest)
		return 4;

	err = do_commands(cb, msg, argc, argv);
	nla_nest_end(msg, nest);

	if (err)
		goto out;

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);
 out:
	nl_cb_put(cb);
 out_free_msg:
	nlmsg_free(msg);
	return err;
 nla_put_failure:
	fprintf(stderr, "building message failed\n");
	return 2;
}

int main(int argc, char **argv)
{
	struct nl80211_state state;
	int err;

	/* strip off self */
	argc--;
	argv++;

	err = nl80211_init(&state);
	if (err) return 1;

	err = handle(&state, argc, argv);
	if (err > 0)
		printf("error!\n");
	else if (err < 0)
		printf("error: %d (%s)\n", err, strerror(-err));

	nl80211_cleanup(&state);

	return err;
}
