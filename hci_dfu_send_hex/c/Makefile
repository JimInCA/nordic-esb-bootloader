#
# You can set this Makefile to build hci_dfu_send_hex for Linux
# or for LMVP. The desired build is set by selecting either the
# arm-linux-gcc or gcc compiler.
#

APP        = hci_dfu_send_hex

CC         = ../../prebuilts/toolschain/usr/bin/arm-linux-gcc
#CC         = gcc
LINKER     = $(CC) -o
RM         = rm -f
MKDIR      = mkdir -p
RMDIR      = rmdir
COPY       = adb push
HOSTDIR    = /mnt/userdata

SRCDIR     = src
INCDIR     = include
OBJDIR     = obj
BINDIR     = bin

CFLAGS    := -I./$(INCDIR)
LFLAGS    :=
OBJECTS   := $(OBJDIR)/hci_dfu_send_hex.o $(OBJDIR)/crc16pure.o
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

