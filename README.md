# Networks 2
This is assignment 2 of
 * Andrew Huang (s1913999)
 * Sebastiaan Alvarez Rodriguez (s1810979)

# Convention
In this section we explain our convention in detail.

## Communication process
We have a server `S` and a client `C`.

### Initial communication
`C` initiates communication by sending ACK to `S`.
> If `C` initiates in any other way, it will receive an EOS from the server, indicating it should stop trying to communicate.
> `C` might receive an EOS from `S`, even when `C` sends an ACK. This is because the server is full.

`S` registers client address details and then `S` will send an ACK to acknowledge the fact that it registered the client. Then `S` will wait for the first RR sent by `C`, with batchnumber 0. If `S` does not start sending anything, the communication initiation packet must have been faulty/dropped. `C` times out, and retries to initiate communication.

### Intermediate communication
`S` sends a batch of packets with a total size depending on current quality level.
`C` receives the packets of this batch. For a batch, the following may happen on the client side:
 1. Everything okay: take data from buffer and send RR to acknowledge next batch
 2. Packet(s) X and Y received out of order: Order X and Y, perform 1.
 3. Packet(s) X missing: `C` sends REJ a X to `S` after timeout, with a equals the batch number `C` from which given packet(s) should be resend. X is one or more numbers, equal to the packets which should be resent.
 4. Packet(s) X checksum mismatch (faulty), treat as missing, then perform 3.
 5. Full batch missing: `C` times out and sends REJ a X, with X equal to all packet numbers it expects.

### Final communication
`S` sends the last batch of packets. Again same rules apply as with the intermediate communication.
After `C` received last batch, it will send an RR to receive the next batch. `S` receives the RR flag, after which `S` will send an EOS flag, meaning end of the stream, and close the connection to `C`.
`S` can then reuse the connection slot for another client. `C` receives the EOS flag and will stop asking the server for packets.
`C` can then successfully clean up and exit, while `S` will wait for the next connection.
> If `C` does not receive the EOS (correctly), then `C` sends the server another `RR` (if not received at all) or `REJ` (if received faulty). `S` no longer knows `C`, and performs initial communication protocols. `C` does not open with `ACK`, thus `C` is send another EOS

### Quality changes
`C` determines the quality, by reviewing the amount of dropped/faulty packets and adjusting quality based on that. `C` sends QTY flag, with in the data section

## Convention visualization
Here below you can see what a packet looks like when sent/received, and you can find what every field means.
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

Flag | Bit    | Special arg | Meaning
---- | ------ | ----------- | -------------
None | `0x00` | No args     | No flag
ACK  | `0x01` | Buffersize  | Ready to receive batch 1
REJ  | `0x02` | Packetnr    | Reject specified packetnr
RR   | `0x04` | Batchnr     | Ready to receive next batch
QTY  | `0x08` | New quality | New quality
EOS  | `0x10` | No args     | End of stream
