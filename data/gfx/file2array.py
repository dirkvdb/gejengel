#!/usr/bin/env python
import array;
import os;
import sys;

f = open(sys.argv[1], 'rb')

data = array.array('B')
data.read(f, os.stat(sys.argv[1]).st_size)

count = 0
array = "{ "

for byte in data:
    array += "0x%02x, " % (int(byte))
    count += 1
    if count == 20:
        count = 0
        array += "\n"

array += "}"

print array
print len(data)
