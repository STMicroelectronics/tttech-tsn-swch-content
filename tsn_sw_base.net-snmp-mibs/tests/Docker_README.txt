**************************************************************************** Running net-snmp tests using docker ****************************************************************************

1. Copy net-snmp-mibs folder to test machine

2. Install docker on test machine using next commands:
sudo apt update
sudo apt install docker.io

3. Possition yourself to directory /net-snmp-mibs/tests with next command:
cd /net-snmp-mibs/tests

4. Open De-eval-jenkins2 configuration file at location /net-snmp-mibs/tests/input/DE-eval-jenkins2 using next command:
sudo nano /net-snmp-mibs/tests/input/DE-eval-jenkins2/DE-eval-jenkins2_configuration.json

5. Change IP address inside file with IP address of your board (for example 192.168.0.1)

6. When you are inside /net-snmp-mibs/tests directory run next command to create docker image for tests:
sudo docker build -t net-snmp-tests .

7. After docker image is created, run that image inside container using next command:
sudo docker run --env target=DE-eval-jenkins2 -v <absolute-path-to>/net-snmp-mibs/MIB_repository:/MIB_repository -v <absolute-path-to>/net-snmp-mibs/tests:/code/results --rm net-snmp-tests

**********************************************************************************************************************************************************************************************


After tests are executed, inside /net-snmp-mibs/tests directory there will be an xml file called result.xml and it will contain results of net-snmp tests execution.



********************************************************************************** useful  docker commands ***********************************************************************************
List all containers using command: 
sudo docker ps -a

Stop container using command: 
sudo docker stop sysrepo_container

Remove container using command:
sudo docker rm sysrepo_container

List docker images using command: 
sudo docker images

Remove docker images using command: 
sudo docker rmi name_of_image
**********************************************************************************************************************************************************************************************


