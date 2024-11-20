CONFIG_MODULE_SIG=y
CONFIG_MODULE_SIG_ALL=y
GIT_COMMIT = $(shell git log -1 --pretty=format:"%h")

ifeq ($(KDIR),)
KDIR := $(KERNEL_SRC)
endif
ifeq ($(KDIR),)
KERNELRELEASE ?= $(shell uname -r)
KDIR := /lib/modules/$(KERNELRELEASE)/build
endif

target ?= platform
sched  ?= fsc
sid    ?= sid

edge_lkm_mod-target-pcie_c10 = edgx_pcie_c10_lkm
edge_lkm_mod-target-pcie     = edgx_pcie_lkm
edge_lkm_mod-target-platform = edgx_pfm_lkm

edge_lkm_mod = $(edge_lkm_mod-target-$(target))

# PSFP requires FSCr
sched-obj-fsc  = edge_st_fsc.o
sched-obj-fscr = edge_psfp.o edge_st_fscr.o edge_sched.o

sched-flags-fsc = -DEDGX_DISABLE_PSFP

obj-m += $(edge_lkm_mod).o

ccflags-y := -I$(PWD) $(sched-flags-$(sched))

$(edge_lkm_mod)-objs := \
		   edge_ac.o                 \
		   edge_br_fdb.o             \
		   edge_br_vlan.o            \
		   edge_bridge.o             \
		   edge_com.o                \
		   edge_com_dma.o            \
		   edge_com_xmii.o           \
		   edge_com_ts.o             \
		   edge_link.o               \
		   edge_preempt.o            \
		   edge_stat.o               \
		   edge_fqtss.o              \
		   edge_br_$(sid).o          \
		   edge_frer.o               \
		   altera_pio.o              \
		   edge_mdio.o               \
		   edge_port.o               \
		   edge_time.o               \
		   $(sched-obj-$(sched))     \
		   edge_$(target).o          \

all : modules

modules modules_install clean :
	@echo Building $@ via $(KDIR)
	KCPPFLAGS="-D_EDGX_GIT="$(GIT_COMMIT) $(MAKE) -C $(KDIR) M=$(PWD) $@

coccicheck:
	@echo Coccichecking via $(KDIR)
	$(MAKE) -C $(KDIR) M=$(PWD) MODE=report coccicheck

rebuild : clean all
