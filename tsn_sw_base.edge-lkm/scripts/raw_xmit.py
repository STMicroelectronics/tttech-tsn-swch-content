"""Demonstrates how to construct and send raw Ethernet packets on the
network.

You probably need root privs to be able to bind to the network interface,
e.g.:

    $ sudo python sendeth.py
"""

from socket import *
import sys

def sendeth(src, dst, eth_type, payload, interface = "sw0p0"):
  """Send raw Ethernet packet on interface."""

  assert(len(src) == len(dst) == 6) # 48-bit ethernet addresses
  assert(len(eth_type) == 2) # 16-bit ethernet type

  s = socket(AF_PACKET, SOCK_RAW)

  # From the docs: "For raw packet
  # sockets the address is a tuple (ifname, proto [,pkttype [,hatype]])"
  s.bind((interface, 0))
  return s.send(src + dst + eth_type + payload)

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print("Need interface to send to ...")
  else:
    print("Sent %d-byte Ethernet packet on " %
          sendeth("\xFE\xED\xFA\xCE\xBE\xEF",
              "\xFE\xED\xFA\xCE\xBE\xEF",
              "\x7A\x05",
              "123456789A123456789B123456789C123456789D123456", sys.argv[1]))
