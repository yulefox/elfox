# Project parameters
###########################################################

PROJECT		:= elf
TARGET		:= elfox
VER_MAJOR	:= 1
VER_MINOR	:= 1
VER_BUILD	:= 18
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
	-I/opt/local/include \
	-I/opt/local/include/mysql55 \
	-I/usr/include/libxml2 \
	-I$(INCDIR) \
	-I$(ENV_PATH)/usr/local/include/lua \
	-I$(ENV_PATH)/usr/local/include \
	-fPIC 
CPPFLAGS	:= \
	-DELF_HAVE_PRAGMA_ONCE
LIBS		:=
LDFLAGS		:=
INCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

