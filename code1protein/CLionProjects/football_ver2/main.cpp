#include <iostream>
#include <tuple>
#include "randomNumbers.h"
#include "lattice.h"

#define dimX 10
#define dimY 10
#define dimZ 10

void updatePos(int ***array, int Xm, int Ym, int Zm, int length, int bid, int xRand, int yRand, int zRand) {
    int xVal = std::get<0>(coord(array, Xm, Ym, Zm, length, bid));
    int yVal = std::get<1>(coord(array, Xm, Ym, Zm, length, bid));
    int zVal = std::get<2>(coord(array, Xm, Ym, Zm, length, bid));
    // remove bid (or rod) from current position
    for (int i = 0; i < length; i++) {
        array[xVal][pos(yVal + i, Ym)][zVal] = 0;
    }
    // place bid (or rd) to a new place
    for (int j = 0; j < length; j++) {
        array[pos(xVal + xRand, Xm)][pos(yVal + yRand + j, Ym)][pos(zVal + zRand, Zm)] = bid;
    }
}


int main() {
    int ***array = allocate3dMatrix(dimX, dimY, dimZ);
    for (int x = 0; x < dimX; ++x) {
        for (int y = 0; y < dimY; ++y) {
            for (int z = 0; z < dimZ; ++z) {
                array[x][y][z] = 0;
            }
        }
    }
    int blocks = 50;
    int length = 3;
    double bondEn = 0.8;
    initialize(array, dimX, dimY, dimZ, blocks, length);
    printArray(array, dimX, dimY, dimZ);
    const int moveDistance = 2;
    int long iterations = 10;
    int long counter = 1;
    while (counter <= iterations) {
        int chosenBlock = randint(1, blocks);
        int xRand = randint(-moveDistance, moveDistance);
        int yRand = randint(-moveDistance, moveDistance);
        int zRand = randint(-moveDistance, moveDistance);
        if (moveCheck(array, dimX, dimY, dimZ, length, chosenBlock, xRand, yRand, zRand)) {
            if (energyCheck(array, dimX, dimY, dimZ, bondEn, length, chosenBlock, xRand, yRand, zRand)) {
                updatePos(array, dimX, dimY, dimZ, length, chosenBlock, xRand, yRand, zRand);
                if (counter % iterations == 0) {
                    printArray(array, dimX, dimY, dimZ);
                    printf("----%li----\n", counter);
                }

                counter++;
            }
        }
    }

    return 0;
}