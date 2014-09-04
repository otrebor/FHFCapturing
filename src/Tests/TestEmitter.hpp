/*
 * TestWorker.hpp
 *
 *  Created on: 05/ott/2012
 *      Author: root
 */

#ifndef TESTWORKER_HPP_
#define TESTWORKER_HPP_
namespace testEmitter{
void testEmitter();
void initFlowID(FlowID_t *fid, const char * dstaddr , const char * srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);
void printHashTable(FHFHashTable::HashTable * ht);
void printFlow(FlowDescriptor * it);
void printIP(u_int32_t UIntIP);

}


#endif /* TESTWORKER_HPP_ */
