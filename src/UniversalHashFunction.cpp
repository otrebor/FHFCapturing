/*
 * UniversalHashFunction.cpp
 *
 *  Created on: 16/set/2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#include <ctime>
#include <cstdlib>
#include <iostream>
#include "UniversalHashFunction.h"

#define BYTE_MAX_VAL 255

namespace FHFMultistageFilter {

UniversalHashFunction::UniversalHashFunction(int dim , int* perm , int keySize) {
	this->m=dim;
	this->a=perm;
	this->keySize=keySize;

}

UniversalHashFunction::~UniversalHashFunction() {
	delete[] a;
}

int UniversalHashFunction::Hash(unsigned char* key){
	int hash = 0;
	for(int i=0;i<keySize;i++){
		int kpart =  int(key[i]) ;
		hash = (hash + kpart * a[i] ) %m;
	}
	return hash ;

}
int* UniversalHashFunction::randomValues(int minValue, int maxValue,unsigned int vectorSize){
	int* a = new int[vectorSize];
	int range=(maxValue-minValue)+1;
	    for(unsigned int index=0; index<vectorSize; index++){
	        a[index] = minValue + int(range*(rand()/(RAND_MAX +1.0)));
	    }
	    return a;
}

UniversalHashFunction* UniversalHashFunction::functionsGenerator(unsigned int tableSize, unsigned int keyLenght, unsigned int numOfFunctions){
	// allocate memory
	int dim = sizeof(UniversalHashFunction);
	UniversalHashFunction* objArray = static_cast<UniversalHashFunction*>( ::operator new ( dim * numOfFunctions ) );
	// invoke constuctors
	for(unsigned int i=0;i<numOfFunctions;i++){
		//generates an array of random integer in the interval [0 , TableSize ]
		int* perm = randomValues(0, tableSize - 1 ,keyLenght);
		new (&objArray[i]) UniversalHashFunction( tableSize, perm , keyLenght);
	}
	return objArray;

}

UniversalHashFunction* UniversalHashFunction::functionGenerator(unsigned int tableSize, unsigned int keyLenght){
	// allocate memory

	int* perm = randomValues(0, tableSize - 1 ,keyLenght);
	UniversalHashFunction* objArray = new UniversalHashFunction(tableSize, perm, keyLenght);

	return objArray;

}


}
