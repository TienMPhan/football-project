#include <iostream>
#include <string>
#include <random>
#include <tuple>
#include <chrono>
#include "time_c.h"
#include "memory_c.h"

using namespace std;

#define Xm 200
#define Ym 500
#define Zm 200

// lattice and coordinate array
int **allocate2dMatrix(int blocks, int dimension);
void deallocate2dMatrix(int **coord, int blocks);
int ***allocate3dMatrix(int dimX, int dimY, int dimZ);
void deallocate3dMatrix(int ***matrix, int dimX, int dimY);

// random number
int randint(int lower, int upper);
double randDouble();
// Metropolis algorithm functions
int pos(int val, int max);
bool checkSpace(int ***arr, int x, int y, int z, int length);
void placeBlock(int ***arr, int x, int y, int z, int length, int currentBlock);
void placeCord(int **cord, int x, int y, int z, int currentBlock);
void initialize(int ***array, int **cord, int blocks, int length);
bool energyCheck(int ***array, int **cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand);
bool moveCheck(int ***array, int **cord, int length, int bid, int xRand, int yRand, int zRand);
void updatePos(int ***array, int **cord, int length, int bid, int xRand, int yRand, int zRand);
// parse params
tuple<string, string, string, string, string, string> parseParams(int argc, char *argv[]);
// write data
void writexyz(int ***lattice, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep);
void writeLattice(int ***lattice, double bondEn, int length, int runId, unsigned long count, unsigned long split, int rep);
void writeCoordinates(int **coord, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep);

default_random_engine dre(chrono::steady_clock::now().time_since_epoch().count());

