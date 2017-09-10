ROOT := ../../../../
SW_ROOT ?= $(ROOT)
export SW_ROOT
MDK_ROOT := $(SW_ROOT)/src/dk/mdk
CLIENT_ROOT := $(MDK_ROOT)/client
L_ROOT := $(TOPDIR)

OS := soc_linux

OUTPUT_DIR := $(CLIENT_ROOT)/$(OS)/obj

ifeq ($(OS), soc_linux)
DEF_OS  := SOC_LINUX -DLinux -DLINUX
endif

CC := $(TOOL_PREFIX)gcc
LD := $(TOOL_PREFIX)gcc  
STRIP := $(TOOL_PREFIX)strip

CFLAGS += -I../common/include -Iinclude -I$(CLIENT_ROOT)/$(OS)/include 
CFLAGS += -I../../../include
CFLAGS += -Os

ifeq ($(AR6000_COMM),y)
CFLAGS += -DAR6000_COMM
endif

LDFLAGS = -lm -lpthread

.EXPORT_ALL_VARIABLES:

