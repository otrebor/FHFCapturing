/*
 * TestHashing.h
 *
 *  Created on: 27/set/2012
 *      Author: bob
 */

#ifndef HASHTABLETEST_H_
#define HASHTABLETEST_H_

#include <iostream>
#include "../UniversalHashFunction.h"
#include <ff/farm.hpp>
#include "../ParallelStages.h"
#include "../HashTable.h"

void hashTableTest();
void printFlow(FlowDescriptor * it);
void printIP(u_int32_t UIntIP);
void printHashTable(FHFHashTable::HashTable * ht);
void initFlowID(FlowID_t *fid, const char * dstaddr , const char * srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);
void initFlowID(FlowID_t *fid, uint32_t dstaddr , uint32_t srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);



#endif /* TESTHASHING_H_ */
