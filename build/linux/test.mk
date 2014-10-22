# Project parameters
###########################################################

PROJECT		:= test
TARGET		:= test
ROOTDIR		:= ../..
OUTDIR		:= $(ROOTDIR)/bin/$(PROJECT)
INCDIR		:= $(ROOTDIR)/src
SRCDIR		:= $(ROOTDIR)/src
LIBDIR		:= $(ROOTDIR)/lib
DOCDIR		:= $(ROOTDIR)/docs
INSTLIBDIR	:=
INSTINCDIR	:=
INSTALLDIR	:=
DEBUG		:= YES
LIBRARY		:= NO
PROFILE		:= NO
CFLAGS		:= \
	-I/usr/include/lua \
	-I/usr/local/include/pb \
	-I$(INCDIR) \
	-I$(INCDIR)/test
CPPFLAGS	:= \
	-DELF_HAVE_PRAGMA_ONCE \
	-DELF_USE_ALL
LIBS		:= \
	-lcurl \
	-llog4cplus \
	-llua \
	-lmysqlclient \
	-lprotobuf \
	-lprotoc \
	-ltolua++ \
	-lxml2

ifeq (YES, $(DEBUG))
	LIBS	+= \
	-lcjson_d \
	-lelfox_d
else
	LIBS	+= \
	-lcjson \
	-lelfox \
	-lpb
endif

LDFLAGS		:= \
	-L$(LIBDIR) \
	-L/usr/lib64/mysql \
	$(LIBS)

SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

