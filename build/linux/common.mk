###########################################################
#                        ATTENTION                        #
#              do not modify the code below               #
#                        author: Fox(yulefox@gmail.com)   #
###########################################################

CC			:= gcc
CXX			:= g++
LD			:= g++
AR			:= ar rc
RANLIB		:= ranlib
DEPDIR		:= .dep/$(PROJECT)
ifeq (YES, $(DEBUG))
	TARGET		:= $(TARGET)_d
	CPPFLAGS	+= -Wall -Wno-format -g -D_DEBUG
	OBJDIR		:= .obj/$(PROJECT)/debug
else
	CPPFLAGS	+= -Wall -Wno-unknown-pragmas -Wno-format -O3
	OBJDIR		:= .obj/$(PROJECT)/release
endif

ifeq (YES, $(LIBRARY))
	CPPFLAGS	+= -fPIC
	LDFLAGS		+= -shared -Wl,-soname,lib$(TARGET).so.$(MAJOR)
	TARGET		:= $(OUTDIR)/lib$(TARGET).so.$(VERSION)
else
	TARGET		:= $(OUTDIR)/$(TARGET)
endif

ifeq (YES, $(PROFILE))
	CPPFLAGS	+= -pg -O3
	LDFLAGS		+= -pg -O3 $(LIBS)
endif

#
# # The next bit checks to see whether `rm` command is in your OS. If not
# # it uses `del` instead, but this can cause (harmless) `File not found' error
# # messages. If you are not using DOS at all, set the variable to something
# # which will unquestioningly remove files.
#

RM := rm -rf

INCS		:= $(subst $(INCDIR)/,,$(shell find $(INCDIR)/$(PROJECT) \
	-name '*.h' -print))

SRCS_C		:= $(shell find $(SRCDIRS) \
	$(SRCS_C_EXCLUDE_FILTER) \
	-name '*.c' -print)

SRCS_CPP	:= $(shell find $(SRCDIRS) \
	$(SRCS_CPP_EXCLUDE_FILTER) \
	-name '*.cpp' -print)

OBJS_C		:= $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJS_CPP	:= $(SRCS_CPP:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS_C		:= $(SRCS_C:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
DEPS_CPP	:= $(SRCS_CPP:$(SRCDIR)/%.cpp=$(DEPDIR)/%.d)
TAR_OBJ		:= $(PROJECT).tar.gz

.PHONY: all clean doc install rebuild tags test

all: $(TARGET)

doc:
	doxygen $(DOCDIR)/doxygen.config

tags:
	@ctags -R $(SRCDIR); \
   	mv tags $(ROOTDIR)

ifeq (YES, $(LIBRARY))
install: all
	@rm -rf $(INSTINCDIR)/$(PROJECT)
	@mkdir -p $(INSTLIBDIR) $(INSTINCDIR)/$(PROJECT)
	@cp $(TARGET) $(INSTLIBDIR) -f
	@cd $(SRCDIR)
	@cd $(SRCDIR); \
		tar -czf $(TAR_OBJ) $(INCS); \
		tar -xzf $(TAR_OBJ) -C $(INSTINCDIR); \
		$(RM) $(TAR_OBJ)
	@ldconfig
	@echo
	@echo Install '$(TARGET)' ... OK
	@echo
endif

rebuild: clean all tags

$(TARGET): $(OBJS_C) $(OBJS_CPP)
	$(LD) -o $@ $(OBJS_C) $(OBJS_CPP) $(CPPFLAGS) $(LDFLAGS)
	@echo
	@echo Compile/Link '$(TARGET)' ... OK
	@echo

$(OBJS_C): $(OBJDIR)%.o: $(SRCDIR)%.c
	@echo Compiling: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJS_CPP): $(OBJDIR)%.o: $(SRCDIR)%.cpp
	@echo Compiling: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DEPS_C): $(DEPDIR)%.d: $(SRCDIR)%.c
	@echo Dependency: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	@set -e; $(RM) $@; \
		$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,$(notdir $*)\.o[ :]*,$(OBJDIR)$*.o $@: ,g' < $@.$$$$ > $@; \
		$(RM) $@.$$$$

$(DEPS_CPP): $(DEPDIR)%.d: $(SRCDIR)%.cpp
	@echo Dependency: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	@set -e; $(RM) $@; \
		$(CXX) -MM $(CFLAGS) $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,$(notdir $*)\.o[ :]*,$(OBJDIR)$*.o $@: ,g' < $@.$$$$ > $@; \
		$(RM) $@.$$$$

ifneq (clean, $(MAKECMDGOALS))
-include $(DEPS_C) $(DEPS_CPP)
endif

clean:
	@$(RM) $(OBJDIR)
	@$(RM) $(DEPDIR)
	@$(RM) $(OUTDIR)/$(TARGET)
	@echo
	@echo Clean '$(TARGET)' ... OK
	@echo
