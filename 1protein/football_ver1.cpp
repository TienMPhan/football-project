#include <iostream>
#include <random>

#define Xm 40
#define Ym 40
#define Zm 40

int ***allocate3dMatrix(int dim_z, int dim_x, int dim_y);
int **allocate2dMatrix(int blocks, int dimension);
void printArray(int ***arr, int xm, int ym, int zm);
void printCord(int **matrix, int blocks, int dimension);
int pos(int val, int max);
bool checkSpace(int*** arr,int x, int y, int z, int length);
int randint(int lower, int upper);
double randDouble();
void placeBlock(int*** arr, int x, int y, int z, int length, int currentBlock);
void placeCord(int** cord, int x, int y, int z, int currentBlock);
void initialize(int*** array, int** cord, int blocks, int length);
bool energyCheck(int*** array, int** cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand);
bool moveCheck(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand);
void updatePos(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand);
int main() {
    // control parameters
    int blocks = 2000;
    int length = 5;
    double bondEn = 0.8;
    int dimension = 4;
    // prepare lattice
    int **cord = allocate2dMatrix(blocks, dimension);
    int*** array = allocate3dMatrix(Zm, Xm, Ym);
    // initialize arr
    for (int z = 0; z < Zm; ++z) {
        for (int y = 0; y < Ym; ++y) {
            for (int x = 0; x < Xm; ++x) {
                array[x][y][z] = 0;
            }
        }
    }
    // initialize cord
    for (int block = 0; block < blocks; block++){
        for(int dim = 0; dim < dimension; dim++){
            cord[block][dim] = 0;
        }
    }
    // initialize random system
    initialize(array, cord, blocks, length);
    // printArray(array, Xm, Ym, Zm);
    // printCord(cord, blocks, dimension);

    int long iterations = 10000000;
    int long counter = 1;
    const int split = int (1*pow(10, 6));
    int runId = 1;
    int moveDistance = 4;
    while(counter <= iterations){
        int bid = randint(1, blocks);
        int xRand = randint(-moveDistance, moveDistance);
        int yRand = randint(-moveDistance, moveDistance);
        int zRand = randint(-moveDistance, moveDistance);
        if (moveCheck(array, cord, length, bid, xRand, yRand, zRand)){
            if (energyCheck(array, cord, bondEn, length, bid, xRand, yRand, zRand)){
                updatePos(array, cord, length, bid, xRand, yRand, zRand);

                if(counter % split == 0){
                    printf("completed %li...\n", counter);
                    char fileName[80];
                    snprintf(fileName, 80, "M-%.2f-%d-%d.txt", bondEn, runId, int(counter/split));
                    FILE *file = fopen(fileName, "w");
                    for (int x = 0; x < Xm; x++){
                        for (int y = 0; y < Ym; y++){
                            for (int z = 0; z < Zm; z++){
                                if (array[x][y][z])
                                    fprintf(file, "1 ");
                                else fprintf(file, "0 ");
                            }
                            fprintf(file, "\n");
                        }
                        fprintf(file, "\n");
                    }
                    fclose(file);
                }
                counter++;
            }
        }

    }


    return 0;
}

int randint(int lower, int upper){
    std::random_device rd;
    std::mt19937_64 rde(rd());
    std::uniform_int_distribution<int> distribution(lower, upper);
    int number = distribution(rde);
    return number;
}

double randDouble(){
    std::random_device rd;
    std::mt19937_64 rde(rd());
    std::uniform_real_distribution<double > distribution(0.0,1.0);
    double r = distribution(rde);
    return r;
}

int **allocate2dMatrix(int blocks, int dimension){
    int** Matrix;
    Matrix = new int *[blocks];
    for(int i = 0; i < blocks; i++)
        Matrix[i] = new int[dimension];
    return Matrix;
}

