#include <iostream>
#include <string>
#include <random>
#include <tuple>
#include <chrono>

#define Xm 200
#define Ym 500
#define Zm 200

// lattice and coordinate array
int **allocate2dMatrix(int blocks, int dimension);
void deallocate2dMatrix(int** coord, int blocks);
int ***allocate3dMatrix(int dimX, int dimY, int dimZ);
void deallocate3dMatrix(int*** matrix, int dimX, int dimY);

// random number
int randint(int lower, int upper);
double randDouble();
// Metropolis algorithm functions
int pos(int val, int max);
bool checkSpace(int*** arr,int x, int y, int z, int length);
void placeBlock(int*** arr, int x, int y, int z, int length, int currentBlock);
void placeCord(int** cord, int x, int y, int z, int currentBlock);
void initialize(int*** array, int** cord, int blocks, int length);
bool energyCheck(int*** array, int** cord, double bondEn, int length, int bid, int xRand, int yRand, int zRand);
bool moveCheck(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand);
void updatePos(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand);
// parse params
std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> parseParams(int argc, char* argv[]);
// bondEn, blocks, length, iterations, split, runId
// write data
void writexyz(int*** lattice, double bondEn, int blocks, int length, int runId, long long int count, long long int split, int rep);
void writeLattice(int*** lattice, double bondEn, int length, int runId, long long int count, long long int split, int rep);
void writeCoordinates(int** coord, double bondEn, int blocks, int length, int runId, long long int count, long long int split, int rep);

// std::random_device rd;
// std::mt19937_64 dre(rd());

unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
std::default_random_engine dre(seed);

