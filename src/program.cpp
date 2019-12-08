#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <tuple>
#include "time_c.h"
#include "memory_c.h"

using namespace std;

char *outputDir;
int outputFileNameLength;
int Xm, Ym, Zm;

// lattice and coordinate array
int **allocate2dMatrix(int blocks, int dimension);
void deallocate2dMatrix(int **coord, int blocks);
int ***allocate3dMatrix(int dimX, int dimY, int dimZ);
void deallocate3dMatrix(int ***matrix, int dimX, int dimY);

// random number
int randInt(int lower, int upper);
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

// seeding random generator
default_random_engine dre(chrono::steady_clock::now().time_since_epoch().count());

int main(int argc, char *argv[])
{

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

    // environment setup
    outputDir = getenv("JOB_OUTPUT_DIR");
    outputFileNameLength = strlen(outputDir) + 128;
    int rep_write = atoi(getenv("WRITE_ID"));
    int dimension = atoi(getenv("DIMENSIONS"));
    Xm = atoi(getenv("XM"));
    Ym = atoi(getenv("YM"));
    Zm = atoi(getenv("ZM"));

    // parse input params
    tuple<string, string, string, string, string, string> parsedParamsTuple = parseParams(argc, argv);
    double bondEn = stof(get<0>(parsedParamsTuple));
    int blocks = stoi(get<1>(parsedParamsTuple));
    int length = stoi(get<2>(parsedParamsTuple));
    unsigned long iterations = stoul(get<3>(parsedParamsTuple));
    unsigned long split = stoul(get<4>(parsedParamsTuple));
    int runId = stoi(get<5>(parsedParamsTuple));

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
        int bid = randInt(1, blocks);
        int xRand = randInt(-moveDistance, moveDistance);
        int yRand = randInt(-moveDistance, moveDistance);
        int zRand = randInt(-moveDistance, moveDistance);
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
    char *printBuffer = new char[Xm * Ym * Zm * 14], *bufferPtr = printBuffer;
    int **latticeX, *latticeY;
    for (int x = 0; x < Xm; x++)
    {
        latticeX = lattice[x];
        for (int y = 0; y < Ym; y++)
        {
            latticeY = latticeX[y];
            for (int z = 0; z < Zm; z++)
            {
                bufferPtr += snprintf(bufferPtr, 10, "%d ", latticeY[z]);
            }
            bufferPtr += snprintf(bufferPtr, 2, "%s", "\n");
        }
        bufferPtr += snprintf(bufferPtr, 2, "%s", "\n");
    }
    char latticeFileName[outputFileNameLength];
    snprintf(latticeFileName, outputFileNameLength, "%s/L%.2fL%d-run%d-id-%d-split-%d.txt", outputDir, bondEn, length, rep, runId, (count / split));
    FILE *f = fopen(latticeFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
    delete[](printBuffer);
}
void writeCoordinates(int **coord, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char *printBuffer = new char[blocks * 83], *bufferPtr = printBuffer;
    int *tempCoord;
    for (int i = 0; i < blocks; i++)
    {
        tempCoord = coord[i];
        bufferPtr += snprintf(bufferPtr, 83, "%d %d %d %d\n", tempCoord[0], tempCoord[1], tempCoord[2], tempCoord[3]);
    }
    char coordinateFileName[outputFileNameLength];
    snprintf(coordinateFileName, outputFileNameLength, "%s/C%.2fL%d-run%d-id-%d-split-%d.txt", outputDir, bondEn, length, rep, runId, (count / split));
    FILE *f = fopen(coordinateFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
    delete[](printBuffer);
}
void writexyz(int ***lattice, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char *printBuffer = new char[12 + (Xm * Ym * Zm * 35)], *bufferPtr = printBuffer;
    bufferPtr += snprintf(bufferPtr, 12, "%d\n\n", blocks * length);
    float floatX, floatY;
    int **latticeX, *latticeY;
    for (int x = 0; x < Xm; x++)
    {
        latticeX = lattice[x];
        floatX = x * 0.2;
        for (int y = 0; y < Ym; y++)
        {
            latticeY = latticeX[y];
            floatY = y * 0.2;
            for (int z = 0; z < Zm; z++)
            {
                if (latticeY[z])
                {
                    bufferPtr += snprintf(bufferPtr, 35, "C %.4f %.4f %.4f\n", z * 0.2, floatY, floatX);
                }
            }
        }
    }
    char xyzFileName[outputFileNameLength];
    snprintf(xyzFileName, outputFileNameLength, "%s/VMD%.2fL%d-run%d-id-%d-split-%d.xyz", outputDir, bondEn, length, rep, runId, (count / split));
    FILE *f = fopen(xyzFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
    delete[](printBuffer);
}
int **allocate2dMatrix(int blocks, int dimension)
{
    int **matrix, *tempMatrix;
    matrix = new int *[blocks];
    for (int i = 0; i < blocks; i++)
    {
        tempMatrix = new int[dimension];
        for (int j = 0; j < dimension; j++)
        {
            tempMatrix[j] = 0;
        }
        matrix[i] = tempMatrix;
    }
    return matrix;
}

void deallocate2dMatrix(int **coord, int blocks)
{
    for (int i = 0; i < blocks; i++)
    {
        delete[](coord[i]);
    }
    delete[](coord);
}

int ***allocate3dMatrix(int dimX, int dimY, int dimZ)
{
    int ***matrix, **matrixX, *matrixY;
    matrix = new int **[dimX];
    for (int x = 0; x < dimX; x++)
    {
        matrixX = new int *[dimY];
        for (int y = 0; y < dimY; y++)
        {
            matrixY = new int[dimZ];
            for (int z = 0; z < dimZ; z++)
            {
                matrixY[z] = 0;
            }
            matrixX[y] = matrixY;
        }
        matrix[x] = matrixX;
    }
    return matrix;
}

void deallocate3dMatrix(int ***matrix, int dimX, int dimY)
{
    int **matrixX;
    for (int x = 0; x < dimX; x++)
    {
        matrixX = matrix[x];
        for (int y = 0; y < dimY; y++)
        {
            delete[](matrixX[y]);
        }
        delete[](matrixX);
    }
    delete[](matrix);
}

int randInt(int lower, int upper)
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
    {
        val -= max;
    }
    if (val < 0)
    {
        val += max;
    }
    return val;
}

bool checkSpace(int ***arr, int x, int y, int z, int length)
{
    int **arrX = arr[x];
    for (int i = 0; i < length; i++)
    {
        if (arrX[pos(y + i, Ym)][z] != 0)
        {
            return false;
        }
    }
    return true;
}

void placeBlock(int ***arr, int x, int y, int z, int length, int currentBlock)
{
    int **arrX = arr[x];
    for (int i = 0; i < length; i++)
    {
        arrX[pos(y + i, Ym)][z] = currentBlock;
    }
}

void placeCord(int **cord, int x, int y, int z, int currentBlock)
{
    int *tempCord = cord[currentBlock];
    tempCord[0] = currentBlock + 1;
    tempCord[1] = x;
    tempCord[2] = y;
    tempCord[3] = z;
}

void initialize(int ***array, int **cord, int blocks, int length)
{
    for (int i = 0; i < blocks; i++)
    {
        int x = randInt(0, Xm - 1);
        int y = randInt(0, Ym - 1);
        int z = randInt(0, Zm - 1);

#ifdef DEBUG
        // printf("block: %d, x: %d, y: %d, z: %d\n", i + 1, x, y, z);
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
                int xp = randInt(0, Xm - 1);
                int yp = randInt(0, Ym - 1);
                int zp = randInt(0, Zm - 1);

#ifdef DEBUG
                // printf("block (else): %d, x: %d, y: %d, z: %d\n", i + 1, xp, yp, zp);
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
    int *tempCord = cord[bid - 1];
    int xVal = tempCord[1];
    int yVal = tempCord[2];
    int zVal = tempCord[3];
    int arrVal,
        **tempArrXPosAddRand = array[pos(xVal + xRand, Xm)];
    int posZAddRand = pos(zVal + zRand, Zm);
    for (int i = 0; i < length; i++)
    {
        arrVal = tempArrXPosAddRand[pos(yVal + yRand + i, Ym)][posZAddRand];
        if (arrVal != 0 && arrVal != bid)
        {
            return false;
        }
    }
    return true;
}

bool energyCheck(int ***array, int **cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand)
{
    int energy1 = 0;
    int energy2 = 0;
    int *tempCord = cord[bid - 1];
    int xVal = tempCord[1];
    int yVal = tempCord[2];
    int zVal = tempCord[3];
    int arrVal,
        **tempArrX = array[xVal],
        **tempArrXPosAddRand = array[pos(xVal + xRand, Xm)],
        **tempArrXPosSubt1 = array[pos(xVal - 1, Xm)],
        **tempArrXPosSubt1AddRand = array[pos(xVal - 1 + xRand, Xm)],
        **tempArrXPosAdd1 = array[pos(xVal + 1, Xm)],
        **tempArrXPosAdd1AddRand = array[pos(xVal + 1 + xRand, Xm)];
    int posZSubt1 = pos(zVal - 1, Zm),
        posZAdd1AddRand = pos(zVal + 1 + zRand, Zm),
        posZAddRand = pos(zVal + zRand, Zm),
        posYAddY, posYAddYAddRand;
    for (int y = 0; y < length; y++)
    {
        posYAddY = pos(yVal + y, Ym);
        posYAddYAddRand = pos(yVal + y + yRand, Ym);
        arrVal = tempArrX[posYAddY][posZSubt1];
        if (arrVal != 0 && arrVal != bid)
        {
            energy1++;
        }
        arrVal = tempArrXPosAddRand[posYAddYAddRand][posZAdd1AddRand];
        if (arrVal != 0 && arrVal != bid)
        {
            energy2++;
        }
        arrVal = tempArrXPosSubt1[posYAddY][zVal];
        if (arrVal != 0 && arrVal != bid)
        {
            energy1++;
        }
        arrVal = tempArrXPosSubt1AddRand[posYAddYAddRand][posZAddRand];
        if (arrVal != 0 && arrVal != bid)
        {
            energy2++;
        }

        arrVal = tempArrXPosAdd1[posYAddY][zVal];
        if (arrVal != 0 && arrVal != bid)
        {
            energy1++;
        }
        arrVal = tempArrXPosAdd1AddRand[posYAddYAddRand][posZAddRand];
        if (arrVal != 0 && arrVal != bid)
        {
            energy2++;
        }
    }
    double r = randDouble();
    return r < exp(bondEn * (energy2 - energy1));
}
void updatePos(int ***array, int **cord, int length, int bid, int xRand, int yRand, int zRand)
{
    int *tempCord = cord[bid - 1];
    int xVal = tempCord[1];
    int yVal = tempCord[2];
    int zVal = tempCord[3];
    int posXAddRand = pos(xVal + xRand, Xm),
        posZAddRand = pos(zVal + zRand, Zm);
    tempCord[1] = posXAddRand;
    tempCord[2] = pos(yVal + yRand, Ym);
    tempCord[3] = posZAddRand;
    int **tempArrX = array[xVal],
        **tempArrXPosAddRand = array[posXAddRand];
    int yAddi;
    for (int i = 0; i < length; i++)
    {
        yAddi = yVal + i;
        tempArrX[pos(yAddi, Ym)][zVal] = 0;
        tempArrXPosAddRand[pos(yAddi + yRand, Ym)][posZAddRand] = bid;
    }
}

tuple<string, string, string, string, string, string> parseParams(int argc, char *argv[])
{
    string bondEn, length, blocks, iterations, split, runId;
    string temp;
    for (int i = 0; i < argc; i++)
    {
        temp = string(argv[i]);

        if (temp == "--bondEn")
        {
            bondEn = string(argv[i + 1]);
        }
        else if (temp == "--length")
        {
            length = string(argv[i + 1]);
        }
        else if (temp == "--blocks")
        {
            blocks = string(argv[i + 1]);
        }
        else if (temp == "--iterations")
        {
            iterations = string(argv[i + 1]);
        }
        else if (temp == "--split")
        {
            split = string(argv[i + 1]);
        }
        else if (temp == "--runId")
        {
            runId = string(argv[i + 1]);
        }
    }
    return make_tuple(bondEn, blocks, length, iterations, split, runId);
}
