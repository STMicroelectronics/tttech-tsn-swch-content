Description:
    IEEE8021-BRIDGE-MIB implementation.
   
    *********************************************************************************************
    --->Source code generated with mib2c ieee8021Bridge[table_name]Table (sequence 2,3,1,1,1)<---
    *********************************************************************************************
The list of implemented objects:
	ieee8021BridgeBaseTable: 
         ieee8021BridgeBaseComponentId
         ieee8021BridgeBaseBridgeAddress
         ieee8021BridgeBaseNumPorts
         ieee8021BridgeBaseComponentType
         ieee8021BridgeBaseTrafficClassesEnabled

	ieee8021BridgeBasePortTable:
      ieee8021BridgeBasePortComponentId
      ieee8021BridgeBasePort
      ieee8021BridgeBasePortIfIndex
     ieee8021BridgeBasePortType
        
Preconditions:
    $ sudo apt-get install iproute
    $ sudo apt install bridge-utils
    
    CONFIGURATION OF THE AGENT AND MASTER 
        1.  Install snmp, snmpd and snmptrapd:
            sudo apt-get update
            sudo apt-get install snmp snmp-mibs-downloader snmpd snmptrapd libsnmp-dev build-essential
        
        2.  Enable access to all MIBs
            sudo nano /etc/snmp/snmp.conf
            Change "mibs:" into "mibs +ALL"
            
        3.  Create user to access Agent Machine remotely
            sudo nano /etc/snmp/snmpd.conf
            createUser username MD5 user_password DES
            rwuser username priv
            sudo service snmpd restart
            
        4.  Create private conf file to enable easier access to remote Agent Machine - this is optional
            mkdir ~/.snmp
            cd ~/.snmp
            nano snmp.conf
            (ctrl+h if you want to see directory from GUI)
        
            defSecurityName username
            defSecurityLevel authPriv
            defAuthType MD5
            defPrivType DES
            defAuthPassphrase user_password
            defPrivPassphrase user_password 
            
        5.  Make username as owner of "mibs" directory and change access rights (This is helpful for further work)
            sudo chown -R username /usr/share/snmp/mibs
            sudo chmod -R 777 /usr/share/snmp/mibs
        
        
    ADDING A MIB (MIB registration)
        1. Copy the MIB txt file into the appropriate location 
        ( % net-snmp-config --default-mibdirs returns a location of MIBs )
        ( before you add mib check EOL convention --> Edit EOL... --> Unix)
		( Encoding --> ANSI )
        ( watch on revision, some newest do not work )
        
            % cp IEEE8021-BRIDGE-MIB.txt /usr/share/snmp/mibs
            
        2. Check if MIB is registered
            % snmptranslate -Pu -Tz -M /usr/share/snmp/mibs -m IEEE8021-BRIDGE-MIB
            % snmptranslate -Tp >>(relevant MIB OID from previous step)<<
        
     
    HOW TO USE SIMULATED DRIVER AND BRIDGE DEVICE
        !!! MAKE SURE THAT SHELL SCRIPTS ARE IN UNIX NOTATION (ONLY LF, WITHOUT CR) !!!
        1.  Checkout TSN_SW_BASE/repos/proxy-lkm/browse
        2.  Build it using "make all"
        3.  After that "sudo make insert-all"
        4.  "lsmod | grep edgx" command should deliver something like
                edgx_sim               16384  0
                edgx_sw_core           20480  5 edgx_sim2
                edgx_net_core          20480  2 edgx_sw_core,edgx_sim
                ptp                    20480  1 edgx_net_core
        5.  "ifconfig -a" should return sw0p0-4
        6.  Driver is loaded now, all network interfaces instantiated. 
              Now the last part you need is a bridge device.
        7.  If not installed already, "sudo apt install bridge-utils"
        8.  Checkout TSN_SW_BASE/repos/proxy-demos/browse
        9.  Navigate to /scripts and run "sudo ./_init.sh 1"
        10. You should get a bridge device named "br0" with "ifconfig -a"
        11. OPTIONAL (if you need) --> To shut everything down again, run "sudo ./_shutdown.sh 1" 
        
    RSTP_SERVER_AND_CLIENT
        1. GET_SOURCES
            Clone it from project git repository 
            or
            Clone it from git hub
            https://github.com/shemminger/RSTP
            
            Replace /usr/include/linux/in6.h with new one. 
            https://git.zx2c4.com/linux/plain/include/uapi/linux/in6.h
                $ sudo cp in6.h /usr/include/linux/in6.h
        2. BUILD_SOURCES
            $ cd rstp
            $ make
            $ sudo cp rstpctl /usr/local/bin/
            $ sudo chmod 755 /usr/local/bin/rstpctl
            $ sudo cp rstpd /usr/local/bin/
            $ sudo chmod 755 /usr/local/bin/rstpd
            restart machine
            
        3. START RSTPD
            before next step you need to have at least one bridge active on system
            $ sudo rstpd -d
            do not close this terminal
            
Build and test:
    1.  Copy ieee8021BridgeMib-mib-module folder to your system
    2.  Change position on that folder 
    3.  % sudo nano /etc/snmp/snmpd.conf
    4.  add "dlmod ieee8021BridgeMib [path to .so file]/ieee8021BridgeMib.so" (ieee8021BridgeMib --> without spaces or special characters)
    5.  % ./build.bash
    6.  % snmpwalk localhost ieee8021BridgeMib
    7.  Expected result:
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBaseBridgeAddress.1 = STRING: 4e:f2:d6:14:8f:0
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBaseNumPorts.1 = INTEGER: 4 ports
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBaseComponentType.1 = INTEGER: cVlanComponent(3)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBaseTrafficClassesEnabled.1 = INTEGER: true(1)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortIfIndex.1.1 = INTEGER: 4
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortIfIndex.1.2 = INTEGER: 5
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortIfIndex.1.3 = INTEGER: 6
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortIfIndex.1.4 = INTEGER: 7
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortType.1.1 = INTEGER: customerVlanPort(2)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortType.1.2 = INTEGER: customerVlanPort(2)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortType.1.3 = INTEGER: customerVlanPort(2)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortType.1.4 = INTEGER: customerVlanPort(2)
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortName.1.1 = STRING: sw0p0
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortName.1.2 = STRING: sw0p1
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortName.1.3 = STRING: sw0p2
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortName.1.4 = STRING: sw0p3
			IEEE8021-BRIDGE-MIB::ieee8021BridgeBasePortName.1.4 = No more variables left in this MIB View (It is past the end of the MIB tree)






