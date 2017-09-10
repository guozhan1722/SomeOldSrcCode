#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "sha1.h"
#include "iw.h"

#undef DEBUG

SECTION(encryption);

enum encryption_cmd {
	ENCRYPT_CMD_RESET = 0,
	ENCRYPT_CMD_SET_ALG,
	ENCRYPT_CMD_SET_NEW_KEY,
	ENCRYPT_CMD_SET_KEY_INDEX,
	ENCRYPT_CMD_COMMIT
};

enum wpa_alg {
	WPA_ALG_NONE = 0,
	WPA_ALG_WEP,
	WPA_ALG_TKIP,
	WPA_ALG_CCMP,
	WPA_ALG_IGTK,
	WPA_ALG_PMK
};

#define WEP_KEY_NUM 4
#define WLAN_MAX_KEY_LEN		32

#define WLAN_CIPHER_SUITE_NONE		0x00000000
#define WLAN_CIPHER_SUITE_WEP40		0x000FAC01
#define WLAN_CIPHER_SUITE_TKIP		0x000FAC02
#define WLAN_CIPHER_SUITE_CCMP		0x000FAC04
#define WLAN_CIPHER_SUITE_WEP104	0x000FAC05
#define WLAN_CIPHER_SUITE_AES_CMAC	0x000FAC06

struct encryption_key_struct
{
    enum encryption_cmd cmd;
    enum wpa_alg alg;
    __u8 key[WLAN_MAX_KEY_LEN][WEP_KEY_NUM];
    int key_len[WEP_KEY_NUM];
    int key_index;
};

static struct encryption_key_struct encryption_key;

static int handle_encryption_commit(struct nl80211_state *state,
                               struct nl_cb *cb,
                               struct nl_msg *msg,
                               int argc, char **argv)
{
    int cipher;
    int key_len;
    __u8 *key;
    int i;

    switch(encryption_key.alg) {
    case WPA_ALG_NONE:
        break;
    case WPA_ALG_WEP:
        key_len = encryption_key.key_len[encryption_key.key_index];
        key = encryption_key.key[encryption_key.key_index];

        if(key_len >= 13) {
            cipher=WLAN_CIPHER_SUITE_WEP104;
            key_len = 13;
            encryption_key.key_len[encryption_key.key_index] = 13;
            key[13] = '\0';
        }
        else
        {
            cipher=WLAN_CIPHER_SUITE_WEP40;
            key_len = 5;
            encryption_key.key_len[encryption_key.key_index] = 5;

            // padding with space if the key is shorter than the shortest key required
            for(i=strlen(key); i<5; i++) {
                key[i] = ' ';
            }
            key[5] = '\0';
        }
        break;

    case WPA_ALG_TKIP:
        cipher=WLAN_CIPHER_SUITE_TKIP;
        break;

    case WPA_ALG_CCMP:
        cipher=WLAN_CIPHER_SUITE_CCMP;
        break;

    default:
        printf("Encryption Algorithm %d is not supported\n", encryption_key.alg);
        return 0;
    }

    encryption_key.cmd = ENCRYPT_CMD_COMMIT;

    NLA_PUT(msg, NL80211_ATTR_TESTDATA, sizeof(struct encryption_key_struct), &encryption_key);
    return 0;

nla_put_failure:
    return -ENOBUFS;
}
COMMAND(encryption, commit , NULL,
	NL80211_CMD_ENCRYPTION_CIPHER, 0, CIB_NETDEV, handle_encryption_commit, 
	"commit the change to the hardware");

static int handle_encryption_reset(struct nl80211_state *state,
			       struct nl_cb *cb,
			       struct nl_msg *msg,
			       int argc, char **argv)
{
#ifdef DEBUG
    printf("BS DEBUG: argc=%d, argv=%s\n", argc, argv[0]);
#endif
	
    memset(&encryption_key, 0, sizeof(struct encryption_key_struct));

    encryption_key.cmd = ENCRYPT_CMD_RESET;

    NLA_PUT(msg, NL80211_ATTR_TESTDATA, sizeof(struct encryption_key_struct), &encryption_key);

	return 0;

nla_put_failure:
    return -ENOBUFS;
}

COMMAND(encryption, reset , NULL,
	NL80211_CMD_ENCRYPTION_CIPHER, 0, CIB_NETDEV, handle_encryption_reset, 
	"reset the encryption configuration buffer");

