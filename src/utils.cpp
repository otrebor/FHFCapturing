/*
 * utils.cpp
 *
 *  Created on: Oct 9, 2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#include "utils.hpp"
#include "DataStructures.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace FHFcapturing {

unsigned int nextPrime(unsigned int num) {
	unsigned int prime = 0, i, nextPrime;
	for (i = 2; i < num / 2; i++) {
		if (num % i == 0) {
			prime = 1;
			break;
		}
	}
	if (prime == 1) {
		nextPrime = num;
		prime = 1;
		while (prime != 0) {
			nextPrime++;
			prime = 0;
			for (i = 2; i < nextPrime / 2; i++) {
				if (nextPrime % i == 0) {
					prime = 1;
					break;
				}
			}

		}
	}
	return nextPrime;
}

void initFlowID(FHFDataStructures::FlowID_t *fid, uint32_t dstaddr, uint32_t srcaddr, uint8_t prot,
		uint16_t dstport, uint16_t srcport) {
	fid->dstaddr = dstaddr;
	fid->srcaddr = srcaddr;
	fid->prot = prot;
	fid->srcport = srcport;
	fid->dstport = dstport;
}

void initFlowID(FHFDataStructures::FlowID_t *fid, const char * dstaddr, const char * srcaddr,
		uint8_t prot, uint16_t dstport, uint16_t srcport) {
	fid->dstaddr = inet_addr(dstaddr);
	fid->srcaddr = inet_addr(srcaddr);
	fid->prot = prot;
	fid->srcport = srcport;
	fid->dstport = dstport;
}

void printHashTable(FHFHashTable::HashTable * t) {
	FHFHashTable::FlowMap * fm = t->getFlowMap();
	std::cout << "======HashMap======" << std::endl;
	// display contents
	int i = 0;
	for (FHFHashTable::FlowMap::const_iterator it = fm->begin();
			it != fm->end(); ++it) {
		std::cout << i++;
		printFlow(it->second);
	}
	std::cout << "========end========" << std::endl;
}

void printFlow(FHFDataStructures::FlowDescriptor * it) {
	std::cout << " [ ID: < DSTA:";
	printIP(it->fid.dstaddr);
	std::cout << ", DSTP:" << it->fid.dstport << ", PROT:"
			<< ((int) it->fid.prot) << ", SRCA:";
	printIP(it->fid.srcaddr);
	std::cout << ", SRCP:" << it->fid.srcport << "> --> oct:" << it->dOctets
			<< " pkt:" << it->dPkts << " ]" << std::endl;
}

void printIP(u_int32_t UIntIP) {
	std::cout << (UIntIP >> 24) << "." << ((UIntIP >> 16) & 0xff) << "." << ((UIntIP >> 8) & 0xff)  << "." << (UIntIP & 0xff) ;

}

}
