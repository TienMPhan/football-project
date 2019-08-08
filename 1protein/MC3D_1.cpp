#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <random>

#define Xmax 200
#define Ymax 200
#define Zmax 200

using namespace std;


//functions
bool startPlacement(int length);
bool checkSpace(int X, int Y, int Z, int length);
void placeBlock(int X,int Y, int Z, int currentBlock, int length);

bool energyCheck(int bid, int xrand, int yrand, int zrand, float energyConstant, int length);

int randomBlock();
int randint(int lower, int upper); // (inclusive, inclusive)
float randfloat();

int xpos(int X);
int ypos(int Y);
int zpos(int Z);

void updatepos(int bid, int X, int Y, int Z);
bool moveCheck(int bid, int xrand, int yrand, int zrand, int length);
void moveBlock(int bid, int xrand, int yrand, int zrand, int length);

int moveDistance = 4;
long long int counter = 1;
// long long int iderations = 2000000000;

const int blocks = 20000;
const int dimension = 4;
// const float energyConstant = .8;
// bool firstTimeOutputted = true;
// int savedFileNumber;
// int setseed = false;
int cord[blocks][dimension];
int array[Xmax][Ymax][Zmax];
int visibleArray [Xmax][Ymax];

// int seed = time(0);
// std::default_random_engine dre (seed);
std::random_device rd;
std::mt19937_64 dre(rd());


