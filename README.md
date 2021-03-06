# Music Protocol
Simple audio-streaming protocol in C.


# Convention
In this section we explain our convention in detail.


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
 * Checksum1: This is a 16-bit Fletcher checksum for fields size, flags, nr, and checksum2
 * Size:      The size of the data buffer
 * Flags:     This is a 8-bit field, containing bitflags. See further below for possible flags and their meaning
 * Nr:        The packet number, to make sure out-of-order packets can be detected
 * Checksum2: This is a 16-bit Fletcher checksum for the data-field
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


## Communication process
We have a server `S` and a client `C`.

### Initial communication
`C` initiates communication by sending ACK to `S`.
> If `C` initiates in any other way, it will receive an EOS 
from the server, indicating it should stop trying to communicate.
> `C` might receive an EOS from `S`, even when `C` sends an ACK. 
This is because the server is full.  

`S` registers client address details and then `S` will send an ACK to
acknowledge the fact that it registered the client. Then `S` will wait for
the first RR sent by `C`. The following may happen:
 1. Everything okay: Initial communication established.
 2. Communication initiation packet (`C` to `S`) failure: `C` times out, because `S` does not send anything. `C` retries to initiate communication.
 3. ACK from `S` to `C` is faulty/dropped: `S` processed initial communication. `C` times out. `C` retries to initiate communication.

### Intermediate communication
`C` sends RR to `S` with batch number.
`S` sends packets belonging to batch, with packet amount depending 
on current quality level.
`C` receives packets of batch. The following may happen:
 1. Everything okay: take data from buffer and send RR to
 acknowledge next batch
 2. RR from `C` to `S` faulty/dropped: `C` times out. `C` sends a REJ,
 requesting all packets of batch, essentially retrying the RR.
 3. Packet(s) X and Y received out of order: Order X and Y, perform 1.
 4. Packet(s) X missing: `C` will time out. `C` sends REJ a X to `S` after
 timeout, with a equals the batch number `C` from which given packet(s)
 should be resend. X is one or more numbers, equal to the packets which
 should be resent.
 5. Packet(s) X checksum mismatch (faulty), treat as missing, then perform 3.
 6. Full batch missing: `C` times out and sends REJ a X, with X equal to all
 packet numbers it expects.

### Final communication
`S` sends the last batch of packets. If the data is not batch-alligned, batch
will be packed with packets of 0 bytes. Because of this, now same rules apply
as with intermediate communication.
After this, `C` will send an RR to receive the next batch. `S` receives the
RR flag, after which `S` will send an EOS flag, meaning end of the stream,
and close the connection to `C`.
`S` can then reuse the connection slot for another client. `C` receives the
EOS flag and will stop asking the server for packets.
`C` can then successfully clean up and exit, while `S` will wait for the next
connection. The following may happen:
 1. Everything okay: final communication successful.
 2. RR from `C` to `S` faulty/dropped: `C` times out. `C` sends a REJ,
 essentially retrying the RR.
 3. EOS flag from `S` to `C` faulty/dropped: `C` times out, sends a REJ,
 essentialy retrying the RR.

> If `C` does not receive the EOS (correctly) in any way, then `C` sends `S`
a `REJ`. `S` no longer knows `C`, and performs initial communication
protocols. `C` does not open with `ACK`, thus `C` is sent another EOS.

### Quality changes
`C` determines the quality, by reviewing amount of dropped/faulty packets
versus amount of healthy packets. When quality should be adjusted, `C` sends
QTY flag, with in the data section one uint8_t containing new quality. `S`
receives and registers new quality parameter. `S` sends an ACK to acknowledge
receival. There are precise limits when a QTY flag may be sent: Never while
batches are being sent. Never when a batch had missing/faulty packets and REJ
should be sent. Only between two successfully retrieved batches.  
The following may happen:
 1. Everything okay: quality adjustment successful.
 2. QTY communication (from `C` to `S`) failure: `C` receives no ACK, times
 out, and restarts QTY communication
 3. ACK from `S` to `C` faulty/dropped: `C` receives no ACK, times out, and
 restarts QTY communication.

