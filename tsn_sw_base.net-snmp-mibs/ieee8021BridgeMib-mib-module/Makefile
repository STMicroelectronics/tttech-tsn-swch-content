CC=gcc

CFLAGS=-I. `net-snmp-config --cflags` -I/usr/include/base -L/usr/local/lib/libtsn/build -I/usr/include/tsn
BUILDLIBS= -lbase -ltsn
#BUILDAGENTLIBS=`net-snmp-config --agent-libs`

# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

ieee8021BridgeMib.so: ieee8021BridgeMib.c \
					ieee8021BridgeBaseTable/ieee8021BridgeBaseTable.c \
					ieee8021BridgeBasePortTable/ieee8021BridgeBasePortTable.c \
					ieee8021BridgePhyPortTable/ieee8021BridgePhyPortTable.c \
					ieee8021BridgeTrafficClassTable/ieee8021BridgeTrafficClassTable.c \
					ieee8021BridgePortPriorityTable/ieee8021BridgePortPriorityTable.c \
					ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.c \
					Makefile
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgeMib.o ieee8021BridgeMib.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgeBaseTable/ieee8021BridgeBaseTable.o ieee8021BridgeBaseTable/ieee8021BridgeBaseTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgeBasePortTable/ieee8021BridgeBasePortTable.o ieee8021BridgeBasePortTable/ieee8021BridgeBasePortTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgePhyPortTable/ieee8021BridgePhyPortTable.o ieee8021BridgePhyPortTable/ieee8021BridgePhyPortTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgeTrafficClassTable/ieee8021BridgeTrafficClassTable.o ieee8021BridgeTrafficClassTable/ieee8021BridgeTrafficClassTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgePortPriorityTable/ieee8021BridgePortPriorityTable.o ieee8021BridgePortPriorityTable/ieee8021BridgePortPriorityTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.o ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.c
	$(CC) $(CFLAGS) $(DLFLAGS) -o ieee8021BridgeMib.so \
					ieee8021BridgeMib.o \
					ieee8021BridgeBaseTable/ieee8021BridgeBaseTable.o \
					ieee8021BridgeBasePortTable/ieee8021BridgeBasePortTable.o \
					ieee8021BridgePhyPortTable/ieee8021BridgePhyPortTable.o \
					ieee8021BridgeTrafficClassTable/ieee8021BridgeTrafficClassTable.o \
					ieee8021BridgePortPriorityTable/ieee8021BridgePortPriorityTable.o \
					ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.o \
					$(BUILDLIBS)

restart_snmpd:
	sudo service snmpd stop
	sudo snmpd -f -Lf snmpd.log

###############################################################################

clean:
	-rm -rf ieee8021Bridge*/*.o
	-rm -rf *.so *.o *.log

