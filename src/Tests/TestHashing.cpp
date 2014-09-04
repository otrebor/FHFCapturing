/*
 * TestHashing.cpp
 *
 *  Created on: 17/set/2012
 *      Author: bob
 */



#include <iostream>
#include "../UniversalHashFunction.h"
#include <vector>

#define TABLE_SIZE 1201 //must be prime
#define KEY_LEN 4 //in bytes
#define STAGES 4


using namespace std;
using namespace FHFMultistageFilter;


void testHashing() {
	cout << "!!!Test Hashing (Basic) !!!" << endl;
	UniversalHashFunction* hashingFun = UniversalHashFunction::functionsGenerator(TABLE_SIZE,KEY_LEN,STAGES);
	int prova = 32412412;
	for(int stage = 0 ; stage < STAGES ; stage++)
		cout << hashingFun[stage].Hash( (unsigned char*)&prova ) << " - ";
	cout << "\n";
	for(int stage = 0 ; stage < STAGES ; stage++)
		cout << hashingFun[stage].Hash( (unsigned char*)&prova ) << " - ";
	cout << "\n ------ \n";
	prova = 32412411;
	for(int stage = 0 ; stage < STAGES ; stage++)
		cout << hashingFun[stage].Hash( (unsigned char*)&prova ) << " - ";
	cout << "\n";
	for(int stage = 0 ; stage < STAGES ; stage++)
		cout << hashingFun[stage].Hash( (unsigned char*)&prova ) << " - ";
}