int main(int argc, char* argv[]) {

#ifdef DEBUG
    double elapsedTime;
	struct timeval t1, t2;
	processMem_t mem;

    gettimeofday(&t1, NULL);
#endif

    if (argc != 13)
    {
        printf("err: check input parameters!\n");
    }
    // parse input params
    tuple<string, string, string, string, string, string> parsedParamsTuple = parseParams(argc, argv);
    double bondEn = stof(get<0>(parsedParamsTuple));
    int blocks = stoi(get<1>(parsedParamsTuple));
    int length = stoi(get<2>(parsedParamsTuple));
    unsigned long iterations = stoul(get<3>(parsedParamsTuple));
    unsigned long split = stoul(get<4>(parsedParamsTuple));
    int runId = stoi(get<5>(parsedParamsTuple));

    // writing id
    int rep_write = 1;

    // make directory
    string strEn = to_string(bondEn);
    system(("mkdir -p En-" + strEn).c_str());

    int dimension = 4;
    // allocate memory and initialize lattice with 0
    int ***lattice = allocate3dMatrix(Xm, Ym, Zm);
    // allocate memmory and initialize coord with 0
    int **coord = allocate2dMatrix(blocks, dimension);
    // create initial state
    initialize(lattice, coord, blocks, length);
    unsigned long count = 1;
    int moveDistance = 1;
    while (count <= iterations)
    {
        int bid = randint(1, blocks);
        int xRand = randint(-moveDistance, moveDistance);
        int yRand = randint(-moveDistance, moveDistance);
        int zRand = randint(-moveDistance, moveDistance);
        if (moveCheck(lattice, coord, length, bid, xRand, yRand, zRand))
        {
            if (energyCheck(lattice, coord, bondEn, length, bid, xRand, yRand, zRand))
            {
                updatePos(lattice, coord, length, bid, xRand, yRand, zRand);
                if (count % split == 0)
                {
                    writexyz(lattice, bondEn, blocks, length, runId, count, split, rep_write);
                    writeLattice(lattice, bondEn, length, runId, count, split, rep_write);
                    writeCoordinates(coord, bondEn, blocks, length, runId, count, split, rep_write);
                }
                count++;
            }
        }
    }

    deallocate2dMatrix(coord, blocks);
    deallocate3dMatrix(lattice, Xm, Ym);

#ifdef DEBUG
        gettimeofday(&t2, NULL);
		elapsedTime = getTimeDifferenceInMilliseconds(&t1, &t2);
		getProcessMemory(&mem);

        printf("\nEn-%f/C%.2fL%d-run%d-id-%d\n\tElapsed Time (ms): %f\n\tPhysical Memory (kB): %u\n\tVirtual Memory (kB): %u\n",
            bondEn, bondEn, length, rep_write, runId, elapsedTime, mem.physicalMem, mem.virtualMem);
#endif

    return 0;
}
void writeLattice(int ***lattice, double bondEn, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char latticeFileName[128];
    char printBuffer[Xm * Ym * Zm * 13];
    int **latticeX, *latticeY;
    snprintf(latticeFileName, 128, "En-%f/L%.2fL%d-run%d-id-%d-split-%u.txt", bondEn, bondEn, length, rep, runId, (count / split));
    for (int x = 0; x < Xm; x++)
    {
        latticeX = lattice[x];
        for (int y = 0; y < Ym; y++)
        {
            latticeY = latticeX[y];
            for (int z = 0; z < Zm; z++)
            {
                snprintf(printBuffer, 11, "%d ", latticeY[z]);
            }
            snprintf(printBuffer, 1, "\n");
        }
        snprintf(printBuffer, 1, "\n");
    }
    FILE *f = fopen(latticeFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
}
void writeCoordinates(int **coord, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char coordinateFileName[128];
    char printBuffer[blocks * 83];
    int *tempCoord;
    snprintf(coordinateFileName, 128, "En-%f/C%.2fL%d-run%d-id-%d-split-%u.txt", bondEn, bondEn, length, rep, runId, (count / split));
    for (int i = 0; i < blocks; i++)
    {
        tempCoord = coord[i];
        snprintf(printBuffer, 83, "%d %d %d %d\n", tempCoord[0], tempCoord[1], tempCoord[2], tempCoord[3]);
    }
    FILE *f = fopen(coordinateFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
}
void writexyz(int ***lattice, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char xyzFileName[128];
    char printBuffer[12 + (Xm * Ym * Zm * 35)];
    float floatX, floatY;
    snprintf(xyzFileName, 128, "En-%f/VMD%.2fL%d-run%d-id-%d-split-%u.xyz", bondEn, bondEn, length, rep, runId, (count / split));
    snprintf(printBuffer, 12, "%d\n\n", blocks * length);
    for (int x = 0; x < Xm; x++)
    {
        floatX = x * 0.2;
        for (int y = 0; y < Ym; y++)
        {
            floatY = y * 0.2;
            for (int z = 0; z < Zm; z++)
            {
                if (lattice[x][y][z])
                    snprintf(printBuffer, 35, "C %.4f %.4f %.4f\n", z * 0.2, floatY, floatX);
            }
        }
    }
    FILE *f = fopen(xyzFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
}
int **allocate2dMatrix(int blocks, int dimension)
{
    int **Matrix;
    Matrix = new int *[blocks];
    for (int i = 0; i < blocks; i++)
    {
        Matrix[i] = new int[dimension];
        for (int j = 0; j < dimension; j++)
        {
            Matrix[i][j] = 0;
        }
    }
    return Matrix;
}

void deallocate2dMatrix(int **coord, int blocks)
{
    for (int i = 0; i < blocks; ++i)
    {
        delete[](coord[i]);
    }
    delete[](coord);
}

int ***allocate3dMatrix(int dimX, int dimY, int dimZ)
{
    int ***matrix = new int **[dimX];
    for (int x = 0; x < dimX; ++x)
    {
        matrix[x] = new int *[dimY];
        for (int y = 0; y < dimY; ++y)
        {
            matrix[x][y] = new int[dimZ];
            for (int z = 0; z < dimZ; z++)
            {
                matrix[x][y][z] = 0;
            }
        }
    }
    return matrix;
}

void deallocate3dMatrix(int ***matrix, int dimX, int dimY)
{
    for (int x = 0; x < dimX; ++x)
    {
        for (int y = 0; y < dimY; ++y)
        {
            delete[](matrix[x][y]);
        }
        delete[](matrix[x]);
    }
    delete[](matrix);
}

int randint(int lower, int upper)
{
    uniform_int_distribution<int> distribution(lower, upper);
    int number = distribution(dre);
    return number;
}
double randDouble()
{
    uniform_real_distribution<double> distribution(0.0, 1.0);
    float r = distribution(dre);
    return r;
}

int pos(int val, int max)
{
    if (val >= max)
        val -= max;
    if (val < 0)
        val += max;
    return val;
}

bool checkSpace(int ***arr, int x, int y, int z, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (arr[x][pos(y + i, Ym)][z] != 0)
            return false;
    }
    return true;
}

void placeBlock(int ***arr, int x, int y, int z, int length, int currentBlock)
{
    for (int i = 0; i < length; i++)
    {
        arr[x][pos(y + i, Ym)][z] = currentBlock;
    }
}

void placeCord(int **cord, int x, int y, int z, int currentBlock)
{
    cord[currentBlock][0] = currentBlock + 1;
    cord[currentBlock][1] = x;
    cord[currentBlock][2] = y;
    cord[currentBlock][3] = z;
}

void initialize(int ***array, int **cord, int blocks, int length)
{
    for (int i = 0; i < blocks; i++)
    {
        int x = randint(0, Xm - 1);
        int y = randint(0, Ym - 1);
        int z = randint(0, Zm - 1);

#ifdef DEBUG
        printf("block: %d, x: %d, y: %d, z: %d\n", i + 1, x, y, z);
#endif

        if (checkSpace(array, x, y, z, length))
        {
            placeCord(cord, x, y, z, i);
            placeBlock(array, x, y, z, length, i + 1);
        }
        else
        {
            while (true)
            {
                int xp = randint(0, Xm - 1);
                int yp = randint(0, Ym - 1);
                int zp = randint(0, Zm - 1);

#ifdef DEBUG
                printf("block (else): %d, x: %d, y: %d, z: %d\n", i + 1, xp, yp, zp);
#endif

                if (checkSpace(array, xp, yp, zp, length))
                {
                    placeCord(cord, xp, yp, zp, i);
                    placeBlock(array, xp, yp, zp, length, i + 1);
                    break;
                }
            }
        }
    }
}

bool moveCheck(int ***array, int **cord, int length, int bid, int xRand, int yRand, int zRand)
{
    int xVal = cord[bid - 1][1];
    int yVal = cord[bid - 1][2];
    int zVal = cord[bid - 1][3];
    for (int i = 0; i < length; i++)
    {
        if (array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][pos(zVal + zRand, Zm)] != 0 && array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][pos(zVal + zRand, Zm)] != bid)
            return false;
    }
    return true;
}

