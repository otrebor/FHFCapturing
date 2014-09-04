/*
 * AnalyzerTest.hpp
 *
 *  Created on: 02/ott/2012
 *      Author: bob
 */

#ifndef ANALYZERTEST_HPP_
#define ANALYZERTEST_HPP_

namespace AnalyzerTest{
void AnalyzerTest();
void initFlowID(FlowID_t *fid, const char * dstaddr , const char * srcaddr, uint8_t prot , uint16_t dstport , uint16_t srcport);
void printHashTable(FHFHashTable::HashTable * ht);
void printFlow(FlowDescriptor * it);
void printIP(u_int32_t UIntIP);

}


#endif /* ANALYZERTEST_HPP_ */
