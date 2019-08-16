#include <iostream>
#include <tuple>
#include "randomNumbers.h"
#include "lattice.h"

#define dimX 5
#define dimY 5
#define dimZ 5

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
    int blocks = 10;
    int length = 3;
    double bondEn = 0.8;
    initialize(array, dimX, dimY, dimZ, blocks, length);
    printArray(array, dimX, dimY, dimZ);
    const int moveDistance = 2;
    int chosenBlock = randint(1, blocks);
    int xPos = std::get<0>(coord(array, dimX, dimY, dimZ, length, chosenBlock));
    int yPos = std::get<1>(coord(array, dimX, dimY, dimZ, length, chosenBlock));
    int zPos = std::get<2>(coord(array, dimX, dimY, dimZ, length, chosenBlock));
    int xRand = randint(-moveDistance, moveDistance);
    int yRand = randint(-moveDistance, moveDistance);
    int zRand = randint(-moveDistance, moveDistance);
    printf("block %d has current position at (%d, %d, %d)\n", chosenBlock, xPos, yPos, zPos);
    printf("proposed move dL = (%d, %d, %d)\n", xRand, yRand, zRand);
    printf("move to (%d, %d, %d)\n", pos(xPos + xRand, dimX), pos(yPos + yRand, dimY), pos(zPos + zRand, dimZ));
    if (moveCheck(array, dimX, dimY, dimZ, length, chosenBlock, xRand, yRand, zRand)) {
        if (energyCheck(array, dimX, dimY, dimZ, bondEn, length, chosenBlock, xRand, yRand, zRand)) {
            updatePos(array, dimX, dimY, dimZ, length, chosenBlock, xRand, yRand, zRand);
            printf("Move accepted!\n");
            printArray(array, dimX, dimY, dimZ);
        } else
            printf("Reject this move!\n");
    } else printf("occupied site!");


    return 0;
}