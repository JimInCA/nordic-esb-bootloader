#
# This Makefile is intended to build esb_test for Linux
#

APP        = esb_test

CC         = gcc
LINKER     = $(CC) -o
RM         = rm -f
MKDIR      = mkdir -p
RMDIR      = rmdir
COPY       = adb push
HOSTDIR    = /mnt/userdata

SRCDIR     = src
OBJDIR     = obj
BINDIR     = bin

CFLAGS    :=
LFLAGS    :=
OBJECTS   := $(OBJDIR)/$(APP).o
DIRS      := $(OBJDIR) $(BINDIR)

all: directories $(BINDIR)/$(APP)

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "Compiled "$@" successfully!"

$(BINDIR)/$(APP): $(OBJECTS)
	$(LINKER) $(LDLAGS) $@ $^
	@echo "Linking "$@" complete!"

.PHONEY: directories
directories: $(DIRS)

$(OBJDIR):
	@$(MKDIR) $(OBJDIR)

$(BINDIR):
	@$(MKDIR) $(BINDIR)

.PHONEY: copy
copy:
	@$(COPY) $(BINDIR)/$(APP) $(HOSTDIR)/.

.PHONEY: clean
clean:
	$(RM) $(OBJDIR)/*
	@echo "Derived objects removed!"

.PHONEY: realclean
realclean: clean
	@$(RM) $(BINDIR)/*
	@$(RMDIR) $(OBJDIR)
	@$(RMDIR) $(BINDIR)
	@echo "Binaries removed!"

