/*
 * AnalyzerTest.cpp
 *
 *  Created on: 01/ott/2012
 *      Author: bob
 */

#include <iostream>
#include <cstdio>
#include "../HashTable.h"
#include "../DataStructures.h"
#include "HashTableTest.h"
#include "AnalyzerTest.hpp"
#include "../Analyzer.h"

//#define DEBUG_ANALYZER
using namespace FHFFarm;
using namespace std;

namespace AnalyzerTest{

void AnalyzerTest(){
		vector<Worker *> workers;

		for(unsigned int i =0 ; i<1;++i){
			Worker * wr = new Worker(200, 13 , 3 , 800);
			workers.push_back(wr);
		}

		cout << "\n ANALYZER TESTS \n " << endl ;
		srand((unsigned)time(0));
		ff::ff_farm<LoadBalancer> farm;
		Emitter em(1, "eth2", false , NULL, -1 , 30, farm.getlb() );
		unsigned int uno = 1;
		FHFcapturing::FileExporter *fex=new FHFcapturing::FileExporter("TestAnalyzer.txt");
		FHFcapturing::Analyzer ana(&workers, &em, uno, fex);

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
			        workers[0]->svc((void *)&fup[index]);
		}

		printHashTable(workers[0]->getHashTable());

		//increment registered flow
		for(int index=0; index<60; index++){
			fup[index].fd->dOctets = 700;
			workers[0]->svc((void *)&fup[index]);
		}

		printHashTable(workers[0]->getHashTable());

		//increment registered flow
		for(int index=0; index<30; index++){
			fup[index].fd->dOctets = 800;
			workers[0]->svc((void *)&fup[index]);
		}
		//questi dovrebbero passare il filtro

		//questi passano l'analizzatore < R
			for(int index=0; index<30; index++){
				fup[index].fd->dOctets = 900;
				workers[0]->svc((void *)&fup[index]);
			}

		printHashTable(workers[0]->getHashTable());
#ifdef DEBUG_ANALYZER
		ana.elab();
#endif

		//questi passano l'analizzatore < R
			for(int index=0; index<30; index++){
				fup[index].fd->dOctets = 900;
				workers[0]->svc((void *)&fup[index]);
			}

		printHashTable(workers[0]->getHashTable());
#ifdef DEBUG_ANALYZER
		ana.elab();
#endif
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
	    	AnalyzerTest::printFlow(it->second);
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

