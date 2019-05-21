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
// 63.75 kb  | 5
//
// packet size = constant 256 bytes = 0.25 kb
// (512 bytes is considered 'safe' for UDP protocol (no MTU exceeds))
// (For streaming services, many small packets are considered better)
// 
// packets per batch   | quality
// 10.00 / 0.25 = 040  | 1
// 15.00 / 0.25 = 060  | 2
// 25.00 / 0.25 = 100  | 3
// 50.00 / 0.25 = 200  | 4
// 63.75 / 0.25 = 255  | 5
//
///////////////////////////////////////////////////


size_t constants_batch_size(unsigned quality) {
    switch (quality) {
        case 1:
            return 10240; // 10.00 * 1024
        case 2:
            return 15360; // 15.00 * 1024
        case 3:
            return 25600; // 25.00 * 1024
        case 4:
            return 51200; // 50.00 * 1024
        case 5:
            return 65280; // 63.75 * 1024
        default:
            printf("Unacceptable quality '%u' specified \n", quality);
            return 0;
    }
}


size_t constants_batch_packets_amount(unsigned quality) {
    switch (quality) {
        case 1:
            return 40; //  10240 / 256
        case 2:
            return 60; //  15360 / 256
        case 3:
            return 100; // 25600 / 256
        case 4:
            return 200; // 51200 / 256
        case 5:
            return 255; // 64512 / 256
        default:
            printf("Unacceptable quality '%u' specified \n", quality);
            return 0;
    }
}


size_t constants_packets_size() {
    return 256;
}