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
`S` registers client address details and then `S` will send an ACK to acknowledge the fact that it registered the client. Then `S` will wait for the first RR sent by `C`. The following may happen:
 1. Everything okay: Initial communication established
 2. Communication initiation packet (`C` to `S`) failure: `C` times out, because `S` does not send anything. `C` retries to initiate communication.
 3. ACK from `S` to `C` is faulty/dropped: `S` processed initial communication. `C` times out. `C` retries to initiate communication.

### Intermediate communication
`C` sends RR to `S` with batch number.
`S` sends packets belonging to batch, with packet amount depending on current quality level.
`C` receives packets of batch. The following may happen:
 1. Everything okay: take data from buffer and send RR to acknowledge next batch
 2. RR from `C` to `S` faulty/dropped: `C` times out. `C` sends a REJ, requesting all packets of batch, essentially retrying the RR.
 3. Packet(s) X and Y received out of order: Order X and Y, perform 1.
 4. Packet(s) X missing: `C` will time out. `C` sends REJ a X to `S` after timeout, with a equals the batch number `C` from which given packet(s) should be resend. X is one or more numbers, equal to the packets which should be resent.
 5. Packet(s) X checksum mismatch (faulty), treat as missing, then perform 3.
 6. Full batch missing: `C` times out and sends REJ a X, with X equal to all packet numbers it expects.

### Final communication
`S` sends the last batch of packets. If the data is not batch-alligned, batch will be packed with packets of 0 bytes. Because of this, now same rules apply as with intermediate communication.
After this, `C` will send an RR to receive the next batch. `S` receives the RR flag, after which `S` will send an EOS flag, meaning end of the stream, and close the connection to `C`.
`S` can then reuse the connection slot for another client. `C` receives the EOS flag and will stop asking the server for packets.
`C` can then successfully clean up and exit, while `S` will wait for the next connection. The following may happen:
 1. Everything okay: final communication successful.
 2. RR from `C` to `S` faulty/dropped: `C` times out. `C` sends a REJ, essentially retrying the RR.
 3. EOS flag from `S` to `C` faulty/dropped: `C` times out, sends a REJ, essentialy retrying the RR.

> If `C` does not receive the EOS (correctly) in any way, then `C` sends `S` a `REJ`. `S` no longer knows `C`, and performs initial communication protocols. `C` does not open with `ACK`, thus `C` is sent another EOS.

### Quality changes
`C` determines the quality, by reviewing amount of dropped/faulty packets versus amount of healthy packets. When quality should be adjusted, `C` sends QTY flag, with in the data section one uint8_t containing new quality. `S` receives and registers new quality parameter. `S` sends an ACK to acknowledge receival. The following may happen:
 1. Everything okay: quality adjustment successful.
 2. QTY communication (from `C` to `S`) failure: `C` receives no ACK, times out, and restarts QTY communication
 3. ACK from `S` to `C` faulty/dropped: `C` receives no ACK, times out, and restarts QTY communication.

### Quality implications
Quality is given with 1,2,3,4,5. The higher the number, the higher the quality. Quality implications:
 1. Batches are only 10KB, consisting of 40 packets. Downsampling is performed to reduce packet size. One every 8 frames is dropped. Music quality drops very much. Also, compression is performed. Total packet size reduction is close to 25%.
 2. Batches are 15KB, consisting of 60 packets. Compression is performed to reduce packet size by 12.5%.
 3. Batches are 25KB, consisting of 100 packets.
 4. Batches are 50KB, consisting of 200 packets.
 5. Batches are 63.75KB, consisting of 255 packets.  

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
