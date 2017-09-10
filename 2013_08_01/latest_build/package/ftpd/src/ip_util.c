/*
 * $Id: ip_util.c,v 1.1 2010/07/14 22:53:05 Bob Exp $
 *
 * Copyright (C) 1995,1996,1997 Lars Fenneberg
 *
 * Copyright 1992 Livingston Enterprises, Inc.
 *
 * Copyright 1992,1993, 1994,1995 The Regents of the University of Michigan
 * and Merit Network, Inc. All Rights Reserved
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#include <radiusclient_config.h>
#include <radiusclient_includes.h>
#include <radiusclient-ng.h>

#if !defined(SA_LEN)
#define SA_LEN(sa) \
  (((sa)->sa_family == AF_INET) ? \
    sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))
#endif

/*
 * Function: rc_get_ipaddr
 *
 * Purpose: return an IP address in host long notation from a host
 *          name or address in dot notation.
 *
 * Returns: 0 on failure
 */

UINT4 rc_get_ipaddr (char *host)
{
	struct hostent *hp;

	if (rc_good_ipaddr (host) == 0)
	{
		return ntohl(inet_addr (host));
	}
	else if ((hp = gethostbyname (host)) == NULL)
	{
		rc_log(LOG_ERR,"rc_get_ipaddr: couldn't resolve hostname: %s", host);
	    return (UINT4)0;
	}
	return ntohl((*(UINT4 *) hp->h_addr));
}

/*
 * Function: rc_good_ipaddr
 *
 * Purpose: check for valid IP address in standard dot notation.
 *
 * Returns: 0 on success, -1 when failure
 *
 */

