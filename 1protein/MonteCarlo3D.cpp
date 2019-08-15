##include <iostream>
#include <ctime>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <random>

using namespace std;


//functions
void printCords(); //prints the cordinate array to a file
void printConvergence(); //prints convergence data to a file
void printClusterData(int L, int R, char Rdim); //prints the clusterfinding algorithms data to a sepperate file
												//rdim accepts 'Z' or 'Y' as argument for choosing the dimension for R in the calculation

bool startPlacement(); //returns true if all of the blocks are able to be placed
bool checkSpace(int X, int Y, int Z); //returns true if there are no blocks where a block will be placed 
void placeBlock(int X,int Y, int Z, int currentBlock); //places the initial blocks for X Y Z from checkspace

bool energyCheck(int bid, int xrand, int yrand, int zrand); //runs energy check

int randomBlock(); //returns random int from 0 to number of blocks -1
int randint(int lower, int upper); // (inclusive, inclusive)
float randfloat(); //returns random float between 0 and 1

int pos(int val, int max); //used for periodic boundary functions. 

void updatepos(int bid, int X, int Y, int Z); //changes the cordinates of the proteins in the cord array
bool moveCheck(int bid, int xrand, int yrand, int zrand); //checks to make sure that the random move is valid, returns false if not valid
void moveBlock(int bid, int xrand, int yrand, int zrand); //moves the protein if move check returns true

int findBonds(int bondThresh); //retuns the number of proteins in the simulation that are bonded with "bondThresh" number of bonds 
void clusterFindingAlgorithm(char Rdim);
double energyForCalculation();
	

int moveDistance = 4;
long long int counter = 0;
long long int iderations = 1000000;
const int Xmax = 200;
const int Ymax = 50;
const int Zmax = 50;
const int length = 5;
const int blocks = 2000;
const int dimension = 4;
float energyConstant = .8;
bool firstTimeOutputted = true;
int savedFileNumber;
int cord[blocks][dimension];
int array[Xmax][Ymax][Zmax];
int visibleArray [Xmax][Ymax];

int seed = time(0);
std::default_random_engine dre (seed);

int main(int argc, char* argv[]){
	if(argc > 1){
		energyConstant = atof(argv[1]);
	}
	if(argc > 2){
		iderations = atoll(argv[2]);
	}
	if(startPlacement() == false){
		cout<<"There was not enough space to place all the proteins"<<endl;
		return 0;
	}
	
	int xrand;
	int yrand; 
	int zrand;
	int value = iderations / 50;
	int value2 = 250000;
	int value3 = 10000000;
	int step = 0;
	int step2 = 0;
	int step3 = 0;
	cout<<"Loading Bar> |                                                  |"<<endl;
	cout<<"             |";
	
	printCords();
	
	while(counter < iderations){
		//prints cords every {value} incriments
		if(step >= value){
			//cout is for loading bar
			cout<<"*";
			step = 0;
		}
		
		//prints convergence every {value2} incriments
		if(step2 >= value2){
			clusterFindingAlgorithm('Y');
			clusterFindingAlgorithm('Z');
			printConvergence();
			step2 = 0;
		}
		
		if(step3 >= value3){
			printCords();
			step3 = 0;
		}
		
		int bid = randint(1, blocks);
		xrand = randint(-(2*moveDistance), (2*moveDistance));
		yrand = randint(-moveDistance, moveDistance);
		zrand = randint(-moveDistance, moveDistance);
		if(moveCheck(bid, xrand, yrand, zrand) == true){
			if(energyCheck(bid, xrand, yrand, zrand) == true){
				moveBlock(bid, xrand, yrand, zrand);
				updatepos(bid, xrand, yrand, zrand);
				counter++;
				step++;
				step2++;
				step3++;
			}
		}
	}
	//cout is for loading bar
	cout<<"*|"<<endl;
	
	printCords();
	
	return 0;	
}


