#!/bin/sh
# Copyright (C) 2010 Microhardcorp

#sh /etc/m_cli/mklink.sh
clitest >/dev/null 2>&1 &
sleep 1
telnet 127.0.0.1 8000 

exit 0
