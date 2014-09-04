/*
 * TestWorker.cpp
 *
 *  Created on: 05/ott/2012
 *      Author: root
 */

#include <iostream>
#include <cstdio>
#include "../HashTable.h"
#include "../DataStructures.h"
#include "HashTableTest.h"
#include "AnalyzerTest.hpp"
#include "../Analyzer.h"
#include "TestEmitter.hpp"

using namespace std;
using namespace FHFcapturing;

namespace testEmitter{

void testEmitter(){

	Worker * worker = new Worker(200, 13 , 3 , 800);


		cout << "\n Emitter TESTS \n " << endl ;
		ff::ff_farm<LoadBalancer> farm;
		srand((unsigned)time(0));
		Emitter pktDispatcher(1, "eth0", true , NULL, 0, 0, farm.getlb() );

		Task fup[100];
		unsigned long int rangeIP = numeric_limits<uint32_t>::max() - numeric_limits<uint32_t>::min();
		rangeIP++;
		unsigned long int rangePort = (numeric_limits<uint16_t>::max()-numeric_limits<uint16_t>::min())+1;
		int rangeProto = (numeric_limits<uint8_t>::max()-numeric_limits<uint8_t>::min())+1;
		for(int index=0; index<100; index++){
			        fup[index].fd->fid.dstaddr = uint32_t(rangeIP*(rand()/(RAND_MAX +1.0)));
			        fup[index].fd->fid.srcaddr = uint32_t(rangeIP*(rand()/(RAND_MAX +1.0)));
			        fup[index].fd->fid.dstport = uint16_t(rangePort*(rand()/(RAND_MAX +1.0)));
			        fup[index].fd->fid.srcport = uint16_t(rangePort*(rand()/(RAND_MAX +1.0)));
			        fup[index].fd->fid.prot = uint8_t(rangeProto*(rand()/(RAND_MAX +1.0)));
			        fup[index].fd->dOctets = 50;
			        worker->svc((void *)&fup[index]);
		}

		printHashTable(worker->getHashTable());

		//increment registered flow
		for(int index=0; index<60; index++){
			fup[index].fd->dOctets = 700;
			worker->svc((void *)&fup[index]);
		}

		printHashTable(worker->getHashTable());

		//increment registered flow
		for(int index=0; index<30; index++){
			fup[index].fd->dOctets = 800;
			worker->svc((void *)&fup[index]);
		}
		//questi dovrebbero passare il filtro

		//questi passano l'analizzatore < R
			for(int index=0; index<30; index++){
				fup[index].fd->dOctets = 900;
				worker->svc((void *)&fup[index]);
			}

		printHashTable(worker->getHashTable());


		//questi passano l'analizzatore < R
			for(int index=0; index<30; index++){
				fup[index].fd->dOctets = 900;
				worker->svc((void *)&fup[index]);
			}

		printHashTable(worker->getHashTable());


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
	    	testEmitter::printFlow(it->second);
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

}


