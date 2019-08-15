#include <iostream>
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
void printCords();
void printConvergence();
void printClusters();

bool startPlacement();
bool checkSpaceP(int X, int Y, int Z);
bool checkSpaceC(int X, int Y, int Z, int ydir, int zdir);
void placeProtein(int X,int Y, int Z, int currentBlock);
void placeCross(int X, int Y, int Z, int ydir, int zdir);
void removeCross(int X, int Y, int Z);

bool energyCheckP(int bid, int xrand, int yrand, int zrand);
bool energyCheckC(int X, int Y, int Z, int ydir, int zdir, int placement);

bool PorC();
int randint(int lower, int upper); // (inclusive, inclusive)
float randfloat();

int pos(int val, int max);

void updatepos(int bid, int X, int Y, int Z);
bool moveCheck(int bid, int xrand, int yrand, int zrand);
void moveBlock(int bid, int xrand, int yrand, int zrand);

int findBonds(int bondThresh);
bool memloss();
void clusterFindingAlgorithm(char Rdim);
float energyForFiles();
	

int moveDistance = 4;
long long int iderations = 1000000000;
const int Xmax = 200;
const int Ymax = 50;
const int Zmax = 50;
int length = 15;
const int blocks = 1000;
const float PorCprob = .5;
float crossConcen = .01;
float energyConstant = 3;
int initialCrosslinkers = Xmax*Ymax*Zmax*crossConcen/4;

long long int counter = 0;
const int dimension = 4;
bool firstTimeOutputted = true;
int savedFileNumber;
int Pcord[blocks][dimension];
int Parray[Xmax][Ymax][Zmax];
int Carray[Xmax][Ymax][Zmax];

std::random_device rd;
std::uniform_int_distribution<int> seed;
std::default_random_engine dre (time(0));


