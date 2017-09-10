######################################################
# Webif base
#
# Description:
#        Holds primary functions. Header, footer, etc..
#
# Author(s) [in order of work date]:
#        A variety of people. Several X-Wrt developers.
#
# Major revisions:
#
# Configuration files referenced:
#

libdir=/usr/lib/webif
wwwdir=/www
cgidir=/www/cgi-bin/webif
rootdir=/cgi-bin/webif
cachedir=/tmp/.webcache
cachetime=7200			# cache time-out: 5 days

. /usr/lib/webif/functions.sh
. /lib/config/uci.sh
. /etc/version

uci_load "webif"
_device="$CONFIG_general_device_name"

init_cache() {
	[ -d $cachedir ] || mkdir $cachedir 2>/dev/null 1>&2
	awk -F: -v rootdir="$rootdir" -v cachedir="$cachedir" -v USER="${1:-root}" \
	    -f /usr/lib/webif/common.awk -f /usr/lib/webif/acl.awk $cgidir/.categories $cgidir/*.sh 2>/dev/null
}

if [ "$PRODUCT" = "VIP4G" -o "$PRODUCT" = "IPnVTn3G" -o "$PRODUCT" = "VIP4GEXP" -o "$PRODUCT" = "IPnVTn3GEXP" ];then
    OVERLAY_WIDTH="1010px"
    MH_LOGO="hlogo"
else
    OVERLAY_WIDTH="90%"
    MH_LOGO="logo"
fi

categories() {
	cachefile="$cachedir/cat_${REMOTE_USER:-root}"
	awk -F: -v selected="$1" -f /usr/lib/webif/categories.awk $cachefile 2>/dev/null
	#sed -e 's%^-:-$%<li>\&nbsp;</li>%' \
	#    -e 's%^\([^:]*\):\(.*\)$%<li><a href="\2?cat=\1">@TR<<\1>></a></li>%' \
	#    -e '/'"$1"'/s%^<li>\(.*\)$%<li class="selected">\1%' $cachefile 2>/dev/null
}

subcategories() {
	cachefile="$cachedir/subcat_${REMOTE_USER:-root}"
	#awk -F: -v category="$1" -v selected="$2" -f /usr/lib/webif/subcategories.awk $cachefile 2>/dev/null
	
	sed -e '/^'"$1"':/!d' \
	    -e 's%[^:]*:[^:]*:\([^:]*\):\(.*\)$%<li><a href="\2">@TR<<\1>></a></li>%' \
	    -e '/<<'"$2"'>>/s%^<li>\(.*\)$%<li class="selected">\1%' $cachefile 2>/dev/null
}

perm_denied() {
	[ "$REMOTE_USER" = "root" -o "$REMOTE_USER" = "admin" ] && return 1
	[ "$SCRIPT_NAME" = "/cgi-bin/webif/config.sh" ] && return 1
	
	cachefile="$cachedir/subcat_${REMOTE_USER:-root}"
	grep -q "$SCRIPT_NAME" $cachefile 2>/dev/null && return 1
	return 0
}

ShowWIPWarning() {
	echo "<div class=\"warning\">@TR<<big_warning#WARNING>>: @TR<<page_incomplete#This page is incomplete and may not work correctly, or at all.>></div>"
}

ShowUntestedWarning() {
	echo "<div class=\"warning\">@TR<<big_warning#WARNING>>: @TR<<page_untested#This page is untested and may or may not work correctly.>></div>"
}

ShowNotUpdatedWarning() {
       echo "<div class=\"warning\">@TR<<big_warning#WARNING>>: @TR<<page_untested_kamikaze#This page has not been updated or checked for correct functionality under Kamikaze.>></div>"
}

update_changes() {
	CHANGES=$(($( (cat /tmp/.webif/config-* ; ls /tmp/.webif/file-*) 2>&- | wc -l)))
        EDITED_FILES=$(find "/tmp/.webif/edited-files" -type f 2>&- | wc -l)
	CHANGES=$(($CHANGES + $EDITED_FILES))
	# calculate and add number of pending uci changes
	for uci_tmp_file in $(ls /tmp/.uci/* 2>&- | grep -v "\\.lock\$" 2>&-); do
		CHANGES_CUR=$(cat "$uci_tmp_file" | grep "\\w" | wc -l)
		CHANGES=$(($CHANGES + $CHANGES_CUR))
	done
}

pcnt=0
nothave=0
_savebutton_bk=""

has_pkgs() {
	retval=0;
	for pkg in "$@"; do
		pcnt=$((pcnt + 1))
		empty $(opkg list_installed | grep "^$pkg ") && {
			echo -n "<p>@TR<<features_require_package#Features on this page require the package>>: \"<b>$pkg</b>\". &nbsp;<a href=\"/cgi-bin/webif/system-ipkg.sh?action=install&amp;pkg=$pkg&amp;prev=$SCRIPT_NAME\">@TR<<features_install#install now>></a>.</p>"
			retval=1;
			nothave=$((nothave + 1))
		}
	done
	[ -z "$_savebutton_bk" ] && _savebutton_bk=$_savebutton
	if [ "$pcnt" = "$nothave" ]; then
		_savebutton=""
	else
		_savebutton=$_savebutton_bk
	fi
	return $retval;
}

mini_header() {

cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<?xml version="1.0" encoding="UTF-8"?>
<head>
	<link rel="stylesheet" type="text/css" href="/themes/active/webif.css" />        
	<title></title>
	<style type="text/css">
		html, body { background-color: transparent; }
	</style>
</head>
EOF
}

header() {
	empty "$ERROR" && {
		_saved_title="${SAVED:+ @TR<<>>}"
	} || {
		FORM_submit="";
		ERROR="<h3 class=\"warning\">$ERROR</h3>"
		_saved_title=": @TR<<Settings not saved>>"
	}

	# Initialize the categories cache if not done already
	[ -s "$cachedir/cat_${REMOTE_USER:-root}" ] || init_cache ${REMOTE_USER:-root}
	#cache_needs_update="$(find $cachedir -mmin -$cachetime -name cat_${REMOTE_USER:-root} 2>/dev/null | wc -l)"
	#[ "$cache_needs_update" = "0" ] && init_cache ${REMOTE_USER:-root}
	

	_category="$1"
	_firmware_version="$CONFIG_general_firmware_version"
	_firmware_name="$CONFIG_general_firmware_name"
        if [ "$HARDWARE" = "v1.0.0" ]; then
            _firmware_name="$PRODUCTNAME"
        elif [ "$HARDWARE" = "v2.0.0" ]; then
            _firmware_name="$PRODUCT"
        fi        
	_firmware_subtitle="$CONFIG_general_firmware_subtitle"
	_use_progressbar="$CONFIG_general_use_progressbar"
	_uptime="$(uptime)"
	_loadavg="${_uptime#*load average: }"
	_uptime="${_uptime#*up }"
	_uptime="${_uptime%%, load *}"
	_hostname=$(cat /proc/sys/kernel/hostname)
	_webif_rev=$(cat /www/.version)
	_head="${3:+<h2>$3$_saved_title</h2>}"
	_form="${5:+<form enctype=\"multipart/form-data\" action=\"$5\" method=\"post\"><input type=\"hidden\" name=\"submit\" value=\"1\" />}"
	[ "$7" = "0" ] || _savebutton="${5:+<div class=\"page-save\"><input id=\"savebutton\" type=\"submit\" name=\"action\" value=\"@TR<<Save Changes>>\" /></div>}"
	_categories=$(categories $1)
	_subcategories=${2:+$(subcategories "$1" "$2")}
	_pagename="${2:+@TR<<$2>> - }"
	_time="`date +%T`"
	_date="`date +%F`"
	if ! equal $6 "" && ! equal $6 "0" ; then _pageload="<SCRIPT type='text/javascript'>start=0; end=$6</SCRIPT><SCRIPT src='/js/pageload.js' type='text/javascript'></SCRIPT><DIV id='loadmain'><SCRIPT type='text/javascript'>document.getElementById(\"loadmain\").style.display = \"none\";</SCRIPT>"; _JSload="<SCRIPT type='text/javascript'>load()</SCRIPT>"; fi

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && grep 'root:!' /etc/passwd >&- 2>&- && {
		_nopasswd=1
		_form=""
		_savebutton=""
	}
[ -f /www/themes/active/webif_header_info ] && . /www/themes/active/webif_header_info
cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
<title>$_pagename$_firmware_name @TR<<Administrative Console>></title>
	<link rel="stylesheet" type="text/css" href="/themes/active/waitbox.css" media="screen" />
	<link rel="stylesheet" type="text/css" href="/themes/active/webif.css" />
	$webif_header_title
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<meta http-equiv="expires" content="-1" />
	<script type="text/javascript" src="/js/styleswitcher.js"></script>
$header_inject_head</head>
<body $4>$header_inject_body

<div id="container">

<div id="overlay" style=" width: $OVERLAY_WIDTH ;z-index:90; display:none; position:absolute; opacity:0.4;background-color:lightskyblue;"> 
<a href="#">
<img id="loadingImage" src="/images/ajax-loader.gif" style="position:absolute; display:none; z-index:150 "  />
</a>
</div>

<div id="lightbox" style="position:absolute; display:none; z-index:100 ">
<a href="#"></a>
</div>

<div id="header">
	<img src="/images/${MH_LOGO}.gif" />
</div>

<div id="mainmenu">
$_categories
</div>

<div id="submenu">
$_subcategories
</div>

$colorswitcher
EOF

if equal $_use_progressbar "1" ; then echo $_pageload
else echo "<script type='text/javascript'>function load() { }</script>"
fi

cat <<EOF
$_form

<div id="content">
	$_head
	$ERROR
EOF

    empty "$FORM_submit" || {
       ! empty "$FORM_action" && empty "$ERROR" && {
            update_changes
            case "$CHANGES" in
	        ""|0)
                    _saved_title="@TR<<No Changes>>"
                ;;

                *)  
                    equal "$_use_progressbar" "1" && {
                        echo '<script type="text/javascript" src="/js/waitbox.js"></script>'

cat <<EOF
<script type="text/javascript">
<!--
    showLightbox_VIP4G(document.getElementById('waitbox'));
-->
</script>
EOF

                    }

                ;;    

            esac
        }
   }

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && {
		! empty "$FORM_passwd1$FORM_passwd2" && {
			equal "$FORM_passwd1" "$FORM_passwd2" && {
				echo '<pre>'
				(
					echo "$FORM_passwd1"
					sleep 1
					echo "$FORM_passwd2"
				) | passwd root 2>&1 && apply_passwd
				echo '</pre><meta http-equiv="refresh" content="4; URL=/cgi-bin/webif/system-info.sh">'
				footer
				exit
			} || {
				echo "<h3 class=\"warning\">@TR<<Password_mismatch#The entered passwords do not match!>></h3>"
			}
		}
		equal "$_nopasswd" 1 && {
			cat <<EOF
<br />
<br />
<br />
<h3>@TR<<Warning>>: @TR<<Password_warning|You haven't set a password for the Web interface and SSH access.<br />Please enter one now (the user name in your browser will be 'root').>></h3>
<br />
<br />
EOF
			empty "$NOINPUT" && cat <<EOF
<form enctype="multipart/form-data" action="$SCRIPT_NAME" method="POST">
<table>
	<tr>
		<td>@TR<<New Password>>:</td>
		<td><input type="password" name="passwd1" /></td>
	</tr>
	<tr>
		<td>@TR<<Confirm Password>>:</td>
		<td><input type="password" name="passwd2" /></td>
	</tr>
	<tr>
		<td colspan="2"><input type="submit" name="action" value="@TR<<Set>>" /></td>
	</tr>
</table>
</form>
EOF
			footer
			exit
		} || {
			apply_passwd
		}
	}
	perm_denied "$_category" && {
		echo "Permission Denied"
		footer
		exit
	}
}

header_vpn() {
	empty "$ERROR" && {
		_saved_title="${SAVED:+ @TR<<>>}"
	} || {
		FORM_submit="";
		ERROR="<h3 class=\"warning\">$ERROR</h3>"
		_saved_title=": @TR<<Settings not saved>>"
	}

	# Initialize the categories cache if not done already
	[ -s "$cachedir/cat_${REMOTE_USER:-root}" ] || init_cache ${REMOTE_USER:-root}
	#cache_needs_update="$(find $cachedir -mmin -$cachetime -name cat_${REMOTE_USER:-root} 2>/dev/null | wc -l)"
	#[ "$cache_needs_update" = "0" ] && init_cache ${REMOTE_USER:-root}
	

	_category="$1"
	_firmware_version="$CONFIG_general_firmware_version"
	_firmware_name="$CONFIG_general_firmware_name"
        if [ "$HARDWARE" = "v1.0.0" ]; then
            _firmware_name="$PRODUCTNAME"
        elif [ "$HARDWARE" = "v2.0.0" ]; then
            _firmware_name="$PRODUCT"
        fi        
	_firmware_subtitle="$CONFIG_general_firmware_subtitle"
	_use_progressbar="$CONFIG_general_use_progressbar"
	_uptime="$(uptime)"
	_loadavg="${_uptime#*load average: }"
	_uptime="${_uptime#*up }"
	_uptime="${_uptime%%, load *}"
	_hostname=$(cat /proc/sys/kernel/hostname)
	_webif_rev=$(cat /www/.version)
	_head="${3:+<h2>$3$_saved_title</h2>}"
	_form="${5:+<form enctype=\"multipart/form-data\" action=\"$5\" method=\"post\"><input type=\"hidden\" name=\"submit\" value=\"1\" />}"
	[ "$7" = "0" ] || _savebutton="${5:+<div class=\"page-save\"><input id=\"savebutton\" type=\"submit\" name=\"action\" value=\"@TR<<Save Changes>>\" /></div>}"
	_categories=$(categories $1)
	_subcategories=${2:+$(subcategories "$1" "$2")}
	_pagename="${2:+@TR<<$2>> - }"
	_time="`date +%T`"
	_date="`date +%F`"
	if ! equal $6 "" && ! equal $6 "0" ; then _pageload="<SCRIPT type='text/javascript'>start=0; end=$6</SCRIPT><SCRIPT src='/js/pageload.js' type='text/javascript'></SCRIPT><DIV id='loadmain'><SCRIPT type='text/javascript'>document.getElementById(\"loadmain\").style.display = \"none\";</SCRIPT>"; _JSload="<SCRIPT type='text/javascript'>load()</SCRIPT>"; fi

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && grep 'root:!' /etc/passwd >&- 2>&- && {
		_nopasswd=1
		_form=""
		_savebutton=""
	}
[ -f /www/themes/active/webif_header_info ] && . /www/themes/active/webif_header_info
cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
<title>$_pagename$_firmware_name @TR<<Administrative Console>></title>
	<link rel="stylesheet" type="text/css" href="/themes/active/waitbox.css" media="screen" />
	<link rel="stylesheet" type="text/css" href="/themes/active/webif.css" />
	$webif_header_title
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<meta http-equiv="expires" content="-1" />
	<script type="text/javascript" src="/js/styleswitcher.js"></script>
$header_inject_head</head>
<body $4>$header_inject_body

<div id="container">

<div id="overlay" style=" width: $OVERLAY_WIDTH ;z-index:90; display:none; position:absolute; opacity:0.4;background-color:lightskyblue;"> 
<a href="#">
<img id="loadingImage" src="/images/ajax-loader.gif" style="position:absolute; display:none; z-index:150 "  />
</a>
</div>

<div id="lightbox" style="position:absolute; display:none; z-index:100 ">
<a href="#"></a>
</div>

<div id="header">
	<img src="/images/${MH_LOGO}.gif" />
</div>

<div id="mainmenu">
$_categories
</div>

<div id="submenu">
$_subcategories
</div>

$colorswitcher
EOF

if equal $_use_progressbar "1" ; then echo $_pageload
else echo "<script type='text/javascript'>function load() { }</script>"
fi

cat <<EOF
$_form

<div id="content">
	$_head
	$ERROR
EOF

    empty "$FORM_submit" || {
    
       ! empty "$FORM_action" && empty "$ERROR" && {
            update_changes
            case "$CHANGES" in
	        ""|0)
                    _saved_title="@TR<<No Changes>>"
                ;;

                *)  
                    equal "$_use_progressbar" "1" && {
                        echo '<script type="text/javascript" src="/js/waitbox.js"></script>'

cat <<EOF
<script type="text/javascript">
<!--
    showLightbox_VIP4G(document.getElementById('waitbox'));
-->
</script>
EOF
                    }

                ;;    

            esac
cat <<EOF
<meta http-equiv="refresh" content="0; url=/cgi-bin/webif/ipsec-s2s-summary.sh" />
EOF
        }

   }

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && {
		! empty "$FORM_passwd1$FORM_passwd2" && {
			equal "$FORM_passwd1" "$FORM_passwd2" && {
				echo '<pre>'
				(
					echo "$FORM_passwd1"
					sleep 1
					echo "$FORM_passwd2"
				) | passwd root 2>&1 && apply_passwd
				echo '</pre><meta http-equiv="refresh" content="4; URL=/cgi-bin/webif/system-info.sh">'
				footer
				exit
			} || {
				echo "<h3 class=\"warning\">@TR<<Password_mismatch#The entered passwords do not match!>></h3>"
			}
		}
		equal "$_nopasswd" 1 && {
			cat <<EOF
<br />
<br />
<br />
<h3>@TR<<Warning>>: @TR<<Password_warning|You haven't set a password for the Web interface and SSH access.<br />Please enter one now (the user name in your browser will be 'root').>></h3>
<br />
<br />
EOF
			empty "$NOINPUT" && cat <<EOF
<form enctype="multipart/form-data" action="$SCRIPT_NAME" method="POST">
<table>
	<tr>
		<td>@TR<<New Password>>:</td>
		<td><input type="password" name="passwd1" /></td>
	</tr>
	<tr>
		<td>@TR<<Confirm Password>>:</td>
		<td><input type="password" name="passwd2" /></td>
	</tr>
	<tr>
		<td colspan="2"><input type="submit" name="action" value="@TR<<Set>>" /></td>
	</tr>
</table>
</form>
EOF
			footer
			exit
		} || {
			apply_passwd
		}
	}
	perm_denied "$_category" && {
		echo "Permission Denied"
		footer
		exit
	}
}

header_nfr() {
	empty "$ERROR" && {
		_saved_title="${SAVED:+ @TR<<>>}"
	} || {
		FORM_submit="";
		ERROR="<h3 class=\"warning\">$ERROR</h3>"
		_saved_title=": @TR<<Settings not saved>>"
	}

	# Initialize the categories cache if not done already
	[ -s "$cachedir/cat_${REMOTE_USER:-root}" ] || init_cache ${REMOTE_USER:-root}
	#cache_needs_update="$(find $cachedir -mmin -$cachetime -name cat_${REMOTE_USER:-root} 2>/dev/null | wc -l)"
	#[ "$cache_needs_update" = "0" ] && init_cache ${REMOTE_USER:-root}
	

	_category="$1"
	_firmware_version="$CONFIG_general_firmware_version"
	_firmware_name="$CONFIG_general_firmware_name"
        if [ "$HARDWARE" = "v1.0.0" ]; then
            _firmware_name="$PRODUCTNAME"
        elif [ "$HARDWARE" = "v2.0.0" ]; then
            _firmware_name="$PRODUCT"
        fi        
	_firmware_subtitle="$CONFIG_general_firmware_subtitle"
	_use_progressbar="$CONFIG_general_use_progressbar"
	_uptime="$(uptime)"
	_loadavg="${_uptime#*load average: }"
	_uptime="${_uptime#*up }"
	_uptime="${_uptime%%, load *}"
	_hostname=$(cat /proc/sys/kernel/hostname)
	_webif_rev=$(cat /www/.version)
	_head="${3:+<h2>$3$_saved_title</h2>}"
	_form="${5:+<form enctype=\"multipart/form-data\" action=\"$5\" method=\"post\"><input type=\"hidden\" name=\"submit\" value=\"1\" />}"
	[ "$7" = "0" ] || _savebutton="${5:+<div class=\"page-save\"><input id=\"savebutton\" type=\"submit\" name=\"action\" value=\"@TR<<Save Changes>>\" /></div>}"
	_categories=$(categories $1)
	_subcategories=${2:+$(subcategories "$1" "$2")}
	_pagename="${2:+@TR<<$2>> - }"
	_time="`date +%T`"
	_date="`date +%F`"
	if ! equal $6 "" && ! equal $6 "0" ; then _pageload="<SCRIPT type='text/javascript'>start=0; end=$6</SCRIPT><SCRIPT src='/js/pageload.js' type='text/javascript'></SCRIPT><DIV id='loadmain'><SCRIPT type='text/javascript'>document.getElementById(\"loadmain\").style.display = \"none\";</SCRIPT>"; _JSload="<SCRIPT type='text/javascript'>load()</SCRIPT>"; fi

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && grep 'root:!' /etc/passwd >&- 2>&- && {
		_nopasswd=1
		_form=""
		_savebutton=""
	}
[ -f /www/themes/active/webif_header_info ] && . /www/themes/active/webif_header_info
cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
<title>$_pagename$_firmware_name @TR<<Administrative Console>></title>
	<link rel="stylesheet" type="text/css" href="/themes/active/waitbox.css" media="screen" />
	<link rel="stylesheet" type="text/css" href="/themes/active/webif.css" />
	$webif_header_title
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<meta http-equiv="expires" content="-1" />
	<script type="text/javascript" src="/js/styleswitcher.js"></script>
$header_inject_head</head>
<body $4>$header_inject_body

<div id="container">

<div id="overlay" style=" width: $OVERLAY_WIDTH ;z-index:90; display:none; position:absolute; opacity:0.4;background-color:lightskyblue;"> 
<a href="#">
<img id="loadingImage" src="/images/ajax-loader.gif" style="position:absolute; display:none; z-index:150 "  />
</a>
</div>

<div id="lightbox" style="position:absolute; display:none; z-index:100 ">
<a href="#"></a>
</div>

<div id="header">
	<img src="/images/${MH_LOGO}.gif" />
</div>

<div id="mainmenu">
$_categories
</div>

<div id="submenu">
$_subcategories
</div>

$colorswitcher
EOF

if equal $_use_progressbar "1" ; then echo $_pageload
else echo "<script type='text/javascript'>function load() { }</script>"
fi

cat <<EOF
$_form

<div id="content">
	$_head
	$ERROR
EOF

    empty "$FORM_submit" || {
    
       ! empty "$FORM_action" && empty "$ERROR" && {
            update_changes
            case "$CHANGES" in
	        ""|0)
                    _saved_title="@TR<<No Changes>>"
                ;;

                *)  
                    equal "$_use_progressbar" "1" && {
                        echo '<script type="text/javascript" src="/js/waitbox.js"></script>'

cat <<EOF
<script type="text/javascript">
<!--
    showLightbox_VIP4G(document.getElementById('waitbox'));
-->
</script>
EOF
                    }

                ;;    

            esac
cat <<EOF
<meta http-equiv="refresh" content="0; url=/cgi-bin/webif/$8" />
EOF
        }

   }

	empty "$REMOTE_USER" && neq "${SCRIPT_NAME#/cgi-bin/}" "webif.sh" && {
		! empty "$FORM_passwd1$FORM_passwd2" && {
			equal "$FORM_passwd1" "$FORM_passwd2" && {
				echo '<pre>'
				(
					echo "$FORM_passwd1"
					sleep 1
					echo "$FORM_passwd2"
				) | passwd root 2>&1 && apply_passwd
				echo '</pre><meta http-equiv="refresh" content="4; URL=/cgi-bin/webif/system-info.sh">'
				footer
				exit
			} || {
				echo "<h3 class=\"warning\">@TR<<Password_mismatch#The entered passwords do not match!>></h3>"
			}
		}
		equal "$_nopasswd" 1 && {
			cat <<EOF
<br />
<br />
<br />
<h3>@TR<<Warning>>: @TR<<Password_warning|You haven't set a password for the Web interface and SSH access.<br />Please enter one now (the user name in your browser will be 'root').>></h3>
<br />
<br />
EOF
			empty "$NOINPUT" && cat <<EOF
<form enctype="multipart/form-data" action="$SCRIPT_NAME" method="POST">
<table>
	<tr>
		<td>@TR<<New Password>>:</td>
		<td><input type="password" name="passwd1" /></td>
	</tr>
	<tr>
		<td>@TR<<Confirm Password>>:</td>
		<td><input type="password" name="passwd2" /></td>
	</tr>
	<tr>
		<td colspan="2"><input type="submit" name="action" value="@TR<<Set>>" /></td>
	</tr>
</table>
</form>
EOF
			footer
			exit
		} || {
			apply_passwd
		}
	}
	perm_denied "$_category" && {
		echo "Permission Denied"
		footer
		exit
	}
}

#######################################################
# footer_nosubmit
#
footer_nosubmit() {
[ "$PRODUCT" = "VIP421ENCOM" ] && {
cat <<EOF
</div>
<br />
<fieldset id="save">
     <ul class="apply">
             <li><a href="#">$PRODUCTNAME</li>
     </ul>
</fieldset>
$_endform
<hr />
</div> <!-- End #container -->
</body>
</html>
EOF

    } || {
    if [ "$PRODUCT" = "IPnVT" ];then
        PRODUCTNAME_DIS="nVIP2400"
    else
        PRODUCTNAME_DIS="$PRODUCTNAME"
    fi

cat <<EOF
</div>
<br />
<fieldset id="save">
     <ul class="apply" style="font-size: 0.8em;">
             <li><a href="#">Copyright © $VENDOR</li>
             <li><a href="#">$PRODUCTNAME_DIS</li>
     </ul>
</fieldset>
$_endform
<hr />
</div> <!-- End #container -->
</body>
</html>
EOF
    }
}

#######################################################
# footer
#
footer() {
	update_changes
	_changes=${CHANGES#0}
	_changes=${_changes:+(${_changes})}
	_endform=${_savebutton:+</form>}

cat <<EOF
</div>
<br />
<fieldset id="save">
	<legend><strong>@TR<<Proceed Changes>></strong></legend>
	$_savebutton
EOF
cat <<EOF
<script type="text/javascript">
<!--
function ischanged()
{
   document.getElementById('savebutton').click();

   return false;
}
-->
</script>
EOF

cat <<EOF
     <ul class="apply" style="font-weight: bold;">
             <li><a id="waitbox" href="#" rel="lightbox" onclick="return ischanged();">@TR<< Submit >> &laquo;</a></li>
             <li><a href="config.sh?mode=clear&amp;cat=$_category&amp;prev=$SCRIPT_NAME">@TR<< Cancel >> &laquo;</a></li>
     </ul>
</fieldset>
EOF

empty "$FORM_submit" || {
       ! empty "$FORM_action" && empty "$ERROR" && {
#cat <<EOF
#<meta http-equiv="refresh" content="0;URL=config.sh?mode=save&amp;cat=$_category&amp;prev=$SCRIPT_NAME" />
#EOF

            case "$CHANGES" in
	        ""|0)
                    _saved_title="@TR<<No Changes>>"
                ;;

                *)  
                    CHANGES=""                                                              
                    sh /usr/lib/webif/apply.sh >/dev/null 2>&1
                    equal "$_use_progressbar" "1" && {
cat <<EOF
<script type="text/javascript">
<!--
    hideLightbox(document.getElementById('waitbox'));
-->
</script>
EOF
                    } 

                ;;    

            esac

            SCRIPT_NAME=${FORM_prev:-$SCRIPT_NAME}  
            unset FORM_submit
            if [ -f "/tmp/run/httpd_restart" ];then
                rm /tmp/run/httpd_restart
	        /etc/init.d/httpd stop
                /etc/init.d/httpd start
               #when httpd change port, https have to restart to get correct http port
                sleep 1
               /etc/init.d/https stop
               /etc/init.d/https start
 
            fi
        }
   }

cat <<EOF
$_endform
<hr />
<div id="footer">
     <h3></h3>
     <em>@TR<<>></em>
</div>
</div> <!-- End #container -->
</body>
</html>
EOF
}

footer_vpn() {
	update_changes
	_changes=${CHANGES#0}
	_changes=${_changes:+(${_changes})}
	_endform=${_savebutton:+</form>}

cat <<EOF
</div>
<br />
<fieldset id="save">
	<legend><strong>@TR<<Proceed Changes>></strong></legend>
	$_savebutton
EOF
cat <<EOF
<script type="text/javascript">
<!--
function ischanged()
{
   document.getElementById('savebutton').click();

   return false;
}
-->
</script>
EOF

cat <<EOF
     <ul class="apply" style="font-weight: bold;">
             <li><a id="waitbox" href="#" rel="lightbox" onclick="return ischanged();">@TR<< Submit >> &laquo;</a></li>
             <li><a href="config.sh?mode=clear&amp;cat=$_category&amp;prev=$1">@TR<< Cancel >> &laquo;</a></li>
     </ul>
</fieldset>
EOF

empty "$FORM_submit" || {
       [ -f "/etc/debugipsec" ] && logger -t "webif.sh" "footer_vpn 2=$2"
       ! empty "$FORM_action" && empty "$ERROR" && {
#cat <<EOF
#<meta http-equiv="refresh" content="0;URL=config.sh?mode=save&amp;cat=$_category&amp;prev=$1" />
#EOF

            case "$CHANGES" in
	        ""|0)
                    _saved_title="@TR<<No Changes>>"
                ;;

                *)  
                    CHANGES=""                                                              
                    /usr/lib/webif/apply.sh $2 $3 >/dev/null 2>&1
                    equal "$_use_progressbar" "1" && {
cat <<EOF
<script type="text/javascript">
<!--
    hideLightbox(document.getElementById('waitbox'));
-->
</script>

EOF
                    } 

                ;;    

            esac
            SCRIPT_NAME=${FORM_prev:-$SCRIPT_NAME}  
            unset FORM_submit
        }
   }

cat <<EOF
$_endform
<hr />
<div id="footer">
     <h3></h3>
     <em>@TR<<>></em>
</div>
</div> <!-- End #container -->
</body>
</html>
EOF
}
#######################################################
apply_passwd() {
	case ${SERVER_SOFTWARE%% *} in
		mini_httpd/*)
			grep '^root:' /etc/passwd | cut -d: -f1,2 > $cgidir/.htpasswd
			killall -HUP mini_httpd
			;;
	esac
}

display_form() {
	if empty "$1"; then
		awk -F'|' -f /usr/lib/webif/common.awk -f /usr/lib/webif/form.awk
	else
		echo "$1" | awk -F'|' -f /usr/lib/webif/common.awk -f /usr/lib/webif/form.awk
	fi
}

