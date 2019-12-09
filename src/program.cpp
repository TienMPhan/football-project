#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <tuple>
#include "math_c.h"
#include "time_c.h"
#include "memory_c.h"
#include "omp.h"

using namespace std;

char *outputDir;
int outputFileNameLength;
int Xm, Ym, Zm;
int nthreads;
int XmChunkSize, blocksChunkSize;
int *latticeThreadStart, *latticeThreadEnd,
    *coordinatesThreadStart, *coordinatesThreadEnd;
int latticeBufferSize, latticeChunkBufferSize,
    xyzBufferSize, xyzChunkBufferSize,
    coordinatesBufferSize, coordinatesChunkBufferSize;

// lattice and coordinate array
int **allocate2dMatrix(int blocks, int dimension)
{
    int **matrix, *tempMatrix;
    matrix = new int *[blocks];
#pragma omp parallel
    {
        int threadId = omp_get_thread_num(), start = coordinatesThreadStart[threadId], end = coordinatesThreadEnd[threadId];
        // int **tempMatrix = new int[];
        for (int i = 0; i < blocks; i++)
        {
            tempMatrix = new int[dimension];
            for (int j = 0; j < dimension; j++)
            {
                tempMatrix[j] = 0;
            }
            matrix[i] = tempMatrix;
        }
    }
    return matrix;
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
void deallocate2dMatrix(int **coord, int blocks)
{
    for (int i = 0; i < blocks; i++)
    {
        delete[](coord[i]);
    }
    delete[](coord);
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

// seeding random generator
default_random_engine dre(chrono::steady_clock::now().time_since_epoch().count());

// random number
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

// Metropolis algorithm functions
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

// parse params
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

// write data
void writeLattice(int ***lattice, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char *latticePrintBuffer = new char[latticeBufferSize], *latticeBufferPtr = latticePrintBuffer;
    char *xyzPrintBuffer = new char[xyzBufferSize], *xyzBufferPtr = xyzPrintBuffer;
    xyzBufferPtr += snprintf(xyzBufferPtr, 12, "%d\n\n", blocks * length);
#pragma omp parallel for ordered schedule(static, 1)
    for (int t = 0; t < nthreads; t++)
    {
        int threadId = omp_get_thread_num(), start = latticeThreadStart[threadId], end = latticeThreadEnd[threadId];
        char *latticeThreadPrintBuffer = new char[latticeChunkBufferSize], *latticeThreadBufferPtr = latticeThreadPrintBuffer,
             *xyzThreadPrintBuffer = new char[xyzChunkBufferSize], *xyzThreadBufferPtr = xyzThreadPrintBuffer;
        int **latticeX, *latticeY, latticeZ;
        float floatX, floatY;
        for (int x = start; x < end; x++)
        {
            latticeX = lattice[x];
            floatX = x * 0.2;
            for (int y = 0; y < Ym; y++)
            {
                latticeY = latticeX[y];
                floatY = y * 0.2;
                for (int z = 0; z < Zm; z++)
                {
                    latticeZ = latticeY[z];
                    latticeThreadBufferPtr += snprintf(latticeThreadBufferPtr, 10, "%d ", latticeZ);
                    if (latticeZ)
                    {
                        xyzThreadBufferPtr += snprintf(xyzThreadBufferPtr, 35, "C %.4f %.4f %.4f\n", z * 0.2, floatY, floatX);
                    }
                }
                latticeThreadBufferPtr += snprintf(latticeThreadBufferPtr, 2, "%s", "\n");
            }
            latticeThreadBufferPtr += snprintf(latticeThreadBufferPtr, 2, "%s", "\n");
        }
        int latticeTotalCharsPrinted = latticeThreadBufferPtr - latticeThreadPrintBuffer;
        int xyzTotalCharsPrinted = xyzThreadBufferPtr - xyzThreadPrintBuffer;
#pragma omp ordered
        {
            latticeBufferPtr += snprintf(latticeBufferPtr, latticeTotalCharsPrinted, "%s", latticeThreadPrintBuffer);
            xyzBufferPtr += snprintf(xyzBufferPtr, xyzTotalCharsPrinted, "%s", xyzThreadPrintBuffer);
        }
        delete[](latticeThreadPrintBuffer);
        delete[](xyzThreadPrintBuffer);
    }
    char fileName[outputFileNameLength];
    unsigned long countDivSplit = count / split;
    snprintf(fileName, outputFileNameLength, "%s/L%.2fL%d-run%d-id-%d-split-%d.txt", outputDir, bondEn, length, rep, runId, countDivSplit);
    FILE *f = fopen(fileName, "w");
    fprintf(f, latticePrintBuffer);
    fclose(f);
    snprintf(fileName, outputFileNameLength, "%s/VMD%.2fL%d-run%d-id-%d-split-%d.xyz", outputDir, bondEn, length, rep, runId, countDivSplit);
    f = fopen(fileName, "w");
    fprintf(f, xyzPrintBuffer);
    fclose(f);
    delete[](latticePrintBuffer);
    delete[](xyzPrintBuffer);
}
void writeCoordinates(int **coord, double bondEn, int blocks, int length, int runId, unsigned long count, unsigned long split, int rep)
{
    char *printBuffer = new char[coordinatesBufferSize], *bufferPtr = printBuffer;
#pragma omp parallel for ordered schedule(static, 1)
    for (int t = 0; t < nthreads; t++)
    {
        int threadId = omp_get_thread_num(), start = coordinatesThreadStart[threadId], end = coordinatesThreadEnd[threadId];
        char *threadPrintBuffer = new char[coordinatesChunkBufferSize], *threadBufferPtr = threadPrintBuffer;
        int *tempCoord;
        for (int i = start; i < end; i++)
        {
            tempCoord = coord[i];
            threadBufferPtr += snprintf(threadBufferPtr, 83, "%d %d %d %d\n", tempCoord[0], tempCoord[1], tempCoord[2], tempCoord[3]);
        }
        int totalCharsPrinted = threadBufferPtr - threadPrintBuffer;
#pragma omp ordered
        {
            bufferPtr += snprintf(bufferPtr, totalCharsPrinted, "%s", threadPrintBuffer);
        }
        delete[](threadPrintBuffer);
    }
    char coordinateFileName[outputFileNameLength];
    snprintf(coordinateFileName, outputFileNameLength, "%s/C%.2fL%d-run%d-id-%d-split-%d.txt", outputDir, bondEn, length, rep, runId, (count / split));
    FILE *f = fopen(coordinateFileName, "w");
    fprintf(f, printBuffer);
    fclose(f);
    delete[](printBuffer);
}

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

    // environment setup (1/2)
    outputDir = getenv("JOB_OUTPUT_DIR");
    outputFileNameLength = strlen(outputDir) + 128;
    Xm = atoi(getenv("XM"));
    Ym = atoi(getenv("YM"));
    Zm = atoi(getenv("ZM"));
    nthreads = atoi(getenv("CORE_COUNT_PER_NODE"));
    int rep_write = atoi(getenv("WRITE_ID"));
    int dimension = atoi(getenv("DIMENSIONS"));

    // parse input params
    tuple<string, string, string, string, string, string> parsedParamsTuple = parseParams(argc, argv);
    double bondEn = stof(get<0>(parsedParamsTuple));
    int blocks = stoi(get<1>(parsedParamsTuple));
    int length = stoi(get<2>(parsedParamsTuple));
    unsigned long iterations = stoul(get<3>(parsedParamsTuple));
    unsigned long split = stoul(get<4>(parsedParamsTuple));
    int runId = stoi(get<5>(parsedParamsTuple));

    // environment setup (2/2)
    omp_set_num_threads(nthreads);
    XmChunkSize = (Xm / nthreads) + (Xm % nthreads);
    blocksChunkSize = (blocks / nthreads) + (blocks % nthreads);

    latticeBufferSize = Xm * Ym * Zm * 14;
    latticeChunkBufferSize = XmChunkSize * Ym * Zm * 14;
    xyzBufferSize = 12 + (Xm * Ym * Zm * 35);
    xyzChunkBufferSize = XmChunkSize * Ym * Zm * 35;
    coordinatesBufferSize = blocks * 83;
    coordinatesChunkBufferSize = blocksChunkSize * 83;

    latticeThreadStart = new int[nthreads];
    latticeThreadEnd = new int[nthreads];
    coordinatesThreadStart = new int[nthreads];
    coordinatesThreadEnd = new int[nthreads];
    for (int threadId = 0,
             latticeThreadStartNum, coordinatesThreadStartNum;
         threadId < nthreads; threadId++)
    {
        latticeThreadStartNum = threadId * XmChunkSize;
        latticeThreadStart[threadId] = latticeThreadStartNum;
        latticeThreadEnd[threadId] = min(latticeThreadStartNum + XmChunkSize, Xm);
        coordinatesThreadStartNum = threadId * blocksChunkSize;
        coordinatesThreadStart[threadId] = coordinatesThreadStartNum;
        coordinatesThreadEnd[threadId] = min(coordinatesThreadStartNum + blocksChunkSize, blocks);
    }

    // allocate memory and initialize lattice with 0
    int ***lattice = allocate3dMatrix(Xm, Ym, Zm);

    // allocate memmory and initialize coord with 0
    int **coord = allocate2dMatrix(blocks, dimension);

    // create initial state
    initialize(lattice, coord, blocks, length);
    for (unsigned long count = 1; count <= iterations; count++)
    {
        int bid = randInt(1, blocks);
        int xRand = randInt(-1, 1);
        int yRand = randInt(-1, 1);
        int zRand = randInt(-1, 1);
        if (moveCheck(lattice, coord, length, bid, xRand, yRand, zRand))
        {
            if (energyCheck(lattice, coord, bondEn, length, bid, xRand, yRand, zRand))
            {
                updatePos(lattice, coord, length, bid, xRand, yRand, zRand);
                if (count % split == 0)
                {
                    writeLattice(lattice, bondEn, blocks, length, runId, count, split, rep_write);
                    writeCoordinates(coord, bondEn, blocks, length, runId, count, split, rep_write);
                }
                count++;
            }
        }
    }

#ifdef DEBUG
    gettimeofday(&t2, NULL);
    elapsedTime = getTimeDifferenceInMilliseconds(&t1, &t2);
    getProcessMemory(&mem);

    printf("\nEn-%f/C%.2fL%d-run%d-id-%d\n\tCore Count Per Node: %d\n\tElapsed Time (ms): %f\n\tPhysical Memory (kB): %u\n\tVirtual Memory (kB): %u\n",
           nthreads, bondEn, bondEn, length, rep_write, runId, elapsedTime, mem.physicalMem, mem.virtualMem);
#endif

    deallocate2dMatrix(coord, blocks);
    deallocate3dMatrix(lattice, Xm, Ym);

    delete[](latticeThreadStart);
    delete[](latticeThreadEnd);
    delete[](coordinatesThreadStart);
    delete[](coordinatesThreadEnd);

    return 0;
}