int main(int argc, char* argv[]) {
    if (argc != 13)
    {
        printf("err: check input parameters!\n");
    }
    // parse input params
    double bondEn = std::stof(std::get<0>(parseParams(argc, argv)));
    int blocks = std::stoi(std::get<1>(parseParams(argc, argv)));
    int length = std::stoi(std::get<2>(parseParams(argc, argv)));
    long long int iterations = std::stoll(std::get<3>(parseParams(argc, argv)));
    long long int split = std::stoll(std::get<4>(parseParams(argc, argv)));
    int runId = std::stoi(std::get<5>(parseParams(argc, argv)));

    // writing id
    int rep_write = 1;

    // make directory
    std::string strEn = std::to_string(bondEn);
    std::system(("mkdir -p En-" + strEn).c_str());

    int dimension = 4;
    // allocate memory and initialize lattice with 0
    int*** lattice = allocate3dMatrix(Xm, Ym, Zm);
    // allocate memmory and initialize coord with 0
    int** coord = allocate2dMatrix(blocks, dimension);
    // create initial state
    initialize(lattice, coord, blocks, length);
    long long int count = 1;
    int moveDistance = 1;
    while (count <= iterations){
        int bid = randint(1, blocks);
        int xRand = randint(-moveDistance, moveDistance);
        int yRand = randint(-moveDistance, moveDistance);
        int zRand = randint(-moveDistance, moveDistance);
        if(moveCheck(lattice, coord, length, bid, xRand, yRand, zRand)){
            if (energyCheck(lattice, coord, bondEn, length, bid, xRand, yRand, zRand)){
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

    return 0;
}
void writeLattice(int*** lattice, double bondEn, int length, int runId, long long int count, long long int split, int rep){
    char latticeFileName[128];
    snprintf(latticeFileName, 128, "En-%f/L%.2fL%d-run%d-id-%d-split-%d.txt", bondEn, bondEn, length, rep, runId, int(count/split));
    FILE *f = fopen(latticeFileName, "w");
    for (int x = 0; x < Xm; x++){
        for (int y = 0; y < Ym; y++){
            for (int z = 0; z < Zm; z++){
                fprintf(f, "%d ", lattice[x][y][z]);
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
void writeCoordinates(int** coord, double bondEn, int blocks, int length, int runId, long long int count, long long int split, int rep){
    char coordinateFileName[128];
    snprintf(coordinateFileName, 128, "En-%f/C%.2fL%d-run%d-id-%d-split-%d.txt", bondEn, bondEn, length, rep, runId, int(count/split));
    FILE *f = fopen(coordinateFileName, "w");
    for (int i = 0; i < blocks; i++){
        fprintf(f, "%d %d %d %d\n", coord[i][0], coord[i][1], coord[i][2], coord[i][3]);
    }
    fclose(f);
}
void writexyz(int*** lattice, double bondEn, int blocks, int length, int runId, long long int count, long long int split, int rep){
    char xyzFileName[128];
    snprintf(xyzFileName, 128, "En-%f/VMD%.2fL%d-run%d-id-%d-split-%d.xyz", bondEn, bondEn, length, rep, runId, int(count/split));
    FILE *f = fopen(xyzFileName, "w");
    fprintf(f, "%d\n\n", blocks*length);
    for(int x = 0; x < Xm; x++){
        for(int y = 0; y < Ym; y++){
            for(int z = 0; z < Zm; z++){
                if (lattice[x][y][z])
                    fprintf(f, "C %.4f %.4f %.4f\n", z*0.2, y*0.2, x*0.2);
            }
        }
    }
    fclose(f);
}
int **allocate2dMatrix(int blocks, int dimension){
    int** Matrix;
    Matrix = new int *[blocks];
    for(int i = 0; i < blocks; i++) {
        Matrix[i] = new int[dimension];
        for (int j = 0; j < dimension; j++) {
            Matrix[i][j] = 0;
        }
    }
    return Matrix;
}

void deallocate2dMatrix(int** coord, int blocks){
    for (int i = 0; i < blocks; ++i) {
        delete[](coord[i]);
    }
    delete[](coord);
}

int ***allocate3dMatrix(int dimX, int dimY, int dimZ) {
    int ***matrix = new int **[dimX];
    for (int x = 0; x < dimX; ++x) {
        matrix[x] = new int *[dimY];
        for (int y = 0; y < dimY; ++y) {
            matrix[x][y] = new int[dimZ];
            for (int z = 0; z < dimZ; z++) {
                matrix[x][y][z] = 0;
            }
        }
    }
    return matrix;
}

void deallocate3dMatrix(int*** matrix, int dimX, int dimY){
    for (int x = 0; x < dimX; ++x) {
        for (int y = 0; y < dimY; ++y) {
            delete[](matrix[x][y]);
        }
        delete[](matrix[x]);
    }
    delete[](matrix);
}

// int randint(int lower, int upper){
//     std::random_device rd;
//     std::mt19937_64 rde(rd());
//     std::uniform_int_distribution<int> distribution(lower, upper);
//     int number = distribution(rde);
//     return number;
// }
// double randDouble(){
//     std::random_device rd;
//     std::mt19937_64 rde(rd());
//     std::uniform_real_distribution<double > distribution(0.0,1.0);
//     double r = distribution(rde);
//     return r;
// }

int randint(int lower, int upper){
    std::uniform_int_distribution<int> distribution(lower, upper);
    int number = distribution(dre);
    return number;
}
double randDouble(){
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    float r = distribution(dre);
    return r;
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

bool moveCheck(int*** array, int** cord, int length, int bid, int xRand, int yRand, int zRand){
    int xVal = cord[bid-1][1];
    int yVal = cord[bid-1][2];
    int zVal = cord[bid-1][3];
    for(int i = 0; i < length; i++){
        if(array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][pos(zVal + zRand, Zm)] != 0 && array[pos(xVal + xRand, Xm)][pos(yVal + yRand + i, Ym)][pos(zVal + zRand, Zm)] != bid)
            return false;
    }
    return true;
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
            if(array[pos(xVal+xRand, Xm)][pos(yVal+y+yRand, Ym)][pos(zVal+z+zRand, Zm)] != 0 && array[pos(xVal+xRand, Xm)][pos(yVal+y+yRand, Ym)][pos(zVal+z+zRand, Zm)] != bid)
                energy2++;
        }
    }
    for(int y = 0; y < length; y++){
        for(int x = -1; x <= 1; x+=2){
            if(array[pos(xVal+x, Xm)][pos(yVal+y, Ym)][zVal] != 0 && array[pos(xVal+x, Xm)][pos(yVal+y, Ym)][zVal] != bid)
                energy1++;
            if(array[pos(xVal+x+xRand, Xm)][pos(yVal+y+yRand, Ym)][pos(zVal+zRand, Zm)] != 0 && array[pos(xVal+x+xRand, Xm)][pos(yVal+y+yRand, Ym)][pos(zVal+zRand, Zm)] != bid)
                energy2++;
        }
    }
    double r = randDouble();
    return r < exp(bondEn*(energy2 - energy1));
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

std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> parseParams(int argc, char* argv[]){
    std::string bondEn, length, blocks, iterations, split, runId;
    if(argc > 1){
        for (int i = 0; i < argc; ++i)
        {
            std::string temp(argv[i]);

            if(temp == "--bondEn") bondEn = std::string(argv[i+1]);
            // else bondEn = "0.8";

            if(temp == "--length") length = std::string(argv[i+1]);
            // else length = "5";

            if(temp == "--blocks") blocks = std::string(argv[i+1]);
            // else blocks = "20000";

            if(temp == "--iterations") iterations = std::string(argv[i+1]);
            // else iterations = "1000000000";

            if(temp == "--split") split = std::string(argv[i+1]);
            // else split = "1000000000";           

            if(temp == "--runId") runId = std::string(argv[i+1]);
            // else runId = "1";
        }

    }
    return std::make_tuple(bondEn, blocks, length, iterations, split, runId);
}
