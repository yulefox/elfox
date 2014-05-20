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
	-I$(INCDIR) \
	-I$(INCDIR)/test \
	-I$(INCDIR)/test/pb
CPPFLAGS	:= \
	-DELF_HAVE_PRAGMA_ONCE \
	-DELF_USE_ALL
LIBS		:= \
	-llog4cplus \
	-llua \
	-ltolua++ \
	-lmysqlclient \
	-lprotoc

ifeq (YES, $(DEBUG))
	LIBS	+= \
	-lelfox_d
else
	LIBS	+= \
	-lelfox
endif

LDFLAGS		:= \
	-L$(LIBDIR) \
	-L/usr/local/lib \
	-L/usr/lib64/mysql \
	$(LIBS)

SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:= \
	 -name 'todo' -prune -o

include common.mk

