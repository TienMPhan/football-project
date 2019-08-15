//
// Created by minht on 8/15/2019.
//
#include <iostream>
#include <tuple>
#include "randomNumbers.h"

int ***allocate3dMatrix(int dimX, int dimY, int dimZ) {
    int ***matrix = new int **[dimX];
    for (int x = 0; x < dimX; ++x) {
        matrix[x] = new int *[dimY];
        for (int y = 0; y < dimY; ++y) {
            matrix[x][y] = new int[dimZ];
        }
    }
    return matrix;
}

void printArray(int ***arr, int xm, int ym, int zm) {
    for (int z = 0; z < zm; z++) {
        for (int x = 0; x < xm; x++) {
            for (int y = 0; y < ym; y++) {
                std::cout << arr[z][x][y] << " " << std::flush;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int pos(int val, int max) {
    if (val >= max) val -= max;
    if (val < 0) val += max;
    return val;
}

bool checkSpace(int ***arr, int x, int y, int z, int length, int Ym) {
    for (int i = 0; i < length; i++) {
        if (arr[x][pos(y + i, Ym)][z] != 0) return false;
    }
    return true;
}

void placeBlock(int ***arr, int x, int y, int z, int length, int Ym, int currentBlock) {
    for (int i = 0; i < length; i++) {
        arr[x][pos(y + i, Ym)][z] = currentBlock;
    }
}

void initialize(int ***array, int Xm, int Ym, int Zm, int blocks, int length) {
    for (int i = 0; i < blocks; i++) {
        int x = randint(0, Xm - 1);
        int y = randint(0, Ym - 1);
        int z = randint(0, Zm - 1);
        if (checkSpace(array, x, y, z, length, Ym)) {
            placeBlock(array, x, y, z, length, Ym, i + 1);
        } else {
            while (true) {
                int xp = randint(0, Xm - 1);
                int yp = randint(0, Ym - 1);
                int zp = randint(0, Zm - 1);
                if (checkSpace(array, xp, yp, zp, length, Ym)) {
                    placeBlock(array, xp, yp, zp, length, Ym, i + 1);
                    break;
                }

            }
        }
    }
}

std::tuple<int, int, int> coord(int ***array, int Xm, int Ym, int Zm, int currentBlock) {
    int posX = 0;
    int posY = 0;
    int posZ = 0;
    for (int x = 0; x < Xm; ++x) {
        for (int y = 0; y < Ym; ++y) {
            for (int z = 0; z < Zm; ++z) {
                if (array[x][y][z] == currentBlock) {
                    posX = x;
                    posY = y;
                    posZ = z;
                    goto stop;
                }
            }
        }
    }
    stop:
    return std::make_tuple(posX, posY, posZ);
}