static int handle_encryption_alg(struct nl80211_state *state,
                               struct nl_cb *cb,
                               struct nl_msg *msg,
                               int argc, char **argv)
{

   	/* strip "wlan0 encryption alg" */
	// argc -= 3;
	// argv += 3;

#ifdef DEBUG
    printf("BS DEBUG: argc=%d, argv=%s\n", argc, argv[0]);
#endif
    if (argc < 1)	return 1;

    if (strcmp("none", argv[0]) == 0)
        encryption_key.alg = WPA_ALG_NONE;
    else if (strcmp("wep", argv[0]) == 0)
        encryption_key.alg = WPA_ALG_WEP;
    else if (strcmp("tkip", argv[0]) == 0)
        encryption_key.alg = WPA_ALG_TKIP;
    else if (strcmp("ccmp", argv[0]) == 0)
        encryption_key.alg = WPA_ALG_CCMP;
    else return 1;

    argc--;
    argv++;
    if (argc)
        return 1;

    encryption_key.cmd = ENCRYPT_CMD_SET_ALG;

    NLA_PUT(msg, NL80211_ATTR_TESTDATA, sizeof(struct encryption_key_struct), &encryption_key);

    return 0;

nla_put_failure:
    return -ENOBUFS;
}
COMMAND(encryption, alg , "<none,wep,tkip,ccmp>",
	NL80211_CMD_ENCRYPTION_CIPHER, 0, CIB_NETDEV, handle_encryption_alg, 
	"set encryption algorithm none/wep/tkip/ccmp etc.");

static int handle_encryption_key_index(struct nl80211_state *state,
                               struct nl_cb *cb,
                               struct nl_msg *msg,
                               int argc, char **argv)
{
    /* strip "wlan0 encryption key_index" */
	// argc -= 3;
	// argv += 3;

    if (argc < 1)	return 1;

    if (strcmp("1", argv[0]) == 0)
        encryption_key.key_index = 0;
    else if (strcmp("2", argv[0]) == 0)
        encryption_key.key_index = 1;
    else if (strcmp("3", argv[0]) == 0)
        encryption_key.key_index = 2;
    else if (strcmp("4", argv[0]) == 0)
        encryption_key.key_index = 3;
    else return 1;

    argc--;
    argv++;
    if (argc)
        return 1;

    encryption_key.cmd = ENCRYPT_CMD_SET_KEY_INDEX;

    NLA_PUT(msg, NL80211_ATTR_TESTDATA, sizeof(struct encryption_key_struct), &encryption_key);

    return 0;

nla_put_failure:
    return -ENOBUFS;
}
COMMAND(encryption, key_index , "<1,2,3,4>",
	NL80211_CMD_ENCRYPTION_CIPHER, 0, CIB_NETDEV, handle_encryption_key_index, 
	"set encryption default key index 1..4");

static int pbkdf2_sha1_f(const char *passphrase, const char *ssid,
			 size_t ssid_len, int iterations, unsigned int count,
			 u8 *digest)
{
	unsigned char tmp[SHA1_MAC_LEN], tmp2[SHA1_MAC_LEN];
	int i, j;
	unsigned char count_buf[4];
	const u8 *addr[2];
	size_t len[2];
	size_t passphrase_len = os_strlen(passphrase);

#ifdef DEBUG
    printf("BS DEBUG: passphrase=%s, ssid=%s\n", passphrase, ssid);
#endif

	addr[0] = (u8 *) ssid;
	len[0] = ssid_len;
	addr[1] = count_buf;
	len[1] = 4;

	/* F(P, S, c, i) = U1 xor U2 xor ... Uc
	 * U1 = PRF(P, S || i)
	 * U2 = PRF(P, U1)
	 * Uc = PRF(P, Uc-1)
	 */

	count_buf[0] = (count >> 24) & 0xff;
	count_buf[1] = (count >> 16) & 0xff;
	count_buf[2] = (count >> 8) & 0xff;
	count_buf[3] = count & 0xff;
	if (hmac_sha1_vector((u8 *) passphrase, passphrase_len, 2, addr, len,
			     tmp))
		return -1;
	os_memcpy(digest, tmp, SHA1_MAC_LEN);

	for (i = 1; i < iterations; i++) {
		if (hmac_sha1((u8 *) passphrase, passphrase_len, tmp,
			      SHA1_MAC_LEN, tmp2))
			return -1;
		os_memcpy(tmp, tmp2, SHA1_MAC_LEN);
		for (j = 0; j < SHA1_MAC_LEN; j++)
			digest[j] ^= tmp2[j];
	}

	return 0;
}


/**
 * pbkdf2_sha1 - SHA1-based key derivation function (PBKDF2) for IEEE 802.11i
 * @passphrase: ASCII passphrase
 * @ssid: SSID
 * @ssid_len: SSID length in bytes
 * @iterations: Number of iterations to run
 * @buf: Buffer for the generated key
 * @buflen: Length of the buffer in bytes
 * Returns: 0 on success, -1 of failure
 *
 * This function is used to derive PSK for WPA-PSK. For this protocol,
 * iterations is set to 4096 and buflen to 32. This function is described in
 * IEEE Std 802.11-2004, Clause H.4. The main construction is from PKCS#5 v2.0.
 */