int main(int argc, char *argv[]){
	if(argc > 1){
		energyConstant = atof(argv[1]);
	}
	if(argc > 2){
		crossConcen = atof(argv[2]);
	}
	if(argc > 3){
		length = atoi(argv[3]);
	}
	if(argc > 4){
		iderations = atoll(argv[4]);
	}
	
	long long int lbvt = iderations/50; //used for the loading bar
	long long int lbv = 1;
	long long int lbvt2 = 250000;  //Adds convergence data every lbvt2 moves
	long long int lbv2 = 1;
	long long int lbv3 = 1;
	long long int lbvt3 = 20000000;  //Prints the lattice data every lbvt3 moves
	if(startPlacement() == false){
		cout<<"There was not enough space to place all the proteins"<<endl;
		return 0;
	}
	
	//loading bar
	cout<<"          |                                                  |"<<endl;
	cout<<"loadingBar|";
	
	printCords();
	int fails = 0;
	while(counter < iderations){
		if(lbv > lbvt){
			cout<<"*";
			lbv = 1;
		}
		if(lbv2 > lbvt2){
			clusterFindingAlgorithm('Y');
			clusterFindingAlgorithm('Z');
			lbv2 = 1;
		}
		if(lbv3 > lbvt3){
			printCords();
			lbv3 = 1;
		}
		
		if(PorC() == true){
			int Xrand = randint(0, Xmax-1);
			int Yrand = randint(0, Ymax-1);
			int Zrand = randint(0, Zmax-1);
			int randomValue = randint(1,4);
			int ydir = 0;
			int zdir = 0;
			switch(randomValue){
				case 1: ydir = 1;  break;
				case 2: ydir = -1; break;
				case 3: zdir = 1;  break;
				case 4: zdir = -1; break;
			}
			
			if(checkSpaceC(Xrand, Yrand, Zrand, ydir, zdir) == true){
				if(energyCheckC(Xrand, Yrand, Zrand, ydir, zdir, 1) == true){
					placeCross(Xrand, Yrand, Zrand, ydir, zdir);
				}
			}
			
			else{
				if(energyCheckC(Xrand, Yrand, Zrand, ydir, zdir, -1) == true){
					removeCross(Xrand, Yrand, Zrand);
				}
			}
		}
		
		else{
			bool test = true;
			int bid = randint(1,blocks);
			int X = Pcord[bid-1][1];
			int Y = Pcord[bid-1][2];
			int Z = Pcord[bid-1][3];
			int Xrand = randint(-2*moveDistance, 2*moveDistance);
			int Yrand = randint(-moveDistance, moveDistance);
			int Zrand = randint(-moveDistance, moveDistance);
			if(moveCheck(bid, Xrand, Yrand, Zrand) == true){
				if(energyCheckP(bid, Xrand, Yrand, Zrand) == true){
					moveBlock(bid, Xrand, Yrand, Zrand);
					updatepos(bid, Xrand, Yrand, Zrand);
					counter++;
					lbv++;
					lbv2++;
					lbv3++;
				}
			}
		}
	}
	cout<<"*|"<<endl;
	
	printCords();

	return 0;	
}
void printCords(){
	ifstream infile;
	ofstream file;
	int fileNumber = -1;
	char val;
	string title = "mc3d2pnoxData";
	stringstream fileName;
	do{
		infile.close();
		stringstream fileName;
		fileNumber++;
		fileName << title << "("<< fileNumber << ").txt";
		infile.open(fileName.str().c_str());
		val = ' ';
		infile >> val;
	}while(val == 'X');
	infile.close();
	fileName << title.c_str() << "("<< fileNumber << ").txt";
	file.open(fileName.str().c_str());
	
	file<<"Xmax = "<<Xmax<<endl;
	file<<"Ymax = "<<Ymax<<endl;
	file<<"Zmax = "<<Zmax<<endl;
	file<<"Proteins = "<<blocks<<endl;
	file<<"Protein Length = "<<length<<endl;
	file<<"Bond Energy = "<<energyConstant<<endl;
	file<<"Iderations = "<<counter<<endl;
	file<<"Crosslinker Concentration "<<crossConcen<<endl;
	file<<"Proteins With Less Than 1 Bond = "<<findBonds(1)<<endl;
	file<<"Proteins With Less Than 2 Bond = "<<findBonds(2)<<endl;
	file<<"Proteins With Less Than 3 Bond = "<<findBonds(3)<<endl;
	file<<"Proteins With Less Than 4 Bond = "<<findBonds(4)<<endl;
	file<<"Proteins With Less Than 5 Bond = "<<findBonds(5)<<endl;
	file<<"Proteins With Less Than 6 Bond = "<<findBonds(6)<<endl;
	file<<"Proteins With Less Than 7 Bond = "<<findBonds(7)<<endl;
	file<<"Proteins With Less Than 8 Bond = "<<findBonds(8)<<endl;
	file<<"Free Energy = "<<energyForFiles()<<endl;
	
	for(int X = 0; X<Xmax; X++){
		for(int Y = 0; Y<Ymax; Y++){
			for(int Z = 0; Z<Zmax; Z++){
				int value = 0;
				if(Parray[X][Y][Z] != 0){
					value += 2;
				}
				if(Carray[X][Y][Z] != 0){
					value+=1;
				}
				file<<value<<" ";
			}
			file<<endl;
		}
		file<<endl;
	}
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
			fileName << title.c_str() << Rdim << "("<< fileNumber << ").txt";
			infile.open(fileName.str().c_str());
			val = ' ';
			infile >> val;
		}while(val == 'C');
		infile.close();
		stringstream fileName;
		fileName << title.c_str() << Rdim << "("<< fileNumber << ").txt";
		file.open(fileName.str().c_str());
		file<< title.c_str() << Rdim << endl << "counter length width len/wid freeEnergy"<<endl;
		file<< crossConcen <<" "<<energyConstant<<endl;
		firstTimeOutputted = false;
		file.close();
		savedFileNumber = fileNumber;
	}
		
	stringstream fileName;
	fileName << title.c_str() << Rdim << "("<< savedFileNumber <<").txt";
	file.open(fileName.str().c_str(), std::ios::app);
	
	double Lval = static_cast<double> (L);
	double Rval = static_cast<double> (R);
	
	file<<counter<<" "<<L<<" "<<R<<" "<<Lval/Rval<<" "<<energyForFiles()<<endl;
	file.close();
}

