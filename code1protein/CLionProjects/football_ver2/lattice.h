//
// Created by minht on 8/15/2019.
//

#ifndef FOOTBALL_VER2_LATTICE_H
#define FOOTBALL_VER2_LATTICE_H

int ***allocate3dMatrix(int dim_z, int dim_x, int dim_y);

void printArray(int ***arr, int xm, int ym, int zm);

int pos(int val, int max);

bool checkSpace(int ***arr, int x, int y, int z, int length, int dim);

void placeBlock(int ***arr, int x, int y, int z, int length, int Ym, int currentBlock);

void initialize(int ***array, int Xm, int Ym, int Zm, int blocks, int length);

std::tuple<int, int, int> coord(int ***array, int Xm, int Ym, int Zm, int currentBlock);

#endif //FOOTBALL_VER2_LATTICE_H
