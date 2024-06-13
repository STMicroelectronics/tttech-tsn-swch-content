src = ./src
tst = ./test
inc = ./include
bld = ./build

VER_MAJ   := 1
VER_MIN   := 0
VER_PATCH := 0

VER      := $(VER_MAJ).$(VER_MIN)
VER_FULL := $(VER).$(VER_PATCH)

VPATH := $(src) $(tst)

LIB_OBJS = \
		base_interfaces_linux_tools.o \
		base_linux_tool_helpers.o \
		base_linux_tools.o \
		base_mstp_linux_tools.o \
		base_prints.o \
		base_private_translators.o \
		base_private_combinators.o \
		base_private_interfaces_configuration.o \
		base_private_mstp_configuration.o \
		base_private_parsers.o \
		base_private_q_bridge_configuration.o \
		base_private_syscalls.o \
		base_private_validators.o \
		base_q_bridge_linux_tools.o \

LIB_TGTS = $(addprefix $(bld)/, $(LIB_OBJS))

NAME   = base
SONAME = lib$(NAME).so.$(VER_MAJ)
LIBSO  = $(bld)/lib$(NAME).so
LIBSOV = $(LIBSO).$(VER)
LIBA   = $(bld)/lib$(NAME).a
CCFLAGS := -fPIC -O3 -W -Wall -D_GNU_SOURCE -Werror -pedantic -std=c99 -I$(inc) -DVER_FULL=\"$(VER_FULL)\"
ARFLAGS := rs

DEBUG =
VM_COMPILE =

all: libs

libs : $(LIBA) $(LIBSO)

# Create shared library
$(LIBSO) : $(LIBSOV)
	@echo Sym-linking $(notdir $@) ...
	@ln -s -f -r $(LIBSOV) $(bld)/$(SONAME)
	@ln -s -f -r $(LIBSOV) $(LIBSO)

$(LIBSOV) : dir $(LIB_TGTS)
	@echo Archiving $(notdir $@) ...
	@$(CC) $(CCFLAGS) $(DEBUG) -shared -Wl,-soname,$(SONAME) -o $(LIBSOV) $(LIB_TGTS)

# Create static library
$(LIBA) : dir $(LIB_TGTS)
	@echo Archiving $(notdir $@) ...
	@$(AR) $(ARFLAGS) $@ $(LIB_TGTS)

# test app
TST_APP= $(bld)/libbase_test_app.o

# Link dynamic library test app
dyn_app: $(TST_APP)
	$(CC) $(CCFLAGS) $(DEBUG) $(VM_COMPILE) $(TST_APP) -lbase -o $(bld)/libbase_test_app_dynamic

clean:
	@-rm -rf $(bld)

dir:
	@mkdir -p $(bld)

$(bld)/%.o : %.c
	@echo Compiling $(notdir $<) ...
	@$(CC) $(CCFLAGS) $(DEBUG) $(VM_COMPILE) -c -o $@ $<

#Compile .c to build dir but not link them
$(TST_APP):
	$(CC) -c $(CCFLAGS) $(DEBUG) $(VM_COMPILE) $(tst)/libbase_test_app.c -o $(TST_APP)
