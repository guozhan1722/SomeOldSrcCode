/*
 * $Id: local.c,v 1.1 2010/07/14 22:53:05 Bob Exp $
 *
 * Copyright (C) 1996 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#include <radiusclient_config.h>
#include <radiusclient_includes.h>
#include <radiusclient-ng.h>
#include <radiusclient_messages.h>
#include <radlogin.h>

extern ENV *env;

LFUNC auth_local(char *username, char *passwd)
{
	struct passwd	*pw;
	char		*xpasswd;
#ifdef SHADOW_PASSWORD
	struct spwd	*spw;
#endif

	if ((pw = getpwnam(username)) == NULL) {
		endpwent();
		#if 0
		rc_log(LOG_NOTICE, "authentication FAILED, type local, username %s", username);
		printf(SC_LOCAL_FAILED);
		#endif
		return NULL;
	}
	endpwent();

#ifdef SHADOW_PASSWORD
        if((spw = getspnam(pw->pw_name)) == NULL) {
			endspent();
			#if 0
			rc_log(LOG_NOTICE, "authentication FAILED, type local, username %s", username);
			printf(SC_LOCAL_FAILED);
			#endif
			return NULL;
        }
        else
        {
        	pw->pw_passwd = spw->sp_pwdp;
        }
        endspent();
#endif /* SHADOW_PASSWORD */

	xpasswd = crypt(passwd, pw->pw_passwd);

	if (*pw->pw_passwd == '\0' || strcmp(xpasswd, pw->pw_passwd)) {
		#if 0
		rc_log(LOG_NOTICE, "authentication FAILED, type local, username %s", username);
		printf(SC_LOCAL_FAILED);
		#endif
		return NULL;
	}

	#if 0
	rc_log(LOG_NOTICE, "authentication OK, type local, username %s", username);
	printf(SC_LOCAL_OK);
	#endif
	return local_login;
}

void
local_login(rc_handle *rh, char *username)
{
	char *login_local = rc_conf_str(rh, "login_local");

	/* login should spot this... but who knows what old /bin/logins
	 * may be still around
	 */
	if (*username == '-') {
		rc_log(LOG_WARNING, "username can't start with a dash");
		exit(ERROR_RC);
	}
	/* the new shadow login seems to require either a -r or a -h
	 * flag for -f to work (so source code, lmain.c) so we supply
	 * it here. shouldn't hurt on other systems,	-lf, 03/13/96
	 */
	execle(login_local, login_local,  "login","-h", "localhost", "-f", username, NULL, env->env);
	rc_log(LOG_ERR, "couldn't execute %s: %s", login_local, strerror(errno));
	sleep(1);	/* give the user time to read */
	exit(ERROR_RC);
}
