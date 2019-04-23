#Networks 2
This is assignment 2 of
 * Andrew Huang (s1913999)
 * Sebastiaan Alvarez Rodriguez (s1810979)
___

#Convention
Here is our convention:
```
 0      7 8     15 16    23 24    31
+--------+--------+--------+--------+
|    checksum1    |       size      |
+--------+--------+--------+--------+
| flags  |   nr   |    checksum2    |
+-----------------+-----------------+
|   data octets ...
+---------------- ...
```
Please see the explanation of every field below:
 * Checksum1: This is a 16-bit checksum for fields size, flags, nr, and checksum2
 * Size:      The size of the data buffer
 * Flags:     This is a 8-bit field, containing bitflags. See further below for possible flags and their meaning
 * Nr:        The packet number, to make sure out-of-order packets can be detected
 * Checksum2: This is a 16-bit checksum for the data-field
 * Data:      Here, all data is stored

##Flags
Flag | Meaning
------------ | -------------
None | No flag
Content in the first column | Content in the second column