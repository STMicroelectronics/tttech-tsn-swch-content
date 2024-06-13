CC=gcc

CFLAGS=-I. `net-snmp-config --cflags` -I/usr/include/base
BUILDLIBS= -lbase
#BUILDAGENTLIBS=`net-snmp-config --agent-libs`

# shared library flags (assumes gcc)
DLFLAGS=-fPIC -shared

ieee8021MstpMib.so: ieee8021MstpMib.c \
					ieee8021MstpCistTable/ieee8021MstpCistTable.c \
					ieee8021MstpCistPortTable/ieee8021MstpCistPortTable.c \
					ieee8021MstpPortTable/ieee8021MstpPortTable.c \
					ieee8021MstpTable/ieee8021MstpTable.c \
					ieee8021MstpConfigIdTable/ieee8021MstpConfigIdTable.c \
					Makefile
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpMib.o ieee8021MstpMib.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpCistTable/ieee8021MstpCistTable.o ieee8021MstpCistTable/ieee8021MstpCistTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpCistPortTable/ieee8021MstpCistPortTable.o ieee8021MstpCistPortTable/ieee8021MstpCistPortTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpTable/ieee8021MstpTable.o ieee8021MstpTable/ieee8021MstpTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpPortTable/ieee8021MstpPortTable.o ieee8021MstpPortTable/ieee8021MstpPortTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o ieee8021MstpConfigIdTable/ieee8021MstpConfigIdTable.o ieee8021MstpConfigIdTable/ieee8021MstpConfigIdTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -o ieee8021MstpMib.so \
					ieee8021MstpMib.o \
					ieee8021MstpCistTable/ieee8021MstpCistTable.o \
					ieee8021MstpCistPortTable/ieee8021MstpCistPortTable.o \
					ieee8021MstpTable/ieee8021MstpTable.o \
					ieee8021MstpPortTable/ieee8021MstpPortTable.o \
					ieee8021MstpConfigIdTable/ieee8021MstpConfigIdTable.o \
					$(BUILDLIBS) 

restart_snmpd:
	sudo service snmpd stop
	sudo snmpd -f -Lf snmpd.log

###############################################################################

clean:
	-rm -rf ieee8021Mstp*/*.o
	-rm -rf *.so *.o *.log
