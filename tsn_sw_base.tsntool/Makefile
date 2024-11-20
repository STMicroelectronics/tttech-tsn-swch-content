libsrc  = ./lib
toolsrc = ./tsntool
bld     = ./build

bindir  = /usr/bin
libdir  = /usr/lib
mandir  = /usr/share/man
incdir  = /usr/include

VER_FULL  = $(or $(TSNTOOL_VERSION), 0.0.0)
VER_SPLIT = $(subst ., , $(VER_FULL))
VER_MAJ   = $(word 1, $(VER_SPLIT))
VER_MIN   = $(word 2, $(VER_SPLIT))
VER       = $(VER_MAJ).$(VER_MIN)

VPATH := $(libsrc) $(toolsrc)

BIN_OBJS = brport.o edge.o fqtss.o preempt.o qbridge.o st.o mstp.o tsntool.o util.o sid.o frer.o psfp.o
LIB_OBJS = tsn.o tsn_bridge.o tsn_edgex.o tsn_fqtss.o tsn_mstp.o tsn_preemp.o tsn_qbridge.o tsn_st.o tsn_sid.o tsn_frer.o tsn_psfp.o tsn_error.o
BIN_TGTS = $(addprefix $(bld)/, $(BIN_OBJS))
LIB_TGTS = $(addprefix $(bld)/, $(LIB_OBJS))

NAME   = tsn
BIN    = $(bld)/$(NAME)tool
LIBSO  = $(bld)/lib$(NAME).so
LIBSOM = $(LIBSO).$(VER_MAJ)
LIBSOV = $(LIBSO).$(VER)
LIBA   = $(bld)/lib$(NAME).a

CC = /usr/bin/gcc

CCFLAGS := -fPIC -O3 -W -Wall -Werror -std=c99 -pedantic -I$(libsrc) -DVER_FULL=\"$(VER_FULL)\"
ARFLAGS := rs
LDLIBS  := -ltsn -lbsd -L$(bld)

all  : bin

libs : $(LIBA) $(LIBSO) $(LIBSOM)
bin  : $(BIN)

# Create tsntool executable
$(BIN): libs $(BIN_TGTS)
	@echo Building $(@F) ...
	@$(CC) $(BIN_TGTS) $(LDFLAGS) $(LDLIBS) -o $@

# Create shared library
$(LIBSO) $(LIBSOM) : $(LIBSOV)
	@echo Sym-linking $(@F) ...
	@ln -s -f $(<F) $@

$(LIBSOV) : dir $(LIB_TGTS)
	@echo Archiving $(@F) ...
	@$(CC) $(LDFLAGS) -shared -Wl,-soname,$(notdir $(LIBSOM)) -o $@ $(LIB_TGTS)

# Create static library
$(LIBA) : dir $(LIB_TGTS)
	@echo Archiving $(@F) ...
	@$(AR) $(ARFLAGS) $@ $(LIB_TGTS)

clean:
	@rm -rf $(bld)

dir:
	@mkdir -p $(bld)

$(bld)/%.o : %.c
	@echo Compiling $(<F) ...
	@$(CC) $(CFLAGS) $(CCFLAGS) -c -o $@ $<

install:
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(libdir) $(DESTDIR)$(mandir)/man8 $(DESTDIR)$(incdir)/libtsn
	install -m 0755 $(BIN) $(DESTDIR)$(bindir)
	install -m 0755 $(LIBSOV) $(DESTDIR)$(libdir)
	cp -P $(LIBSOM) $(LIBSO) $(DESTDIR)$(libdir)
	install -m 0644 $(LIBA) $(DESTDIR)$(libdir)
	install -m 0644 ./man/tsntool.8 $(DESTDIR)$(mandir)/man8
	install -m 0644 ./lib/*.h $(DESTDIR)$(incdir)/libtsn

.PHONY: all bin libs dir clean install