int ***allocate3dMatrix(int dim_z, int dim_x, int dim_y) {
    int ***Matrix;
    int *ptr;
    int k, i;
    Matrix = (int ***)malloc(dim_z * sizeof(int **));
    for (k = 0; k < dim_z; k++) {
        // Allocate 64-byte aligned memory for each x-y plane
        ptr = (int *)malloc(dim_x * dim_y * sizeof(int));
        // Allocate pointer array for each row
        Matrix[k] = (int **)malloc(dim_x * sizeof(int *));
        for (i = 0; i < dim_x; i++) {
            Matrix[k][i] = &ptr[i * dim_y];
        }
    }
    return Matrix;
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

void printCord(int **matrix, int blocks, int dimension)
{
    for (int i = 0; i < blocks; i++)
    {
        for (int j = 0; j < dimension; j++)
            printf("%d ", matrix[i][j]);
        printf("\n");
    }
}

int pos(int val, int max){
    if(val >= max) val -= max;
    if(val < 0) val += max;
    return val;
}

bool checkSpace(int*** arr, int x, int y, int z, int length){
    for(int i = 0; i < length; i++){
        if(arr[x][pos(y + i, Ym)][z] != 0) return false;
    }
    return true;
}

void placeBlock(int*** arr, int x, int y, int z, int length, int currentBlock){
    for (int i = 0; i < length; i++){
        arr[x][pos(y + i, Ym)][z] = currentBlock;
    }
}

void placeCord(int** cord, int x, int y, int z, int currentBlock){
    cord[currentBlock][0] = currentBlock+1;
    cord[currentBlock][1] = x;
    cord[currentBlock][2] = y;
    cord[currentBlock][3] = z;
}

void initialize(int*** array, int** cord, int blocks, int length){
    for(int i = 0; i < blocks; i++){
        int x = randint(0, Xm-1);
        int y = randint(0, Ym-1);
        int z = randint(0, Zm-1);
        // printf("block: %d, x: %d, y: %d, z: %d\n", i+1, x, y, z);
        if (checkSpace(array, x, y, z, length)) {
            placeCord(cord, x, y, z, i);
            placeBlock(array, x, y, z, length, i+1);
        }else{
            while (true){
                int xp = randint(0, Xm-1);
                int yp = randint(0, Ym-1);
                int zp = randint(0, Zm-1);
                // printf("block (else): %d, x: %d, y: %d, z: %d\n", i+1, xp, yp, zp);
                if (checkSpace(array, xp, yp, zp, length)){
                    placeCord(cord, xp, yp, zp, i);
                    placeBlock(array, xp, yp, zp, length, i+1);
                    break;
                }

            }
        }
    }
}

bool energyCheck(int*** array, int** cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand){
    int energy1 = 0;
    int energy2 = 0;
    int xVal = cord[bid-1][1];
    int yVal = cord[bid-1][2];
    int zVal = cord[bid-1][3];
    for(int y = 0; y < length; y++){
        for(int z = -1; z <= 1; z+=2){
            if(array[xVal][pos(yVal+y, Ym)][pos(zVal + z, Zm)] != 0 && array[xVal][pos(yVal+y, Ym)][pos(zVal + z, Zm)] != bid)
                energy1++;
            if(array[xVal][pos(yVal+y+yRand, Ym)][pos(zVal+z+zRand, Zm)] != 0 && array[xVal][pos(yVal+y+yRand, Ym)][pos(zVal+z+zRand, Zm)] != bid)
                energy2++;
        }
    }
    for(int y = 0; y < length; y++){
        for(int x = -1; x <= 1; x+=2){
            if(array[pos(xVal+x, Xm)][pos(yVal+y, Ym)][zVal] != 0 && array[pos(xVal+x, Xm)][pos(yVal+y, Ym)][zVal] != bid)
                energy1++;
            if(array[pos(xVal+x+xRand, Xm)][pos(yVal+y+yRand, Ym)][zVal] != 0 && array[pos(xVal+x+xRand, Xm)][pos(yVal+y+yRand, Ym)][zVal] != bid)
                energy2++;
        }
    }
    double dE = energy2 - energy1;
    // printf("E1: %d, E2: %d, dE: %.1f\n", energy1, energy2, dE);
    if(dE >= 0) return true;
    double boltzmannWeight = exp(dE*bondEn);
    double r = randDouble();
    // printf("r: %f, Boltzmann weight: %f\n", r, boltzmannWeight);
    return r < boltzmannWeight;
}

bool moveCheck(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand){
    int xVal = cord[bid-1][1];
    int yVal = cord[bid-1][2];
    int zVal = cord[bid-1][3];
    for(int i = 0; i < length; i++){
        if(array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][zVal + zRand] != 0 && array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][zVal + zRand] != bid)
            return false;
    }
    return true;
}

void updatePos(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand){
    int xVal = cord[bid-1][1];
    int yVal = cord[bid-1][2];
    int zVal = cord[bid-1][3];
    cord[bid-1][1] = pos(xVal + xRand, Xm);
    cord[bid-1][2] = pos(yVal + yRand, Ym);
    cord[bid-1][3] = pos(zVal + zRand, Zm);
    for (int i = 0; i < length; i++){
        array[xVal][pos(yVal+i, Ym)][zVal] = 0;
    }
    for(int i = 0; i < length; i++){
        array[pos(xVal+xRand, Xm)][pos(yVal+yRand+i, Ym)][pos(zVal+zRand, Zm)] = bid;
    }
}