void printConvergence(){
	ofstream file;
	ifstream infile;
	int fileNumber = -1;
	char val;
	string title = "mc3d2pnoxConvergenceData";
	if(firstTimeOutputted == true){
		do{
			infile.close();
			stringstream fileName;
			fileNumber++;
			fileName << title.c_str() <<"("<< fileNumber << ").txt";
			infile.open(fileName.str().c_str());
			val = ' ';
			infile >> val;
		}while(val == 'C');
		infile.close();
		stringstream fileName;
		fileName << title.c_str() <<"("<< fileNumber << ").txt";
		file.open(fileName.str().c_str());
		file<< title <<endl<<"IterationsCompleted   blocks<1   blocks<2   blocks<3   blocks<4   blocks<5   blocks<6   blocks<7   blocks<8"<<endl;
		firstTimeOutputted = false;
		file.close();
		savedFileNumber = fileNumber;
	}
	stringstream fileName;
	fileName << title.c_str() <<"("<< savedFileNumber <<").txt";
	file.open(fileName.str().c_str(), std::ios::app);
	
	file<<counter<<" "<<findBonds(1)<<" "<<findBonds(2)<<" "<<findBonds(3)<<" "<<findBonds(4)<<" "<<findBonds(5)<<" "<<findBonds(6)<<" "<<findBonds(7)<<" "<<findBonds(8)<<" "<<energyForFiles()<<endl;
}

bool startPlacement(){
	int value = 0;
	//sets the array to 0 
	for(int z=0; z < Zmax; z++){
		for(int y=0; y < Ymax; y++){
			for(int x=0; x < Xmax; x++){
				Parray[x][y][z]=0;
				Carray[x][y][z]=0;
			}
		}
	}
	
	//places the proteins
	int blocksPlaced = 0;
	int fails = 0;
	while(blocksPlaced < blocks){
		int x = randint(0, Xmax-1);
		int y = randint(0, Ymax-1);
		int z = randint(0, Zmax-1);
		if(checkSpaceP(x,y,z) == true){
			Pcord[blocksPlaced][0] = blocksPlaced+1;
			Pcord[blocksPlaced][1] = x;
			Pcord[blocksPlaced][2] = y;
			Pcord[blocksPlaced][3] = z;
			blocksPlaced++;
			placeProtein(x, y, z, blocksPlaced);
			fails = 0;
		} 
	}
	
	//places random crosslinkers
	while(value < initialCrosslinkers){
		int Xrand = randint(0, Xmax-1);
		int Yrand = randint(0, Ymax-1);
		int Zrand = randint(0, Zmax-1);
		int randomValue = randint(1,4);
		int ydir = 0;
		int zdir = 0;
		switch(randomValue){
			case 1: ydir = 1;  break;
			case 2: ydir = -1; break;
			case 3: zdir = 1;  break;
			case 4: zdir = -1; break;
		}
		if(checkSpaceC(Xrand, Yrand, Zrand, ydir, zdir) == true){
			placeCross(Xrand, Yrand, Zrand, ydir, zdir);
			value++;
		}
	}
	return true;
}