int rc_good_ipaddr (char *addr)
{
	int             dot_count;
	int             digit_count;

	if (addr == NULL)
		return -1;

	dot_count = 0;
	digit_count = 0;
	while (*addr != '\0' && *addr != ' ')
	{
		if (*addr == '.')
		{
			dot_count++;
			digit_count = 0;
		}
		else if (!isdigit (*addr))
		{
			dot_count = 5;
		}
		else
		{
			digit_count++;
			if (digit_count > 3)
			{
				dot_count = 5;
			}
		}
		addr++;
	}
	if (dot_count != 3)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/*
 * Function: rc_ip_hostname
 *
 * Purpose: Return a printable host name (or IP address in dot notation)
 *	    for the supplied IP address.
 *
 */

const char *rc_ip_hostname (UINT4 h_ipaddr)
{
	struct hostent  *hp;
	UINT4           n_ipaddr = htonl (h_ipaddr);

	if ((hp = gethostbyaddr ((char *) &n_ipaddr, sizeof (struct in_addr),
			    AF_INET)) == NULL) {
		rc_log(LOG_ERR,"rc_ip_hostname: couldn't look up host by addr: %08lX", h_ipaddr);
	}

	return (hp == NULL) ? "unknown" : hp->h_name;
}

/*
 * Function: rc_getport
 *
 * Purpose: get the port number for the supplied request type
 *
 */

unsigned short rc_getport(int type)
{
	struct servent *svp;

	if ((svp = getservbyname ((type==AUTH)?"radius":"radacct", "udp")) == NULL)
	{
		return (type==AUTH) ? PW_AUTH_UDP_PORT : PW_ACCT_UDP_PORT;
	} else {
		return ntohs ((unsigned short) svp->s_port);
	}
}

/*
 * Function: rc_own_hostname
 *
 * Purpose: get the hostname of this machine
 *
 * Returns  -1 on failure, 0 on success
 *
 */

int
rc_own_hostname(char *hostname, int len)
{
#ifdef HAVE_UNAME
	struct	utsname uts;
#endif

#if defined(HAVE_UNAME)
	if (uname(&uts) < 0)
	{
		rc_log(LOG_ERR,"rc_own_hostname: couldn't get own hostname");
		return -1;
	}
	strncpy(hostname, uts.nodename, len);
#elif defined(HAVE_GETHOSTNAME)
	if (gethostname(hostname, len) < 0)
	{
		rc_log(LOG_ERR,"rc_own_hostname: couldn't get own hostname");
		return -1;
	}
#elif defined(HAVE_SYSINFO)
	if (sysinfo(SI_HOSTNAME, hostname, len) < 0)
	{
		rc_log(LOG_ERR,"rc_own_hostname: couldn't get own hostname");
		return -1;
	}
#else
	return -1;
#endif

	return 0;
}

/*
 * Function: rc_own_ipaddress
 *
 * Purpose: get the IP address of this host in host order
 *
 * Returns: IP address on success, 0 on failure
 *
 */

UINT4 rc_own_ipaddress(rc_handle *rh)
{
	char hostname[256];

	if (!rh->this_host_ipaddr) {
		if (rc_conf_str(rh, "bindaddr") == NULL ||
		    strcmp(rc_conf_str(rh, "bindaddr"), "*") == 0) {
			if (rc_own_hostname(hostname, sizeof(hostname)) < 0)
				return 0;
		} else {
			strncpy(hostname, rc_conf_str(rh, "bindaddr"), sizeof(hostname));
			hostname[sizeof(hostname) - 1] = '\0';
		}
		if ((rh->this_host_ipaddr = rc_get_ipaddr (hostname)) == 0) {
			rc_log(LOG_ERR, "rc_own_ipaddress: couldn't get own IP address");
			return 0;
		}
	}

	return rh->this_host_ipaddr;
}

/*
 * Function: rc_own_bind_ipaddress
 *
 * Purpose: get the IP address to be used as a source address
 *          for sending requests in host order
 *
 * Returns: IP address
 *
 */

UINT4 rc_own_bind_ipaddress(rc_handle *rh)
{
	char hostname[256];
	UINT4 rval;

	if (rh->this_host_bind_ipaddr != NULL)
		return *rh->this_host_bind_ipaddr;

	rh->this_host_bind_ipaddr = malloc(sizeof(*rh->this_host_bind_ipaddr));
	if (rh->this_host_bind_ipaddr == NULL)
		rc_log(LOG_CRIT, "rc_own_bind_ipaddress: out of memory");
	if (rc_conf_str(rh, "bindaddr") == NULL ||
	    strcmp(rc_conf_str(rh, "bindaddr"), "*") == 0) {
		rval = INADDR_ANY;
	} else {
		strncpy(hostname, rc_conf_str(rh, "bindaddr"), sizeof(hostname));
		hostname[sizeof(hostname) - 1] = '\0';
		if ((rval = rc_get_ipaddr (hostname)) == 0) {
			rc_log(LOG_ERR, "rc_own_ipaddress: couldn't get IP address from bindaddr");
			rval = INADDR_ANY;
		}
	}
	if (rh->this_host_bind_ipaddr != NULL)
		*rh->this_host_bind_ipaddr = rval;

	return rval;
}

/*
 * Function: rc_get_srcaddr
 *
 * Purpose: given remote address find local address which the
 *          system will use as a source address for sending
 *          datagrams to that remote address
 *
 * Returns: 0 in success, -1 on failure, address is filled into
 *          the first argument.
 *
 */
int
rc_get_srcaddr(struct sockaddr *lia, struct sockaddr *ria)
{
	int temp_sock;
	socklen_t namelen;

	temp_sock = socket(ria->sa_family, SOCK_DGRAM, 0);
	if (temp_sock == -1) {
		rc_log(LOG_ERR, "rc_get_srcaddr: socket: %s", strerror(errno));
		return -1;
	}

	if (connect(temp_sock, ria, SA_LEN(ria)) != 0) {
		rc_log(LOG_ERR, "rc_get_srcaddr: connect: %s",
		    strerror(errno));
		close(temp_sock);
		return -1;
	}

	namelen = SA_LEN(ria);
	if (getsockname(temp_sock, lia, &namelen) != 0) {
		rc_log(LOG_ERR, "rc_get_srcaddr: getsockname: %s",
		    strerror(errno));
		close(temp_sock);
		return -1;
	}

	close(temp_sock);
	return 0;
}
