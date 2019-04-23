#Networks 2
This is assignment 2 of
 * Andrew Huang (s1913999)
 * Sebastiaan Alvarez Rodriguez (s1810979)

# Convention
In this section we explain our convention in detail.

## Communication process
We have a server `S` and a client `C`.

### Initial communication
`C` initiates communication by sending ACK to `S`, together with its buffersize.
`S` registers client address details. If `S` will start sending batches of packets. If `S` does not start sending anything, the communication initiation packet must have been faulty/dropped. `C` times out, and retries to initiate communication.

### Intermediate communication
`S` sends a batch of packets with a total size exactly fitting into the buffer of `C`.
`C` receives the packets of this batch. For a batch, the following may happen on the client side:
 1. Everything okay: take data from buffer and send RR to acknowledge next batch
 2. Packet(s) X and Y received out of order: No problem, perform 1.
 3. Packet(s) X missing: `C` sends REJ X to `S` after timeout. `S` resends X.
 4. Packet(s) X checksum mismatch (faulty): Perform 3.


## Convention visualisation
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

## Flags

Flag | Bit   | Special arg | Meaning
---- | ----- | ----------- | -------------
None | `0x0` | No args     | No flag
ACK  | `0x1` | Buffersize  | Ready to receive batch 1
REJ  | `0x2` | Packetnr    | Reject specified packetnr
RR   | `0x4` | Batchnr     | Ready to receive next batch