int pbkdf2_sha1(const char *passphrase, const char *ssid, size_t ssid_len,
		int iterations, u8 *buf, size_t buflen)
{
	unsigned int count = 0;
	unsigned char *pos = buf;
	size_t left = buflen, plen;
	unsigned char digest[SHA1_MAC_LEN];

	while (left > 0) {
		count++;
		if (pbkdf2_sha1_f(passphrase, ssid, ssid_len, iterations,
				  count, digest))
			return -1;
		plen = left > SHA1_MAC_LEN ? SHA1_MAC_LEN : left;
		os_memcpy(pos, digest, plen);
		pos += plen;
		left -= plen;
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Parse a key from the command line.
 * Return size of the key, or 0 (no key) or -1 (error)
 * If the key is too long, it's simply truncated...
 */
static int iw_in_key(const char *input, unsigned char *key)
{
    int keylen = 0;

#ifdef DEBUG
    printf("BS DEBUG: input=%s\n", input);
#endif

    /* Check the type of key */
    if(!strncmp(input, "s:", 2))
    {
        /* First case : as an ASCII string (Lucent/Agere cards) */
        keylen = strlen(input + 2);		/* skip "s:" */
        if(keylen > WLAN_MAX_KEY_LEN)
            keylen = WLAN_MAX_KEY_LEN;
        memcpy(key, input + 2, keylen);
#ifdef DEBUG
    printf("BS DEBUG: keylen=%d, key=%s\n", keylen, key);
#endif

    }
    else if(!strncmp(input, "p:", 2))
    {

        /* Second case : as a passphrase (PrismII cards) */
        __u8 ssid[] = "IEEE";
        pbkdf2_sha1(input + 2, ssid, os_strlen(ssid), 4096, key, 32);
        keylen = 32;

#ifdef DEBUG
        {
            int i = 0;
            for(i=0; i<32; i++) {
                printf("%02X", key[i]);
            }
            printf("\n");
        }
#endif
    }
    else
    {
        const char *p;
        int		dlen;	/* Digits sequence length */
        unsigned char	out[WLAN_MAX_KEY_LEN];

        /* Third case : as hexadecimal digits */
        p = input;
        dlen = -1;

        /* Loop until we run out of chars in input or overflow the output */
        while(*p != '\0')
        {
            int	temph;
            int	templ;
            int	count;
            /* No more chars in this sequence */
            if(dlen <= 0)
            {
                /* Skip separator */
                if(dlen == 0)
                    p++;
                /* Calculate num of char to next separator */
                dlen = strcspn(p, "-:;.,");
            }
            /* Get each char separatly (and not by two) so that we don't
            * get confused by 'enc' (=> '0E'+'0C') and similar */
            count = sscanf(p, "%1X%1X", &temph, &templ);
            if(count < 1)
                return(-1);		/* Error -> non-hex char */
            /* Fixup odd strings such as '123' is '01'+'23' and not '12'+'03'*/
            if(dlen % 2)
                count = 1;
            /* Put back two chars as one byte and output */
            if(count == 2)
                templ |= temph << 4;
            else
                templ = temph;
            out[keylen++] = (unsigned char) (templ & 0xFF);
            /* Check overflow in output */
            if(keylen >= WLAN_MAX_KEY_LEN)
                break;
            /* Move on to next chars */
            p += count;
            dlen -= count;
        }
	/* We use a temporary output buffer 'out' so that if there is
	 * an error, we don't overwrite the original key buffer.
	 * Because of the way iwconfig loop on multiple key/enc arguments
	 * until it finds an error in here, this is necessary to avoid
	 * silently corrupting the encryption key... */
        memcpy(key, out, keylen);
    }

    return(keylen);
}

static int handle_encryption_new_key(struct nl80211_state *state,
                               struct nl_cb *cb,
                               struct nl_msg *msg,
                               int argc, char **argv)
{
    /* strip "wlan0 encryption new_key" */
    // argc -= 3;
    // argv += 3;

#ifdef DEBUG
    printf("BS DEBUG: argc=%d, argv=%s\n", argc, argv[0]);
#endif

    if (argc < 1)	return 1;

    if (strcmp("1", argv[0]) == 0)
        encryption_key.key_index = 0;
    else if (strcmp("2", argv[0]) == 0)
        encryption_key.key_index = 1;
    else if (strcmp("3", argv[0]) == 0)
        encryption_key.key_index = 2;
    else if (strcmp("4", argv[0]) == 0)
        encryption_key.key_index = 3;
    else return 1;

    argc--;
    argv++;

#ifdef DEBUG
    printf("BS DEBUG: argc=%d, argv=%s\n", argc, argv[0]);
#endif

    if (argc < 1)	return 1;

    encryption_key.key_len[encryption_key.key_index] = iw_in_key(argv[0], &(encryption_key.key[encryption_key.key_index]));

    argc--;
    argv++;
    if (argc)
        return 1;

    encryption_key.cmd = ENCRYPT_CMD_SET_NEW_KEY;

    NLA_PUT(msg, NL80211_ATTR_TESTDATA, sizeof(struct encryption_key_struct), &encryption_key);

    return 0;

nla_put_failure:
    return -ENOBUFS;
}
COMMAND(encryption, new_key , "<keyindex, key>",
	NL80211_CMD_ENCRYPTION_CIPHER, 0, CIB_NETDEV, handle_encryption_new_key, 
	"specify encryption key with keyindex and key. 's:' prefix as ASCII 'p:' prefix as passphrase");

