/*
 * HashTableTest.cpp
 *
 *  Created on: 27/set/2012
 *      Author: bob
 */
#include <iostream>
#include <cstdio>
#include "../HashTable.h"
#include "../DataStructures.h"
#include "HashTableTest.h"


using namespace std;

void hashTableTest(){
		FlowID_t id1;
		initFlowID(&id1, "192.168.0.1", "192.168.0.2", 7 , 1024 , 800);


		FlowUpdate fup1(id1);
		fup1.dOctets = 30;

		FlowID_t id2;
		initFlowID(&id1, "192.168.10.1", "192.168.10.2", 16 , 1023 , 801);

		FlowUpdate fup2(id2);
		fup2.dOctets=40;

		FHFHashTable::HashTable ht(100);
		ht.insertFlow(&fup1);

		if(ht.lookupFlow(&id1)){
			std::cout << "id1 aggiunto e trovato" << endl;
		}

		if(!ht.lookupFlow(&id2)){
				std::cout << "id2 non trovato!" << endl;
		}
		printHashTable(&ht);

		ht.insertFlow(&fup2);

		if(ht.lookupFlow(&id2)){
					std::cout << "id2 aggiunto e trovato trovato!" << endl;
					fup2.dOctets = 50;
					ht.updateTable(&fup2);
		}

		printHashTable(&ht);
		cout << "clear table" << endl;
		ht.clearTable();
		printHashTable(&ht);

		cout << "\n MORE INTENSIVE TESTS \n " << endl ;
		srand((unsigned)time(0));
		FlowUpdate fup[100];
		unsigned long int rangeIP = numeric_limits<uint32_t>::max() - numeric_limits<uint32_t>::min();
		rangeIP++;
		unsigned long int rangePort = (numeric_limits<uint16_t>::max()-numeric_limits<uint16_t>::min())+1;
		int rangeProto = (numeric_limits<uint8_t>::max()-numeric_limits<uint8_t>::min())+1;
		for(int index=0; index<100; index++){
			        fup[index].fid.dstaddr = uint32_t(rangeIP*(rand()/(RAND_MAX +1.0)));
			        fup[index].fid.srcaddr = uint32_t(rangeIP*(rand()/(RAND_MAX +1.0)));
			        fup[index].fid.dstport = uint16_t(rangePort*(rand()/(RAND_MAX +1.0)));
			        fup[index].fid.srcport = uint16_t(rangePort*(rand()/(RAND_MAX +1.0)));
			        fup[index].fid.prot = uint8_t(rangeProto*(rand()/(RAND_MAX +1.0)));
			        fup[index].dOctets = 50;
			        ht.insertFlow(&fup[index]);
		}

		printHashTable(&ht);

		//increment registered flow
		for(int index=0; index<60; index++){
			fup[index].dOctets = 100;
			ht.updateTable(&fup[index]);
		}

		printHashTable(&ht);

		//increment registered flow
		for(int index=0; index<30; index++){
			fup[index].dOctets = 200;
			ht.updateTable(&fup[index]);
		}

		printHashTable(&ht);

}

void initFlowID(FlowID_t *fid, uint32_t dstaddr , uint32_t srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport){
	fid->dstaddr=dstaddr;
	fid->srcaddr=srcaddr;
	fid->prot=prot;
	fid->srcport=srcport;
	fid->dstport=dstport;
}

void initFlowID(FlowID_t *fid, const char * dstaddr , const char * srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport){
	fid->dstaddr=inet_addr(dstaddr);
	fid->srcaddr=inet_addr(srcaddr);
	fid->prot=prot;
	fid->srcport=srcport;
	fid->dstport=dstport;
}

void printHashTable(FHFHashTable::HashTable * ht){
	FHFHashTable::FlowMap * fm = ht->getFlowMap();
	std::cout << "======HashMap======" << endl;
	// display contents
	int i = 0;
	    for (FHFHashTable::FlowMap::const_iterator it = fm->begin(); it != fm->end(); ++it){
	        cout<<i++;
	    	printFlow(it->second);
	    }
	std::cout << "========end========" << endl;
}

void printFlow(FlowDescriptor * it){
	std::cout << " [ ID: < DSTA:" ;
	printIP(it->fid.dstaddr);
	std::cout << ", DSTP:" << it->fid.dstport << ", PROT:" << ((int)it->fid.prot) << ", SRCA:" ;
	printIP(it->fid.srcaddr);
	std::cout << ", SRCP:" << it->fid.srcport << "> --> oct:"<< it->dOctets << " pkt:" << it->dPkts <<" ]"<< endl;
}

void printIP(u_int32_t UIntIP){
    std::cout << (UIntIP & 0xff) << "." << ((UIntIP >> 8) & 0xff) << "." << ((UIntIP >> 16) & 0xff) << "." << (UIntIP >> 24);
}
