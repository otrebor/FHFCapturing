/*
 * UniversalHashFunction.h
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

#ifndef UNIVERSALHASHFUNCTION_H_
#define UNIVERSALHASHFUNCTION_H_

#include <vector>
namespace FHFMultistageFilter {
class UniversalHashFunction {
private:
	int m;
	int* a;
	int keySize;//bytes
	static int* randomValues(int minValue, int maxValue,unsigned int vectorSize);
public:
	UniversalHashFunction(int dim , int* perm , int keySize);
	virtual ~UniversalHashFunction();
	int Hash(unsigned char* key);
	/**
	 *
	 * \param tableSize must be a prime number.
	 * \param keyLenght lenght in byte of keys.
	 * \param numOfFunctions number of different hashing functions needed.
	 * returns an array of universalHashingFunction
	 */
	static UniversalHashFunction * functionsGenerator(unsigned int tableSize, unsigned int keyLenght, unsigned int numOfFunctions );
	static UniversalHashFunction* functionGenerator(unsigned int tableSize, unsigned int keyLenght);
};
}
#endif /* UNIVERSALHASHFUNCTION_H_ */
