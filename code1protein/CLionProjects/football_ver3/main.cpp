#include <iostream>
#include <tuple>
#include "lattice.h"
#include "randomNumbers.h"

#define dimZ 3
#define dimY 5
#define dimX 5

void updatePos(int *array, int zm, int ym, int xm, int length, int bid, int xRand, int yRand, int zRand) {
    int xVal = std::get<0>(coord(array, zm, ym, xm, length, bid));
    int yVal = std::get<1>(coord(array, zm, ym, xm, length, bid));
    int zVal = std::get<2>(coord(array, zm, ym, xm, length, bid));
    // remove bid (or rod) from current position
    for (int i = 0; i < length; i++) {
        array[xVal + pos(yVal + i, ym)*xm + zVal*xm*ym] = 0;
    }
    // place bid (or rd) to a new place
    for (int j = 0; j < length; j++) {
        array[pos(xVal + xRand, xm) + pos(yVal + yRand + j, ym)*xm + pos(zVal + zRand, zm)*xm*ym] = bid;
    }
}

int main() {
    int* array = allocate3dMatrix(dimZ, dimY, dimX);
    for (int z = 0; z < dimZ; ++z) {
        for (int y = 0; y < dimY; ++y) {
            for (int x = 0; x < dimX; ++x) {
                array[x + y*dimX + z*dimX*dimY] = 0;
            }
        }
    }

    int blocks = 9;
    int length = 3;
    double bondEn = 0.8;
    initialize(array, dimZ, dimY, dimX, blocks, length);
    printArray(array, dimZ, dimY, dimX);
    const int moveDistance = 2;
    int chosenBlock = 1;     // randint(1, blocks);
    int xPos = std::get<0>(coord(array, dimZ, dimY, dimX, length, chosenBlock));
    int yPos = std::get<1>(coord(array, dimZ, dimY, dimX, length, chosenBlock));
    int zPos = std::get<2>(coord(array, dimZ, dimY, dimX, length, chosenBlock));
    int xRand = randint(-moveDistance, moveDistance);
    int yRand = randint(-moveDistance, moveDistance);
    int zRand = randint(-moveDistance, moveDistance);
    printf("block %d has current position at (%d, %d, %d)\n", chosenBlock, xPos, yPos, zPos);
    printf("proposed move dL = (%d, %d, %d)\n", xRand, yRand, zRand);
    printf("move to (%d, %d, %d)\n", pos(xPos + xRand, dimX), pos(yPos + yRand, dimY), pos(zPos + zRand, dimZ));
    if (moveCheck(array, dimZ, dimY, dimX, length, chosenBlock, xRand, yRand, zRand)) {
        if (energyCheck(array, dimZ, dimY, dimX, bondEn, length, chosenBlock, xRand, yRand, zRand)) {
            updatePos(array, dimZ, dimY, dimX, length, chosenBlock, xRand, yRand, zRand);
            printf("Move accepted!\n");
            printArray(array, dimZ, dimY, dimX);
        } else
            printf("Reject this move!\n");
    } else printf("occupied site!");

    return 0;
}