int main(int argc, char* argv[]){

	if (argc != 5)
        printf("Err: %s needs input paramenter!\n", argv[0]);

    float energyConstant = atof(argv[1]);
    int length = stoi(argv[2]);
    long long int iderations = atoll(argv[3]);
    int id = atoi(argv[4]);

	if(startPlacement(length) == false){
		cout<<"There was not enough space to place all the proteins"<<endl;
		return 0;
	}

	int fails = 0;
	int xrand;
	int yrand;
	int zrand;

	while(counter <= iderations){
		int bid = randint(1, blocks);
		xrand = randint(-(2*moveDistance), (2*moveDistance));
		yrand = randint(-moveDistance, moveDistance);
		zrand = randint(-moveDistance, moveDistance);
		if(moveCheck(bid, xrand, yrand, zrand, length) == true){
			if(energyCheck(bid, xrand, yrand, zrand, energyConstant, length) == true){
				moveBlock(bid, xrand, yrand, zrand, length);
				updatepos(bid, xrand, yrand, zrand);

				if(counter % iderations == 0){
					char fileName[80];
					snprintf(fileName, 80, "Arr-%.2f-%d.txt", energyConstant, id);
					FILE *file = fopen(fileName, "w");
					// fwrite(array, sizeof(array), 1, file);
					for(int i = 0; i < Zmax; i++){
						for(int j = 0; j < Ymax; j++){
							for(int k = 0; k < Xmax; k++){
								if(array[k][j][i])
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
			else{
				fails++;
			}
		}
		else{
			fails++;
		}
	}

	return 0;
}

//Initial Placement Functions
bool startPlacement(int length){
	int reset = 0;
	while(reset<5){
		//Exits the function if there are too many fails
		if(reset > 5){
			return false;
		}

		//sets the array to 0
		for(int z=0; z < Zmax; z++){
			for(int y=0; y < Ymax; y++){
				for(int x=0; x < Xmax; x++){
					array[x][y][z]=0;
				}
			}
		}

		//places the blocks
		int blocksPlaced = 0;
		int fails = 0;
		while(blocksPlaced < blocks){
			int x = randint(0, Xmax-1);
			int y = randint(0, Ymax-1);
			int z = randint(0, Zmax-1);
			if(checkSpace(x, y, z, length) == true){
				cord[blocksPlaced][0] = blocksPlaced+1;
				cord[blocksPlaced][1] = x;
				cord[blocksPlaced][2] = y;
				cord[blocksPlaced][3] = z;
				blocksPlaced++;
				placeBlock(x, y, z, blocksPlaced, length);
				fails = 0;
			}
			else{
				fails++;
			}
			if(fails > 10000){
				reset++;
				break;
			}
		}
		if(fails <= 10000){
			return true;
		}
	}
	return false;
}
bool checkSpace(int X, int Y, int Z, int length){
	for(int i = 0; i<length; i++){
		if(array[xpos(X+i)][Y][Z] != 0){
			return false;
		}
	}
	return true;
}
void placeBlock(int X, int Y, int Z, int currentBlock, int length){
	for(int i =0; i < length; i++){
		array[xpos(X+i)][Y][Z] = currentBlock;
	}
}

bool energyCheck(int bid, int xrand, int yrand, int zrand, float energyConstant, int length){

	float energy1 = 0;
	float energy2 = 0;
	int Xval = cord[bid-1][1];
	int Yval = cord[bid-1][2];
	int Zval = cord[bid-1][3];
	for(int X = 0; X < length; X++){
		for(int Y = -1; Y<=1; Y+=2){
			if(array[xpos(Xval+X)][ypos(Yval+Y)][Zval] != 0 && (array[xpos(Xval+X)][ypos(Yval+Y)][Zval]) != bid){
				energy1 += 1;
			}
		}
	}
	for(int X = 0; X < length; X++){
		for(int Y = -1; Y<=1; Y+=2){
			if(array[xpos(Xval+X+xrand)][ypos(Yval+Y+yrand)][zpos(Zval+zrand)] != 0 && (array[xpos(Xval+X+xrand)][ypos(Yval+Y+yrand)][zpos(Zval+zrand)]) != bid){
				energy2 += 1;
			}
		}
	}
	for(int X = 0; X < length; X++){
		for(int Z = -1; Z<=1; Z+=2){
			if(array[xpos(Xval+X)][Yval][zpos(Zval+Z)] != 0 && (array[xpos(Xval+X)][Yval][zpos(Zval+Z)]) != bid){
				energy1 += 1;
			}
		}
	}
	for(int X = 0; X < length; X++){
		for(int Z = -1; Z<=1; Z+=2){
			if(array[xpos(Xval+X+xrand)][ypos(Yval+yrand)][zpos(Zval+Z+zrand)] != 0 && (array[xpos(Xval+X+xrand)][ypos(Yval+yrand)][zpos(Zval+Z+zrand)]) != bid){
				energy2 += 1;
			}
		}
	}

	float tenergy = energy2 - energy1;
	if(tenergy >=0){
		return true;
	}
	float boltz = exp(tenergy * energyConstant);
	float r = randfloat();
	if(r < boltz){
		return true;
	}
	else{
		return false;
	}
}

int randint(int lower, int upper){
	std::uniform_int_distribution<int> distribution(lower, upper);
	int number = distribution(dre);
	return number;
}
float randfloat(){
	std::uniform_real_distribution<float> distribution(0.0,1.0);
	float r = distribution(dre);
	return r;
}

int xpos(int X){
	int X2 = X;
	if(X2 >= Xmax){
		X2 -= Xmax;
	}
	if(X2 < 0){
		X2 += Xmax;
	}
	return X2;
}
int ypos(int Y){
	int Y2 = Y;
	if(Y2 >= Ymax){
		Y2 -= Ymax;
	}
	if(Y2 < 0){
		Y2 += Ymax;
	}
	return Y2;
}
int zpos(int Z){
	int Z2 = Z;
	if(Z2 >= Zmax){
		Z2 -= Zmax;
	}
	if(Z2 < 0){
		Z2 += Zmax;
	}
	return Z2;
}

void updatepos(int bid, int xrand, int yrand, int zrand){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	cord[bid-1][1] = xpos(X+xrand);
	cord[bid-1][2] = ypos(Y+yrand);
	cord[bid-1][3] = zpos(Z+zrand);
}
bool moveCheck(int bid, int xrand, int yrand, int zrand, int length){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	for(int i = 0; i<length; i++){
		if(array[xpos(X+i+xrand)][ypos(Y+yrand)][zpos(Z+zrand)] != 0 && array[xpos(X+i+xrand)][ypos(Y+yrand)][zpos(Z+zrand)] != bid){
			return false;
		}
	}
	return true;
}
void moveBlock(int bid, int xrand, int yrand, int zrand, int length){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	for(int i = 0; i<length; i++){
		array[xpos(X+i)][Y][Z] = 0;
	}
	for(int i = 0; i<length; i++){
		array[xpos(X+i+xrand)][ypos(Y+yrand)][zpos(Z+zrand)] = bid;
	}
}

