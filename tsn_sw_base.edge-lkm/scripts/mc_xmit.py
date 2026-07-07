import socket

MCAST_GRP = '224.1.1.1'
MCAST_PORT = 5007

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
sock.bind(("192.168.3.2", 0))   # binding required, doesn't work without!

sock.sendto("robot", (MCAST_GRP, MCAST_PORT))

# Note: to see an effect, port_xmit-function must be instrumented and
#       interfaces must have an IP address