### Quality implications
Quality is given with 1,2,3,4,5. The higher the number, the higher the
quality. Quality implications:
 1. Batches are only 10KB, consisting of 40 packets. Downsampling is
 performed to reduce packet size. One every 8 frames is dropped. Music
 quality drops very much. Also, compression is performed. Total packet size
 reduction is close to 25%.
 2. Batches are 15KB, consisting of 60 packets. Compression is performed to
 reduce packet size by 12.5%.
 3. Batches are 25KB, consisting of 100 packets.
 4. Batches are 50KB, consisting of 200 packets.
 5. Batches are 63.75KB, consisting of 255 packets.  

> IMPORTANT: If quality equals 1, music frames are dropped (we apply
downsampling). This sounds absolutely horrible on almost al music, commonly
introducing a tone, which was not in the original music. This is as it is
supposed to be with downsampling.



# Design choices
Here, we will explain our most important design choices.


## Burst protocol: Safety of checking one by one, but fast
For our protocol, we wanted to implement something fast and safe, which uses
network cables for as little time as possible, such that other connected
machines to the same wire could use it without hindrance. For this purpose,
we decided that the optimal solution would be in a protocol 'bursting'
packets. The protocol follows the following pattern:
 1. initial communication
 2. `C` requests batch `a`.
 3. `S` sends batch `a`, containing `x` packets, with `x` depending on quality.
 4. `C` verifies batch, possibly requesting resend for missing/faulty packets
 (also in a batch). A REJ is sent in case there exist missing/faulty packets, which is blocking. 
 5. `C` checks if quality level is okay, possibly telling server to change
 quality.
 6. Go back to 2, except for when an EOS has been received.  
In the subsections below, we will go in further detail about this.


## Batches and qualities
When creating this protocol, we figured: If a connection is unreliable, we
should perform more checks per time unit, to allow for a higher throughput of
correct packets. Connections may change from reliable to unreliable very
quickly. Thus, we decided that quality should decrease fast when connection
reliability decreases, but should increase slowly when connection seems to be
reliable enough.  
 * Quality must increase if there was a problem with less than 6.25% of the
 packets in last 5 batches.
 * Quality must decrease if there was a problem with more than 12.5% of the
 packets in last 5 batches.  