bool checkSpaceP(int X, int Y, int Z){
	for(int i = 0; i<length; i++){
		if(Parray[pos(X+i, Xmax)][Y][Z] != 0){
			return false;
		}
	}
	return true;
}
bool checkSpaceC(int X, int Y, int Z, int ydir, int zdir){
	if(Carray[X][Y][Z] == 0  &&  Carray[X][pos(Y+ydir, Ymax)][pos(Z+zdir, Zmax)] == 0){
		return true;
	}
	return false;
}
void placeProtein(int X, int Y, int Z, int currentBlock){
	for(int i =0; i < length; i++){
		Parray[pos(X+i, Xmax)][Y][Z] = currentBlock;
	}
}
void placeCross(int X, int Y, int Z, int ydir, int zdir){
	int cid = X + 3*Y + 5*Z + 1;
	Carray[X][Y][Z] = cid;
	Carray[X][pos(Y+ydir, Ymax)][pos(Z+zdir, Zmax)] = cid;
}
void removeCross(int X, int Y, int Z){
	int cid = Carray[X][Y][Z];
	Carray[X][Y][Z] = 0;

	     if(Carray[X][pos(Y+1, Ymax)][Z] == cid) {Carray[X][pos(Y+1, Ymax)][Z] = 0;}
	else if(Carray[X][pos(Y-1, Ymax)][Z] == cid) {Carray[X][pos(Y-1, Ymax)][Z] = 0;}
	else if(Carray[X][Y][pos(Z+1, Zmax)] == cid) {Carray[X][Y][pos(Z+1, Zmax)] = 0;}
	else if(Carray[X][Y][pos(Z-1, Zmax)] == cid) {Carray[X][Y][pos(Z-1, Zmax)] = 0;}
}


bool energyCheckP(int bid, int xrand, int yrand, int zrand){

	float energy1 = 0;
	float energy2 = 0;
	int Xval = Pcord[bid-1][1];
	int Yval = Pcord[bid-1][2];
	int Zval = Pcord[bid-1][3];
	for(int X = 0; X < length; X++){
		if(Carray[pos(Xval+X, Xmax)][Yval][Zval] != 0){
			energy1 += 1;
		}
	}
	for(int X = 0; X < length; X++){
		if(Carray[pos(Xval+X+xrand, Xmax)][pos(Yval+yrand, Ymax)][pos(Zval+zrand, Zmax)] != 0){
			energy2 += 1;
		}
	}
	
	float tenergy = energy2 - energy1;
	if(tenergy >=0){
		return true;
	}
	float boltz = exp(tenergy * energyConstant);
	float random = randfloat();
	if(random < boltz){
		return true;
	}
	else{
		return false;
	}
}
bool energyCheckC(int X, int Y, int Z, int ydir, int zdir, int placement){
	float energy = 0;
	if(placement == -1){
		int ydir = 0;
		int zdir = 0;
		int cid = Carray[X][Y][Z];
		
		     if(Carray[X][pos(Y+1, Ymax)][Z] == cid) {ydir =  1; zdir = 0;}
		else if(Carray[X][pos(Y-1, Ymax)][Z] == cid) {ydir = -1; zdir = 0;}
		else if(Carray[X][Y][pos(Z+1, Zmax)] == cid) {ydir =  0; zdir = 1;}
		else if(Carray[X][Y][pos(Z-1, Zmax)] == cid) {ydir =  0; zdir =-1;}
	}
	if(Parray[X][Y][Z] != 0){
		energy += 1;
	}
	if(Parray[X][pos(Y+ydir, Ymax)][pos(Z+zdir, Zmax)] != 0){
		energy += 1;
	}
	float boltz = pow(crossConcen, placement) * exp(placement * energy * energyConstant);
	float random = randfloat();
	if(random < boltz){
		return true;
	}
	else{
		return false;
	}
}

