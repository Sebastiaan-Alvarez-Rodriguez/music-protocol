#include <unistd.h>
#include <stdio.h>
#include "constants.h"

///////////////////////////////////////////////////
// Important - Read me
//
// batchsize | quality
// 010 kb    | 1
// 015 kb    | 2
// 025 kb    | 3
// 050 kb    | 4
// 100 kb    | 5
//
// packet size = constant 256 bytes = 0.25 kb
// (512 bytes is considered 'safe' for UDP protocol (no MTU exceeds))
// (For streaming services, many small packets are considered better)
// 
// packets per batch | quality
// 010 / 0.25 = 040  | 1
// 015 / 0.25 = 060  | 2
// 025 / 0.25 = 100  | 3
// 050 / 0.25 = 200  | 4
// 100 / 0.25 = 400  | 5
//
///////////////////////////////////////////////////


size_t constants_batch_size(unsigned quality) {
    switch (quality) {
        case 1:
            return 10240; // 10 * 1024
        case 2:
            return 15360; // 15 * 1024
        case 3:
            return 25600; // 25 * 1024
        case 4:
            return 51200; // 50 * 1024
        case 5:
            return 102400;//100 * 1024
        default:
            printf("Unacceptable quality '%u' specified \n", quality);
            return 0;
    }
}


size_t constants_batch_packets_amount(unsigned quality) {
    switch (quality) {
        case 1:
            return 40; //  010240 / 256
        case 2:
            return 60; //  015360 / 256
        case 3:
            return 100; // 025600 / 256
        case 4:
            return 200; // 051200 / 256
        case 5:
            return 400; // 102400 / 256
        default:
            printf("Unacceptable quality '%u' specified \n", quality);
            return 0;
    }
}


size_t constants_packets_size() {
    return 256;
}