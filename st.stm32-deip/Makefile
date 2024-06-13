obj-m := stm32_deip.o

#-----------------------------------------------------
MSRC := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(MSRC) modules

modules_install:
	make -C $(KDIR) M=$(MSRC) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install
clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
