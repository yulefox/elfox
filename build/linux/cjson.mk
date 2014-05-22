# Project parameters
###########################################################

PROJECT		:= cJSON
TARGET		:= cjson
ROOTDIR		:= ../..
OUTDIR		:= $(ROOTDIR)/lib
INCDIR		:= $(ROOTDIR)/src
SRCDIR		:= $(ROOTDIR)/src
DOCDIR		:= $(ROOTDIR)/docs
INSTLIBDIR	:= /usr/local/lib
INSTINCDIR	:= /usr/local/include
INSTALLDIR	:=
DEBUG		:= YES
LIBRARY		:= YES
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