For a lower quality, we send fewer packets per burst, or 'batch'. For a
higher quality, we send more. For this, see Section [Quality implications](#quality-implications) above.


## Batches and buffers
The only drawback of this 'burst' protocol approach is: a batch must fit in
the client's buffer. This tends to become a problem when a very small buffer
size is chosen. Therefore, we set a minimum buffer size of 2 times the
highest possible batch size. This comes down to 2 times 63.75KB = 127.5KB,
about 128KB, as can be seen in Section [Quality implications](#quality-implications).  
We decided to multiply the highest possible batch size by 2 as minimum, such 
that no underruns can happen by default.  
If there is a maximum quality stream (63.75KB batches) and we would use a
63.75KB buffer, we must wait with requesting next batch until there is enough
space (63.75 KB free) in the buffer. In other words: we could only request a
batch, when our buffer is empty. An empty buffer means there is no more music
to play, and ALSA lib would experience buffer underruns and music would stop.
This is undesirable behaviour, thus we made our minimum buffer easy-fitting.


## Who gets to do what
All different tasks may be spread between client and server in many ways. In
our approach, we made these kinds of design decisions with multi-client
support in mind.

### Client
The client is responsible for:
 - All error detections and solutions
 - Reording packets in case of misordered receival
 - adjusting quality in case of need
The meaning of this is: Client must measure connection reliability.

### Server
The server is responsible for:
 - Keeping track of all different clients
 - Keeping track of what music data the current client should retrieve
This means: In case of any missing/faulty packets from client to server, the server just ignores the request and lets client timeout.


## Timeouts
Since our protocol is based on client side error handling and we let client
timeout if communication from client to server fails, we had to set a low
timeout, as to not waste too much time until client finally moves on and
resolves errors. Timeout is currently set to 16 milliseconds.



# Simulation operations
At this point, we support 4 different simulation options:
 * Package reordering
 * Bit flipping
 * Package dropping
 * Package delaying

Each option can be given a chance. Some options have extra suboptions.
Package delaying, per example, has both a minimal and maximal wait time suboption.  


## Simulation defaults
Per default, our simulation setting is as follows:
 - `Package reordering chance`: 35%
    -`Package swap chance`: 25%
 - `Bit flip occurs chance`: 0%
    -`Bit flip chance`: 15%
 - `Package drop chance`: 5%
 - `Package wait chance`: 5%
    - `Minimum wait`: 1  (ms)
    - `Maximum wait`: 10 (ms)
There is also an option to print when one of 4 options above happen (quite
verbose for high chances, as more options get more frequently applied).

In `src/communication/simulation/simulation.h` all defines are located, which
are used to set options.



# Experiments
To test the power of our methods, we have done the following experiments.
When we talk about 'Network speed', we mean for the client: 
The usable input of bytes/sec. Faulty/lost packets, which had to be requested
once more, are not counted double in bytes, but their time is added to
seconds. Thus, the network speed shows how many usable bytes arrive 
per second on average.  

When we talk about 'Network speed', we mean for the server: 
The usable output of bytes/sec. Faulty/lost packets, which were requested
once more, are not counted double in bytes, but their time is added to
seconds. Thus, the network speed shows how many usable bytes depart per second.

We used our own laptops to test on, using localhost ip. All tests are run on borderlands.wav, a self-created wav file with playtime of `212` seconds and size of `37398572` bytes. We chose this file since experiments with a bigger file produce more accurate result averages.

Lastly, all byte-related statistics are given in KB, MB, etcetera instead of in KiB, MiB and the like.


## Alpha
We tested without simulated unreliable networking. We got with an
average network speed of 24.11 MB/s with the client.


## Beta
We tested the default settings (see 'Simulation defaults' section above).
With the client, we got an average network speed of 19.04 MB/s.


## Gamma
When testing with very hard simulation settings from below, we got an average
network speed of 8.86 MB/s. There were twice as many package 
drops, and a faulty packet modifier of 10 percent has been introduced, making
the total network stability four times less reliable in total. We expect a 
4.0 relative malus ratio, but we only see a (19.04/8,86=) 2.14 relative malus
ratio. The 1.86 relative malus ratio we saved was due to the quality 
behaviour adaptions we implemented, as discussed in Section [Quality implications](#quality-implications).

```c
#define SIMULATE
#ifdef SIMULATE
    // Use only if you want to print verbose information about simulation
    // #define SIMULATE_PRINT

    // Use if you want to simulate packet reordering
    #define SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE 40.0f
    #ifdef SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE
        #define SIMULATE_RANDOMIZE_PACKET_ORDER_SWAP_CHANCE 25.0f
    #endif

    // Use if you want to simulate bit flipping
    #define SIMULATE_BIT_SOME_FLIP_CHANCE 10.0f
    #ifdef SIMULATE_BIT_SOME_FLIP_CHANCE
        #define SIMULATE_BIT_FLIP_CHANCE 15.0f
    #endif

    // Use if you want to simulate packet dropping
    #define SIMULATE_DROP_PACKET_CHANCE 10.0f

    // Use if you want to simulate packet waiting
    #define SIMULATE_RANDOM_WAIT_CHANCE 5.0f
    #ifdef SIMULATE_RANDOM_WAIT_CHANCE
        // Times below represent ms, must be integer
        #define SIMULATE_RANDOM_WAIT_MIN 1
        #define SIMULATE_RANDOM_WAIT_MAX 10
    #endif
#endif
```
