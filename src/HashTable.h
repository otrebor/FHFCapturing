/*
 * HashTable.h
 *
 *  Created on: 25/set/2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 *
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <boost/unordered_map.hpp>
#include <stdexcept>
#include "MultistageFilter.h"
#include "UniversalHashFunction.h"
#include "DataStructures.h"

using namespace FHFDataStructures;

namespace FHFHashTable {

typedef boost::unordered_map<FlowID_t, FlowDescriptor *, ihash , iequal_to > FlowMap;

class HashTable {

private:
	FlowMap * fmap;
	unsigned int tableSize ;
public:
	HashTable(unsigned int tableSize);
	virtual ~HashTable();
	bool lookupFlow(const FlowID_t * fid);
	void updateTable(FlowUpdate * fup);
	void insertFlow(FlowUpdate * fup);
	void clearTable();
	FlowMap * getFlowMap(void);
	unsigned int getNumOfRecords();
	unsigned int getTableMaxSize();
};

} /* namespace HashTable */
#endif /* HASHTABLE_H_ */
