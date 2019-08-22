//
// Created by minht on 8/21/2019.
//
#include <iostream>
#include <tuple>
#include "randomNumbers.h"

int* allocate3dMatrix(int zm, int ym, int xm){
    int* array = new int[xm * ym * zm];
    return array;
}

void printArray(int *array, int zm, int ym, int xm) {
    for (int z = 0; z < zm; z++) {
        for (int y = 0; y < xm; y++) {
            for (int x = 0; x < xm; x++) {
                std::cout << array[x + y*xm + z*xm*ym] << " " << std::flush;
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

bool checkSpace(const int *array, int z, int y, int x, int length, int xm, int ym) {
    for (int i = 0; i < length; i++) {
        if (array[x + pos(y + i, ym) * xm + z * xm * ym] != 0) return false;
    }
    return true;
}

void placeBlock(int *array, int z, int y, int x, int length, int xm, int ym, int currentBlock) {
    for (int i = 0; i < length; i++) {
        array[x + pos(y + i, ym)*xm + z * xm * ym] = currentBlock;
    }
}

void initialize(int *array, int zm, int ym, int xm, int blocks, int length) {
    for (int i = 0; i < blocks; i++) {
        int x = randint(0, xm - 1);
        int y = randint(0, ym - 1);
        int z = randint(0, zm - 1);
        if (checkSpace(array, z, y, x, length, xm, ym)) {
            placeBlock(array, z, y, x, length, xm, ym, i + 1);
        } else {
            while (true) {
                int xp = randint(0, xm - 1);
                int yp = randint(0, ym - 1);
                int zp = randint(0, zm - 1);
                if (checkSpace(array, zp, yp, xp, length, xm, ym)) {
                    placeBlock(array, zp, yp, xp, length, xm, ym, i + 1);
                    break;
                }

            }
        }
    }
}

std::tuple<int, int, int> coord(const int *array, int zm, int ym, int xm, int length, int currentBlock) {
    int posX = 0;
    int posY = 0;
    int posZ = 0;
    for (int z = 0; z < zm; ++z) {
        for (int y = 0; y < ym; ++y) {
            for (int x = 0; x < xm; ++x) {
                if (array[x + y*xm + z * xm * ym] == currentBlock) {
                    if (array[x + ((y - 1 + ym) % ym)*xm + z * xm * ym] == currentBlock) {
                        for (int i = 0; i < length - 1; ++i) {
                            if (array[x + ((y - 1 + ym) % ym)*xm + z * xm * ym] != currentBlock) {
                                posX = x;
                                posY = (y - 1 + ym + i) % ym;
                                posZ = z;
                                goto stop;
                            }
                        }
                    } else {
                        posX = x;
                        posY = y;
                        posZ = z;
                        goto stop;
                    }

                }
            }
        }
    }
    stop:
    return std::make_tuple(posX, posY, posZ);
}

bool
energyCheck(int *array, int zm, int ym, int xm, double bondEn, int length, int bid, int xRand, int yRand, int zRand) {
    int energy1 = 0;
    int energy2 = 0;
    int xVal = std::get<0>(coord(array, zm, ym, xm, length, bid));
    int yVal = std::get<1>(coord(array, zm, ym, xm, length, bid));
    int zVal = std::get<2>(coord(array, zm, ym, xm, length, bid));
    for (int y = 0; y < length; y++) {
        for (int z = -1; z <= 1; z += 2) {
            if (array[xVal + pos(yVal + y, ym)*xm + pos(zVal + z, zm)*xm*ym] != 0 &&
                array[xVal + pos(yVal + y, ym) + pos(zVal + z, zm)*xm*ym] != bid)
                energy1++;
            if (array[pos(xVal + xRand, xm) + pos(yVal + y + yRand, ym)*xm + pos(zVal + z + zRand, zm)*xm*ym] != 0 &&
                array[pos(xVal + xRand, xm) + pos(yVal + y + yRand, ym)*xm + pos(zVal + z + zRand, zm)*xm*ym] != bid)
                energy2++;
        }
    }
    for (int y = 0; y < length; y++) {
        for (int x = -1; x <= 1; x += 2) {
            if (array[pos(xVal + x, xm) + pos(yVal + y, ym)*xm + zVal*xm*ym] != 0 &&
                array[pos(xVal + x, xm) + pos(yVal + y, ym)*xm + zVal*xm*ym] != bid)
                energy1++;
            if (array[pos(xVal + x + xRand, xm) + pos(yVal + y + yRand, ym)*xm + pos(zVal + zRand, zm)*xm*ym] != 0 &&
                array[pos(xVal + x + xRand, xm) + pos(yVal + y + yRand, ym)*xm + pos(zVal + zRand, zm)*xm*ym] != bid)
                energy2++;
        }
    }
    double dE = energy2 - energy1;
    //printf("E1: %d, E2: %d, dE: %.1f\n", energy1, energy2, dE);
    if (dE >= 0) return true;
    else {
        double boltzmannWeight = exp(dE * bondEn);
        double r = randDouble();
        //printf("r: %f, Boltzmann weight: %f\n", r, boltzmannWeight);
        return r < boltzmannWeight;
    }

}

bool moveCheck(int *array, int zm, int ym, int xm, int length, int bid, int xRand, int yRand, int zRand) {
    int xVal = std::get<0>(coord(array, zm, ym, xm, length, bid));
    int yVal = std::get<1>(coord(array, zm, ym, xm, length, bid));
    int zVal = std::get<2>(coord(array, zm, ym, xm, length, bid));
    for (int i = 0; i < length; i++) {
        if (array[pos(xVal + xRand, xm) + pos(yVal + yRand + i, ym)*xm + pos(zVal + zRand, zm)*xm*ym] != 0 &&
            array[pos(xVal + xRand, xm) + pos(yVal + yRand + i, ym)*xm + pos(zVal + zRand, zm)*xm*ym] != bid)
            return false;
    }
    return true;
}