bool energyCheck(int ***array, int **cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand)
{
    int energy1 = 0;
    int energy2 = 0;
    int xVal = cord[bid - 1][1];
    int yVal = cord[bid - 1][2];
    int zVal = cord[bid - 1][3];
    for (int y = 0; y < length; y++)
    {
        for (int z = -1; z <= 1; z += 2)
        {
            if (array[xVal][pos(yVal + y, Ym)][pos(zVal + z, Zm)] != 0 && array[xVal][pos(yVal + y, Ym)][pos(zVal + z, Zm)] != bid)
                energy1++;
            if (array[pos(xVal + xRand, Xm)][pos(yVal + y + yRand, Ym)][pos(zVal + z + zRand, Zm)] != 0 && array[pos(xVal + xRand, Xm)][pos(yVal + y + yRand, Ym)][pos(zVal + z + zRand, Zm)] != bid)
                energy2++;
        }
    }
    for (int y = 0; y < length; y++)
    {
        for (int x = -1; x <= 1; x += 2)
        {
            if (array[pos(xVal + x, Xm)][pos(yVal + y, Ym)][zVal] != 0 && array[pos(xVal + x, Xm)][pos(yVal + y, Ym)][zVal] != bid)
                energy1++;
            if (array[pos(xVal + x + xRand, Xm)][pos(yVal + y + yRand, Ym)][pos(zVal + zRand, Zm)] != 0 && array[pos(xVal + x + xRand, Xm)][pos(yVal + y + yRand, Ym)][pos(zVal + zRand, Zm)] != bid)
                energy2++;
        }
    }
    double r = randDouble();
    return r < exp(bondEn * (energy2 - energy1));
}
void updatePos(int ***array, int **cord, int length, int bid, int xRand, int yRand, int zRand)
{
    int xVal = cord[bid - 1][1];
    int yVal = cord[bid - 1][2];
    int zVal = cord[bid - 1][3];
    cord[bid - 1][1] = pos(xVal + xRand, Xm);
    cord[bid - 1][2] = pos(yVal + yRand, Ym);
    cord[bid - 1][3] = pos(zVal + zRand, Zm);
    for (int i = 0; i < length; i++)
    {
        array[xVal][pos(yVal + i, Ym)][zVal] = 0;
    }
    for (int i = 0; i < length; i++)
    {
        array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][pos(zVal + zRand, Zm)] = bid;
    }
}

tuple<string, string, string, string, string, string> parseParams(int argc, char *argv[])
{
    string bondEn, length, blocks, iterations, split, runId;
    if (argc > 1)
    {
        for (int i = 0; i < argc; ++i)
        {
            string temp(argv[i]);

            if (temp == "--bondEn")
            {
                bondEn = string(argv[i + 1]);
            }

            if (temp == "--length")
            {
                length = string(argv[i + 1]);
            }

            if (temp == "--blocks")
            {
                blocks = string(argv[i + 1]);
            }

            if (temp == "--iterations")
            {
                iterations = string(argv[i + 1]);
            }

            if (temp == "--split")
            {
                split = string(argv[i + 1]);
            }

            if (temp == "--runId")
            {
                runId = string(argv[i + 1]);
            }
        }
    }
    return make_tuple(bondEn, blocks, length, iterations, split, runId);
}
