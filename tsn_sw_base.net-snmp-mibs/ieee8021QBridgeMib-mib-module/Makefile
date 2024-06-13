CC=gcc

CFLAGS=-I. `net-snmp-config --cflags` -I/usr/include/base -L/usr/local/lib/libtsn/build -I/usr/include/tsn
BUILDLIBS= -lbase -ltsn
#BUILDAGENTLIBS=`net-snmp-config --agent-libs`

# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

ieee8021QBridgeMib.so: ieee8021QBridgeMib.c \
					ieee8021QBridgeTable/ieee8021QBridgeTable.c \
					ieee8021QBridgeVlanStaticTable/ieee8021QBridgeVlanStaticTable.c \
					Makefile
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021QBridgeMib.o ieee8021QBridgeMib.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021QBridgeTable/ieee8021QBridgeTable.o ieee8021QBridgeTable/ieee8021QBridgeTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021QBridgeVlanStaticTable/ieee8021QBridgeVlanStaticTable.o ieee8021QBridgeVlanStaticTable/ieee8021QBridgeVlanStaticTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -o ieee8021QBridgeMib.so \
					ieee8021QBridgeMib.o \
					ieee8021QBridgeTable/ieee8021QBridgeTable.o \
					ieee8021QBridgeVlanStaticTable/ieee8021QBridgeVlanStaticTable.o \
					$(BUILDLIBS)

restart_snmpd:
	sudo service snmpd stop
	sudo snmpd -f -Lf snmpd.log

###############################################################################

clean:
	-rm -rf ieee8021QBridgeTable/*.o
	-rm -rf ieee8021QBridgeVlanStaticTable/*.o
	-rm -rf *.so *.o *.log

