//
// Created by minht on 8/21/2019.
//

#ifndef FOOTBALL_VER3_LATTICE_H
#define FOOTBALL_VER3_LATTICE_H

int* allocate3dMatrix(int zm, int ym, int xm);

void printArray(int *array, int zm, int ym, int xm);

int pos(int val, int max);

bool checkSpace(const int *array, int z, int y, int x, int length, int xm, int ym);

void placeBlock(int *array, int z, int y, int x, int length, int xm, int ym, int currentBlock);

void initialize(int *array, int zm, int ym, int xm, int blocks, int length);

std::tuple<int, int, int> coord(const int *array, int zm, int ym, int xm, int length, int currentBlock);

bool
energyCheck(int *array, int zm, int ym, int xm, double bondEn, int length, int bid, int xRand, int yRand, int zRand);

bool moveCheck(int *array, int zm, int ym, int xm, int length, int bid, int xRand, int yRand, int zRand);

#endif //FOOTBALL_VER3_LATTICE_H
