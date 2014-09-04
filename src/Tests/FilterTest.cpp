/*
 * FilterTest.cpp
 *
 *  Created on: 27/set/2012
 *      Author: bob
 */

#include "FilterTest.h"
#include "../MultistageFilter.h"
#include "../DataStructures.h"
#include "HashTableTest.h"

#include <cstdio>
#include <iostream>

namespace FHFcapturing {
using namespace std;

void filterTest() {
	const int tableSize = 100;
	const int threshold = 800;
	const int stages = 3;
	FHFMultistageFilter::MultistageFilter mfilter(tableSize, sizeof(FlowID_t),
			threshold, stages);
	FHFHashTable::HashTable ht(tableSize);

	srand((unsigned) time(0));
	FlowUpdate fup[100];
	unsigned long int rangeIP = numeric_limits<uint32_t>::max()
			- numeric_limits<uint32_t>::min();
	rangeIP++;
	unsigned long int rangePort = (numeric_limits<uint16_t>::max()
			- numeric_limits<uint16_t>::min()) + 1;
	int rangeProto = (numeric_limits<uint8_t>::max()
			- numeric_limits<uint8_t>::min()) + 1;
	for (int index = 0; index < 100; index++) {
		fup[index].fid.dstaddr = uint32_t(
				rangeIP * (rand() / (RAND_MAX + 1.0)));
		fup[index].fid.srcaddr = uint32_t(
				rangeIP * (rand() / (RAND_MAX + 1.0)));
		fup[index].fid.dstport = uint16_t(
				rangePort * (rand() / (RAND_MAX + 1.0)));
		fup[index].fid.srcport = uint16_t(
				rangePort * (rand() / (RAND_MAX + 1.0)));
		fup[index].fid.prot = uint8_t(rangeProto * (rand() / (RAND_MAX + 1.0)));
		fup[index].dOctets = 300;
		analyzeFlow(&fup[index], &ht ,&mfilter);
	}

	printHashTable(&ht);

	//init Testing
	//increment registered flow
	for(int index=0; index<80; index++){
		fup[index].dOctets = 350;
		analyzeFlow(&fup[index], &ht ,&mfilter);
	}

	printHashTable(&ht);

	//increment registered flow
	for(int index=0; index<60; index++){
		fup[index].dOctets = 350;
		analyzeFlow(&fup[index], &ht ,&mfilter);
	}

	printHashTable(&ht);

	//increment registered flow
	for(int index=0; index<30; index++){
		fup[index].dOctets = 550;
		analyzeFlow(&fup[index], &ht ,&mfilter);
	}

	printHashTable(&ht);
	ht.clearTable();



}

void analyzeFlow(FlowUpdate * fup, FHFHashTable::HashTable * ht , FHFMultistageFilter::MultistageFilter * mfilter){
	//shielding
	if(ht->lookupFlow(&(fup->fid))){
		ht->updateTable(fup);
	}else if(mfilter->filter(fup)){
		//pass the filter
		ht->insertFlow(fup);
	}
}

} /* namespace FHFcapturing */
