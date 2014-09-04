/*
 * FilterTest.h
 *
 *  Created on: 27/set/2012
 *      Author: bob
 */

#ifndef FILTERTEST_H_
#define FILTERTEST_H_

#include "../MultistageFilter.h"
#include "../HashTable.h"

namespace FHFcapturing {

void filterTest();
void analyzeFlow(FlowUpdate * fup, FHFHashTable::HashTable * ht , FHFMultistageFilter::MultistageFilter * mfilter);



} /* namespace FHFcapturing */
#endif /* FILTERTEST_H_ */