void printCords(){
	ifstream infile;
	ofstream file;
	int fileNumber = -1;
	char val;
	stringstream fileName;
	do{
		infile.close();
		stringstream fileName;
		fileNumber++;
		fileName << "3DCordData("<< fileNumber << ").txt";
		infile.open(fileName.str().c_str());
		val = ' ';
		infile >> val;
	}while(val == 'X');
	infile.close();
	fileName << "3DCordData("<< fileNumber << ").txt";
	file.open(fileName.str().c_str());
	
	file<<"Xmax = "<<Xmax<<endl;
	file<<"Ymax = "<<Ymax<<endl;
	file<<"Zmax = "<<Zmax<<endl;
	file<<"Proteins = "<<blocks<<endl;
	file<<"Protein Length = "<<length<<endl;
	file<<"Bond Energy = "<<energyConstant<<endl;
	file<<"Iderations = "<<counter<<endl;
	file<<"Proteins With Less Than 1 Bond = "<<findBonds(1)<<endl;
	file<<"Proteins With Less Than 2 Bond = "<<findBonds(2)<<endl;
	file<<"Proteins With Less Than 3 Bond = "<<findBonds(3)<<endl;
	file<<"Proteins With Less Than 4 Bond = "<<findBonds(4)<<endl;
	file<<"Proteins With Less Than 5 Bond = "<<findBonds(5)<<endl;
	file<<"Proteins With Less Than 6 Bond = "<<findBonds(6)<<endl;
	file<<"Proteins With Less Than 7 Bond = "<<findBonds(7)<<endl;
	file<<"Proteins With Less Than 8 Bond = "<<findBonds(8)<<endl;
	
	
	for(int Z = 0; Z<Zmax; Z++){
		for(int Y = 0; Y<Ymax; Y++){
			for(int X = 0; X<Xmax; X++){
				if(array[X][Y][Z] != 0){
					file<<"1 ";
				}
				else{
					file<<"0 ";
				}
			}
			file<<endl;
		}
		file<<endl;
	}
}

void printConvergence(){
	ofstream file;
	ifstream infile;
	int fileNumber = -1;
	char val;
	string title = "ConvergenceData";
	if(firstTimeOutputted == true){
		do{
			infile.close();
			stringstream fileName;
			fileNumber++;
			fileName << title << "("<< fileNumber << ").txt";
			infile.open(fileName.str().c_str());
			val = ' ';
			infile >> val;
		}while(val == 'C');
		infile.close();
		stringstream fileName;
		fileName << title << "("<< fileNumber << ").txt";
		file.open(fileName.str().c_str());
		file<< title <<endl<<"IterationsCompleted   blocks<1   blocks<2   blocks<3   blocks<4   blocks<5   blocks<6   blocks<7   blocks<8"<<endl;
		firstTimeOutputted = false;
		file.close();
		savedFileNumber = fileNumber;
	}
	stringstream fileName;
	fileName << title << "("<< savedFileNumber <<").txt";
	file.open(fileName.str().c_str(), std::ios::app);
	
	file<<counter<<" "<<findBonds(1)<<" "<<findBonds(2)<<" "<<findBonds(3)<<" "<<findBonds(4)<<" "<<findBonds(5)<<" "<<findBonds(6)<<" "<<findBonds(7)<<" "<<findBonds(8)<<endl;
	file.close();
}