bool PorC(){
	float value = randfloat();
	if(value <= PorCprob){
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
	int X = Pcord[bid-1][1];
	int Y = Pcord[bid-1][2];
	int Z = Pcord[bid-1][3];
	Pcord[bid-1][1] = pos(X+xrand, Xmax);
	Pcord[bid-1][2] = pos(Y+yrand, Ymax);
	Pcord[bid-1][3] = pos(Z+zrand, Zmax);
}
bool moveCheck(int bid, int xrand, int yrand, int zrand){
	int X = Pcord[bid-1][1];
	int Y = Pcord[bid-1][2];
	int Z = Pcord[bid-1][3];
	for(int i = 0; i<length; i++){
		if(Parray[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] != 0 && Parray[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] != bid){
			return false;
		}
	}
	return true;
}
void moveBlock(int bid, int xrand, int yrand, int zrand){
	int X = Pcord[bid-1][1];
	int Y = Pcord[bid-1][2];
	int Z = Pcord[bid-1][3];
	for(int i = 0; i<length; i++){
		Parray[pos(X+i, Xmax)][Y][Z] = 0;
	}
	for(int i = 0; i<length; i++){
		Parray[pos(X+i+xrand, Xmax)][pos(Y+yrand, Ymax)][pos(Z+zrand, Zmax)] = bid;
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
		int X = Pcord[bid-1][1];
		int Y = Pcord[bid-1][2];
		int Z = Pcord[bid-1][3];
		for(int yval = -1; yval<=1; yval+=2){
			for(int xval = 0; xval<length; xval++){
				if(Parray[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z] != 0){
					bool test = true;
					for(int previous = 0; previous < 8; previous++){
						if(Parray[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z] == listOfInteractions[previous]){
							test = false;
							break;
						}
					}
					if(test == true){
						bondsPerBlock++;
						listOfInteractions[listval] = Parray[pos(X+xval, Xmax)][pos(Y+yval, Ymax)][Z]; 
						listval++;
					}
				}
			}
		}
		for(int zval = -1; zval<=1; zval+=2){
			for(int xval = 0; xval<length; xval++){
				if(Parray[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)] != 0){
					bool test = true;
					for(int previous = 0; previous < 8; previous++){
						if(Parray[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)] == listOfInteractions[previous]){
							test = false;
							break;
						}
					}
					if(test == true){
						bondsPerBlock++;
						listOfInteractions[listval] = Parray[pos(X+xval, Xmax)][Y][pos(Z+zval, Zmax)]; 
						listval++;
					}
				}
			}
		}	
		if(bondsPerBlock < bondThresh){
			totalBonds++;
		}	
	}
	return totalBonds;	
}
bool memloss(){
	for(int Z; Z<Zmax; Z++){
		for(int Y; Y<Ymax; Y++){
			for(int X; X<Xmax; X++){
				if(Carray[X][Y][Z] != 0){
					int test = 0;
					int cid = Carray[X][Y][Z];
					
					if(Carray[pos(X+1, Xmax)][Y][Z] == cid) {test++;} 
					if(Carray[pos(X-1, Xmax)][Y][Z] == cid) {test++;} 
					if(Carray[X][pos(Y+1, Ymax)][Z] == cid) {test++;} 
					if(Carray[X][pos(Y-1, Ymax)][Z] == cid) {test++;} 
					if(Carray[X][Y][pos(Z+1, Zmax)] == cid) {test++;} 
					if(Carray[X][Y][pos(Z-1, Zmax)] == cid) {test++;} 
					 
					if(test != 1){
						return false;
					}
				}
			}
		}
	}
	return true;
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
					clusterArray[L][R] = Parray[L][R][layer];
					initialClusterArray[L][R] = 0;
				}
			}
		}
		else{
			for(int L = 0; L<Lmax; L++){
				for(int R = 0; R<Rmax; R++){
					clusterArray[L][R] = Parray[L][layer][R];
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
	if(largestCurrentL[0] > 2*length  &&  largestCurrentR > 3){
		printClusterData(largestCurrentL[0], largestCurrentR, Rdim);
	}
}

float energyForFiles(){
	int energy = blocks * length;
	for(int X = 0; X<Xmax; X++){
		for(int Y = 0; Y<Ymax; Y++){
			for(int Z = 0; Z<Zmax; Z++){
				if(Parray[X][Y][Z] != 0  &&  Carray[X][Y][Z] != 0){
					energy--;
				}
				else if(Parray[X][Y][Z] == 0  &&  Carray[X][Y][Z] != 0){
					energy++;
				}
			}
		}
	}
	float freeEnergy = energy * energyConstant;
	return freeEnergy;
}


