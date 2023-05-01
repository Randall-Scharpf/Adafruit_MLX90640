#include <fstream>
#include <iostream>
#include <stdint.h>

#include "lib/MLX90640.h"

using namespace std;

// build with `g++ main.cpp lib/*.cpp -o frame-data-converter

uint8_t makeByte(char* digits) {
    for (int i = 0; i < 2; i++) {
        if (digits[i] >= 'A') {
            digits[i] -= 'A';
            digits[i] += 10;
        } else {
            digits[i] -= '0';
        }
    }
    return 16*digits[0] + digits[1];
}

bool ishexdig(char c) {
    return ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F'));
}

int loadFile(char* fname, uint8_t* buffer, int maxlen) {
    ifstream in;
    in.open(fname);
    char nextByte[2];
    int num_bytes = 0;
    for (num_bytes = 0; num_bytes < maxlen; num_bytes++) {
        int nybblePos = 0;
        for (;;) {
            in.read(nextByte + nybblePos, 1);
            if (in.fail() && (nybblePos == 1)) {
                fprintf(stderr, "Warning: An odd number of hex digits were provided in %s\r\n", fname);
                nextByte[1] = '0';
                buffer[num_bytes] = makeByte(nextByte);
                goto eof;
            } else if (in.fail() || in.eof()) {
                fprintf(stderr, "Warning: only %d bytes were parsesd from %s\r\n", num_bytes, fname);
                goto eof;
            } else {
                if (ishexdig(nextByte[nybblePos])) {
                    nybblePos++;
                    if (nybblePos == 2) {
                        buffer[num_bytes] = makeByte(nextByte);
                        goto eobyte;
                    }
                }
            }
        }
        eobyte:
        continue;
    }
    eof:
    in.close();
    return num_bytes;
}

int main() {
    uint8_t eeData[1664] = {0};
    char ee_fname[] = "eeprom_data.hex";
    loadFile(ee_fname, eeData, 1664);
    MLX90640_DataProcessor proc;
    proc.loadCalibration(eeData);
    uint8_t frameData[1672] = {0};
    char frame_fname[] = "frame_data.hex";
    loadFile(frame_fname, frameData, 1672);
    float pixels[768] = {0};
    proc.decodeFrame(frameData, pixels);
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 32; j++) {
            fprintf(stdout, "%.5f%s", pixels[i*32+j], j==31?"\n":" ");
        }
    }
}