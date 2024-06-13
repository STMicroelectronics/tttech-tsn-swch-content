#!/bin/bash
echo "Creating entries of lists inside ieee802-dot1cb-frer yang module"

echo "Creating sequence-identification-list entries"

tsntool frer seqenccreate 1 1 sw0p2
tsntool frer seqencaddstr 11 1 sw0p2
tsntool frer seqencaddstr 22 1 sw0p2

tsntool frer seqenccreate 1 1 sw0p3
tsntool frer seqencaddstr 33 1 sw0p3
tsntool frer seqencaddstr 44 1 sw0p3

echo "Creating sequence-generation-list entries"
tsntool frer seqgencreate 1 0 br0
tsntool frer seqgenaddstr 55 1 br0
tsntool frer seqgenaddstr 66 1 br0

tsntool frer seqgencreate 2 0 br0
tsntool frer seqgenaddstr 77 2 br0
tsntool frer seqgenaddstr 88 2 br0

echo "Creating sequence-recovery-list entries"

tsntool frer seqreccreate 5 br0
tsntool frer seqrecaddstr 99 5 br0
tsntool frer seqrecaddport sw0p2 5
tsntool frer seqrecaddport sw0p3 5

tsntool frer seqreccreate 6 br0
tsntool frer seqrecaddstr 100 6 br0
tsntool frer seqrecaddport sw0p2 6
tsntool frer seqrecaddport sw0p3 6

echo "Creating entries of lists inside ieee802-dot1cb-stream-identification yang module"

tsntool sid create 55 NULL 01-be-84-7d-22-33 1 7 br0
tsntool sid add 55 out-fac-in sw0p2 br0
tsntool sid add 55 out-fac-in sw0p3 br0

tsntool sid create 33 NULL 01-be-84-7d-22-33 1 7 br0
tsntool sid add 33 out-fac-in sw0p3 br0
tsntool sid add 33 out-fac-in sw0p4 br0