Description:
    IEEE8021-MSTP-MIB implementation.
    *********************************************************************************************
    --->Source code generated with mib2c ieee8021Mstp[table_name]Table (sequence 2,3,1,1,1)<---
    *********************************************************************************************

IEEE8021-MSTP-MIB module modifications:
ieee8021MstpCistPortRole type modified: enum "disabled" added.
ieee8021MstpPortState type modified: enum "discarding" added.
ieee8021MstpPortRole type modified: enum "disabled" added.
ieee8021MstpPortPriority type modified: from "Integer32 (0..240)" to "Integer32 (0..15)"

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
        ( % net-snmp-config --default-mibdirs   returns a location of MIBs )
        ( before you add mib check EOL convention --> Edit EOL... )
        ( watch on revision, some latest do not work )
        
            % cp IEEE8021-MSTP-MIB.txt /usr/share/snmp/mibs
            
        2. Check if MIB is registered
            % snmptranslate -Pu -Tz -M /usr/share/snmp/mibs -m IEEE8021-MSTP-MIB
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
        
    MSTP_SERVER_AND_CLIENT
        1. PRECONDITIONS:
            $ sudo apt-get install autoconf
            $ sudo apt-get install libtool
            $ sudo apt install curl
        2. BUILD:
            $ export DEBIAN_FRONTEND=noninteractive
            $ git clone --depth=1 --branch=master https://github.com/mstpd/mstpd.git mstpd/mstpd
            $ cd mstpd/mstpd ON LAST VALID COMMIT
            $ git checkout -qf 01e2e94fd5f28e1cdcac084079fc25adfc06bde1
            $ export DEBIAN_FRONTEND=noninteractive
            $ curl -sSL "https://ftp-master.debian.org/keys/archive-key-7.0.asc" | sudo -E apt-key add
            OK
            $ echo "deb http://ftp.us.debian.org/debian unstable main contrib non-free" | sudo tee -a /etc/apt/sources.list > /dev/null
            $ export DEBIAN_FRONTEND=noninteractive
            $ sudo -E apt-get -yq update &>> ~/apt-get-update.log
            $ sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes install shellcheck
            $ export CC=gcc
            $ gcc --version
            $ set -e
            The command "set -e" exited with 0.
            $ shellcheck utils/ifupdown.sh.in utils/mstp_config_bridge.in utils/mstpctl-utils-functions.sh
            The command "shellcheck utils/ifupdown.sh.in utils/mstp_config_bridge.in utils/mstpctl-utils-functions.sh" exited with 0.
            $ ./autogen.sh && ./configure && make V=s
        3. ENABLE_IT_ON_SYSTEM
            $ sudo cp mstpctl /usr/local/bin/
            $ sudo chmod 755 /usr/local/bin/mstpctl
            $ sudo cp mstpd /usr/local/bin/
            $ sudo chmod 755 /usr/local/bin/mstpd
            restart machine
        4. START_SERVER
            run mstpd with root privilege. 
            $ sudo mstpd
            
Build and test:
    1.  Copy ieee8021MstpMib-module folder to your system
    2.  Change position on that folder 
    3.  % sudo nano /etc/snmp/snmpd.conf
    4.  add "dlmod ieee8021MstpMib [path to .so file]/ieee8021BridgeMib.so" (ieee8021BridgeMib --> without spaces or special characters)
        e.g. dlmod ieee8021MstpMib /home/devel/smb_share/net-snmp-mibs/ieee8021MstpMib-module/ieee8021MstpMib.so
    5.  % ./build.bash
    6.  % snmpwalk localhost ieee8021MstpMib
    7.  Expected result:
            IEEE8021-MSTP-MIB::ieee8021MstpCistBridgeIdentifier.1 = STRING: "8.000.EE:65:B9:B0:99:00"
            IEEE8021-MSTP-MIB::ieee8021MstpCistTopologyChange.1 = INTEGER: 0
            IEEE8021-MSTP-MIB::ieee8021MstpCistRegionalRootIdentifier.1 = STRING: "8.000.EE:65:B9:B0:99:00"
            IEEE8021-MSTP-MIB::ieee8021MstpCistPathCost.1 = Gauge32: 0
            IEEE8021-MSTP-MIB::ieee8021MstpCistMaxHops.1 = INTEGER: 20
            IEEE8021-MSTP-MIB::ieee8021MstpCistMaxHops.1 = No more variables left in this MIB View (It is past the end of the MIB tree)