void printClusterData(int L, int R, char Rdim){
	ofstream file;
	ifstream infile;
	int fileNumber = -1;
	char val;
	string title = "ClusterData";
	if(firstTimeOutputted == true){
		do{
			infile.close();
			stringstream fileName;
			fileNumber++;
			fileName << title << Rdim << "("<< fileNumber << ").txt";
			infile.open(fileName.str().c_str());
			val = ' ';
			infile >> val;
		}while(val == 'C');
		infile.close();
		stringstream fileName;
		fileName << title << Rdim << "("<< fileNumber << ").txt";
		file.open(fileName.str().c_str());
		file<< title << Rdim << endl << "counter length width len/wid freeEnergy"<<endl;
		firstTimeOutputted = false;
		file.close();
		savedFileNumber = fileNumber;
	}
	stringstream fileName;
	fileName << title << Rdim << "("<< savedFileNumber <<").txt";
	file.open(fileName.str().c_str(), std::ios::app);
	
	double Rval = static_cast<double>(R);
	double Lval = static_cast<double>(L);
	
	file<<counter<<" "<<L<<" "<<R<<" "<<Lval/Rval<<" "<<energyForCalculation()<<endl;
	file.close();
}

//Initial Placement Functions
bool startPlacement(){
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
			if(checkSpace(x,y,z) == true){
				cord[blocksPlaced][0] = blocksPlaced+1;
				cord[blocksPlaced][1] = x;
				cord[blocksPlaced][2] = y;
				cord[blocksPlaced][3] = z;
				blocksPlaced++;
				placeBlock(x, y, z, blocksPlaced);
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
bool checkSpace(int X, int Y, int Z){
	for(int i = 0; i<length; i++){
		if(array[pos(X+i, Xmax)][Y][Z] != 0){
			return false;
		}
	}
	return true;
}
void placeBlock(int X, int Y, int Z, int currentBlock){
	for(int i =0; i < length; i++){
		array[pos(X+i, Xmax)][Y][Z] = currentBlock;
	}
}

bool energyCheck(int bid, int xrand, int yrand, int zrand){

	float energy1 = 0;
	float energy2 = 0;
	int Xval = cord[bid-1][1];
	int Yval = cord[bid-1][2];
	int Zval = cord[bid-1][3];
	for(int X = 0; X < length; X++){
		for(int Y = -1; Y<=1; Y+=2){
			if(array[pos(Xval+X, Xmax)][pos(Yval+Y, Ymax)][Zval] != 0 && (array[pos(Xval+X, Xmax)][pos(Yval+Y, Ymax)][Zval]) != bid){
				energy1 += 1;
			}
		}
	}
	for(int X = 0; X < length; X++){
		for(int Y = -1; Y<=1; Y+=2){
			if(array[pos(Xval+X+xrand, Xmax)][pos(Yval+Y+yrand, Ymax)][pos(Zval+zrand, Zmax)] != 0 && (array[pos(Xval+X+xrand, Xmax)][pos(Yval+Y+yrand, Ymax)][pos(Zval+zrand, Zmax)]) != bid){
				energy2 += 1;
			}
		}	
	}
	for(int X = 0; X < length; X++){
		for(int Z = -1; Z<=1; Z+=2){
			if(array[pos(Xval+X, Xmax)][Yval][pos(Zval+Z, Zmax)] != 0 && (array[pos(Xval+X, Xmax)][Yval][pos(Zval+Z, Zmax)]) != bid){
				energy1 += 1;
			}
		}
	}
	for(int X = 0; X < length; X++){
		for(int Z = -1; Z<=1; Z+=2){
			if(array[pos(Xval+X+xrand, Xmax)][pos(Yval+yrand, Ymax)][pos(Zval+Z+zrand, Zmax)] != 0 && (array[pos(Xval+X+xrand, Xmax)][pos(Yval+yrand, Ymax)][pos(Zval+Z+zrand, Zmax)]) != bid){
				energy2 += 1;
			}
		}	
	}
//	if(array[xpos(Xval-1)][Yval][Zval] != 0 && array[xpos(Xval-1)][Yval][Zval] != bid){
//		energy1 += 0;
//	}
//	if(array[xpos(Xval+length)][Yval][Zval] != 0 && array[xpos(Xval+length)][Yval][Zval] != bid){
//		energy1 += 0;
//	}
//	if(array[xpos(Xval-1+xrand)][ypos(Yval+yrand)][zpos(Zval+zrand)] != 0 && array[xpos(Xval-1+xrand)][ypos(Yval+yrand)][zpos(Zval+zrand)] != bid){
//		energy2 += 0;
//	}
//	if(array[xpos(Xval+length+xrand)][ypos(Yval+yrand)][zpos(Zval+zrand)] != 0 && array[xpos(Xval+length+xrand)][ypos(Yval+yrand)][zpos(Zval+zrand)] != bid){
//		energy2 += 0;
//	}
	
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

int pos(int val, int max){
	if(val >= max){
		val -= max;
	}
	if(val < 0){
		val += max;
	}
	return val;
}

void updatepos(int bid, int xrand, int yrand, int zrand){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	cord[bid-1][1] = pos(X+xrand, Xmax);
	cord[bid-1][2] = pos(Y+yrand, Ymax);
	cord[bid-1][3] = pos(Z+zrand, Zmax);
}
bool moveCheck(int bid, int xrand, int yrand, int zrand){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	for(int i = 0; i<length; i++){
		if(array[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] != 0 && array[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] != bid){
			return false;
		}
	}
	return true;
}
void moveBlock(int bid, int xrand, int yrand, int zrand){
	int X = cord[bid-1][1];
	int Y = cord[bid-1][2];
	int Z = cord[bid-1][3];
	for(int i = 0; i<length; i++){
		array[pos(X+i, Xmax)][Y][Z] = 0;
	}
	for(int i = 0; i<length; i++){
		array[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] = bid;
	}
}

int findBonds(int bondThresh){
	int totalBonds=0;
	int listOfInteractions[8];
	
	for(int bid = 1; bid <= blocks; bid++){
		int listval = 0;
		for(int i=0; i<8; i++){
			listOfInteractions[i] = 0;
		}
		
		int bondsPerBlock = 0;
		int X = cord[bid-1][1];
		int Y = cord[bid-1][2];
		int Z = cord[bid-1][3];
		for(int yval = -1; yval<=1; yval+=2){
			for(int xval = 0; xval<length; xval++){
				if(array[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z] != 0){
					bool test = true;
					for(int previous = 0; previous < 8; previous++){
						if(array[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z] == listOfInteractions[previous]){
							test = false;
							break;
						}
					}
					if(test == true){
						bondsPerBlock++;
						listOfInteractions[listval] = array[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z]; 
						listval++;
					}
				}
			}
		}
		for(int zval = -1; zval<=1; zval+=2){
			for(int xval = 0; xval<length; xval++){
				if(array[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)] != 0){
					bool test = true;
					for(int previous = 0; previous < 8; previous++){
						if(array[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)] == listOfInteractions[previous]){
							test = false;
							break;
						}
					}
					if(test == true){
						bondsPerBlock++;
						listOfInteractions[listval] = array[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)]; 
						listval++;
					}
				}
			}
		}	
		if(bondsPerBlock < bondThresh){
			totalBonds++;
		}	
	}
	return blocks - totalBonds;	
}

