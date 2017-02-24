# Project parameters
###########################################################

PROJECT		:= cJSON
TARGET		:= cjson
VER_MAJOR	:= 1
VER_MINOR	:= 0
VER_BUILD	:= 1
VERSION		:= $(VER_MAJOR).$(VER_MINOR).$(VER_BUILD)
ROOTDIR		:= ../..
OUTDIR		:= $(ROOTDIR)/lib
INCDIR		:= $(ROOTDIR)/src
SRCDIR		:= $(ROOTDIR)/src
DOCDIR		:= $(ROOTDIR)/docs
INSTLIBDIR	:= $(ENV_PATH)/usr/local/lib
INSTINCDIR	:= $(ENV_PATH)/usr/local/include
INSTALLDIR	:=
LIBRARY		:= YES
SHARED		:= NO
PROFILE		:= NO
CFLAGS		:= \
	-I$(INCDIR)
CPPFLAGS	:= \
	-DELF_HAVE_PRAGMA_ONCE
LIBS		:=
LDFLAGS		:=
INCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

