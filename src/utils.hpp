/*
 * utils.hpp
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

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <stdio.h>
#include "DataStructures.h"
#include "HashTable.h"

namespace FHFcapturing{

unsigned int nextPrime(unsigned int num);

void initFlowID(FHFDataStructures::FlowID_t *fid, uint32_t dstaddr , uint32_t srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);
void initFlowID(FHFDataStructures::FlowID_t *fid, const char * dstaddr , const char * srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);
void printHashTable(FHFHashTable::HashTable *t);
void printFlow(FHFDataStructures::FlowDescriptor * it);
void printIP(u_int32_t UIntIP);


}

#endif /* UTILS_HPP_ */