void clusterFindingAlgorithm(char Rdim){
	int Lval;
	int Rval;
	int lay;
	
	switch(Rdim){		
		case 'Y':
			Rval = Ymax;
			lay = Zmax;
			break;
		case 'Z':
			Rval = Zmax;
			lay = Ymax;
			break;
	}
	
	const int Lmax = Xmax;
	const int Rmax = Rval;
	const int layers = lay;
	const int size = Rmax * Lmax / length;
	double maxRatio;
	
	int initialClusterArray[Lmax][Rmax];
	int clusterArray[Lmax][Rmax];
	
	int largestCurrentL[2] = {0,-1}; //length, cluster Id
	int largestCurrentR = 0;
	
	for(int layer = 0; layer < layers; layer++){
		if(Rdim == 'Y'){
			for(int L = 0; L<Lmax; L++){
				for(int R = 0; R<Rmax; R++){
					clusterArray[L][R] = array[L][R][layer];
					initialClusterArray[L][R] = 0;
				}
			}
		}
		else{
			for(int L = 0; L<Lmax; L++){
				for(int R = 0; R<Rmax; R++){
					clusterArray[L][R] = array[L][layer][R];
					initialClusterArray[L][R] = 0;
				}
			}
		}
		
		int largestLabel = 0;
		for(int R = 0; R<=Rmax; R++){
			for(int L = 0; L<=Lmax; L++){
				if(clusterArray[pos(L, Lmax)][pos(R, Rmax)] != 0){
					int left = initialClusterArray[pos(L-1, Lmax)][pos(R, Rmax)];
					int up = initialClusterArray[pos(L, Lmax)][pos(R-1, Rmax)];
					int right = initialClusterArray[pos(L+1, Lmax)][pos(R, Rmax)];
					int center = initialClusterArray[pos(L, Lmax)][pos(R, Rmax)];
					if(L == Lmax){
						if(center != 0  &&  left != 0){
							for(int i = 0; i<Lmax; i++){
								for(int j = 0; j<Rmax; j++){
									if(initialClusterArray[i][j] == center){
										initialClusterArray[i][j] = left;
									}
								}
							}
						}
					}
					else if(R == Rmax){
						if(center != 0  &&  up != 0){
							for(int i = 0; i<Lmax; i++){
								for(int j = 0; j<Rmax; j++){
									if(initialClusterArray[i][j] == center){
										initialClusterArray[i][j] = up;
									}
								}
							}
						}
					}
					else if(left  == 0  &&  up == 0){
						largestLabel++;
						initialClusterArray[pos(L, Lmax)][pos(R, Rmax)] = largestLabel;
					}
					else if(left  != 0  &&  up == 0){
						initialClusterArray[pos(L, Lmax)][pos(R, Rmax)] = left;
					}
					else if(left == 0  &&  up != 0){
						initialClusterArray[pos(L, Lmax)][pos(R, Rmax)] = up;
					}
					else if(left != 0  &&  up != 0){
						if(left == up){
							initialClusterArray[pos(L, Lmax)][pos(R, Rmax)] = up;
						}
						else{
							initialClusterArray[pos(L, Lmax)][pos(R, Rmax)] = up;
							for(int i = 0; i<Lmax; i++){
								for(int j = 0; j<Rmax; j++){
									if(initialClusterArray[i][j] == left){
										initialClusterArray[i][j] = up;
									}
								}
							}
						}
					}
				}	
			}
		}
		int currentL = 0;
		int currentR = 0;
		int lastBid = -1;
		bool connected = false;
		for(int R = 0; R<Rmax; R++){
			int lval = 0;
			lastBid = -1;
			while(lval < Lmax  ||  connected == true){
				if(initialClusterArray[pos(lval, Lmax)][R] != 0){
					currentL++;
					lastBid = initialClusterArray[pos(lval, Lmax)][R];
					lval++;
					connected = true;
				}
				else if(connected = true){
					bool up = true; 
					bool down = true;
					int lbuf = 0;
					while(connected == true){
						if(initialClusterArray[pos(lval, Lmax)][R] == 0){
							if(up == true){
								if(initialClusterArray[pos(lval-1, Lmax)][pos(R+1, Rmax)] == 0){
									up = false;
								}
							}
							if(down == true){
								if(initialClusterArray[pos(lval-1, Lmax)][pos(R-1, Rmax)] == 0){
									down = false;
								}
							}
							if(up == false  &&  down == false){
								if(largestCurrentL[0] < currentL){
									largestCurrentL[0] = currentL;
									largestCurrentL[1] = lastBid;
								}
								currentL = 0;
								connected = false;
								lbuf = 0;
								lval++;
								break;
							}
						}
						else{
							if(up == true  &&  initialClusterArray[pos(lval, Lmax)][pos(R+1, Rmax)] != 0  &&  initialClusterArray[pos(lval-1, Lmax)][pos(R+1, Rmax)] != 0){
								lval++;
								currentL = currentL + lbuf;
								lbuf = 0;
								break;
							}
							else if(down == true  &&  initialClusterArray[pos(lval, Lmax)][pos(R-1, Rmax)] != 0  &&  initialClusterArray[pos(lval-1, Lmax)][pos(R-1, Rmax)] != 0){
								lval++;
								currentL = currentL + lbuf;
								lbuf = 0;
								break;
							}
						}
						lbuf++;
						lval++;
						if(lval > 2*Lmax){
							largestCurrentL[0] = Lmax;
							break;
						}
					}
				}
				else{
					lval++;
				}
				if(lval > 2*Lmax){
					largestCurrentL[0] = Lmax;
					break;
				}
			}
		}
		connected = false;
		for(int L = 0; L<Lmax; L++){
			int rval = 0;
			while(rval < Rmax  ||  connected == true){
				if(initialClusterArray[L][pos(rval, Rmax)] == largestCurrentL[1]){
					currentR++;
					rval++;
					connected = true;
				}
				else if(connected = true){
					bool left = true; 
					bool right = true;
					int rbuf = 0;
					while(connected == true){
						if(initialClusterArray[L][pos(rval, Rmax)] == 0){
							if(left == true){
								if(initialClusterArray[pos(L-1, Lmax)][pos(rval-1, Rmax)] == 0){
									left = false;
								}
							}
							if(right == true){
								if(initialClusterArray[pos(L+1, Lmax)][pos(rval-1, Rmax)] == 0){
									right = false;
								}
							}
							if(left == false  &&  right == false){
								if(largestCurrentR < currentR){
									largestCurrentR = currentR;
								}
								currentR = 0;
								connected = false;
								rbuf = 0;
								rval++;
								break;
							}
						}
						else{
							if(left == true  &&  initialClusterArray[pos(L-1, Lmax)][pos(rval, Rmax)] == largestCurrentL[1]  &&  initialClusterArray[pos(L-1, Lmax)][pos(rval-1, Rmax)] == largestCurrentL[1]){
								rval++;
								currentR = currentR + rbuf;
								rbuf = 0;
								break;
							}
							else if(right == true  &&  initialClusterArray[pos(L+1, Lmax)][pos(rval, Rmax)] == largestCurrentL[1]  &&  initialClusterArray[pos(L+1, Lmax)][pos(rval-1, Rmax)] == largestCurrentL[1]){
								rval++;
								currentR = currentR + rbuf;
								rbuf = 0;
								break;
							}
						}
						rbuf++;
						rval++;
						if(rval > 2*Rmax){
							largestCurrentR = Rmax;
							break;
						}
					}
				}
				else{
					rval++;
				}
				if(rval > 2*Rmax){
					largestCurrentR = Rmax;
					break;
				}
			}
		}
	}
	if(largestCurrentL[0] > 2*length  ||  largestCurrentR > 3){
		printClusterData(largestCurrentL[0], largestCurrentR, Rdim);
	}
}

double energyForCalculation(){
	int totalEnergy = blocks*length*8;
	for(int Z = 0; Z<Zmax; Z++){
		for(int Y = 0; Y<Ymax; Y++){
			for(int X = 0; X<Xmax; X++){
				if(array[X][Y][Z] != 0){
					if(array[X][pos(Y-1, Ymax)][pos(Z, Zmax)] != 0){totalEnergy--;}
					if(array[X][pos(Y+1, Ymax)][pos(Z, Zmax)] != 0){totalEnergy--;}
					if(array[X][pos(Y, Ymax)][pos(Z+1, Zmax)] != 0){totalEnergy--;}
					if(array[X][pos(Y, Ymax)][pos(Z-1, Zmax)] != 0){totalEnergy--;}
				}
			}
		}
	}
	double freeEnergy = totalEnergy*energyConstant;
	return freeEnergy;
}
