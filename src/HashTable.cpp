/*
 * HashTable.cpp
 *
 *  Created on: 25/set/2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#include "HashTable.h"


namespace FHFHashTable {

HashTable::HashTable(unsigned int tableSize):tableSize(tableSize) {
	fmap = new FlowMap(tableSize);
}

HashTable::~HashTable() {
	clearTable();
	delete fmap;
}

bool HashTable::lookupFlow(const FlowID_t * fid){
	// try to find the id
	if(fmap->find(*fid) != fmap->end()){
		return true;
	}
	return false;
}
void HashTable::updateTable(FlowUpdate * fup){
	FlowID_t * fid = &(fup->fid);
	for (FlowMap::const_iterator it = fmap->find(*fid); it != fmap->end(); ++it){
			//object found we will add the size of the pkt
			(*(it->second)).incrementd_0ctects(fup->dOctets);
			(*(it->second)).increment_dPkts();
			return;
		}
	throw std::invalid_argument("Updating value of non stored Flow");
}

void HashTable::insertFlow(FlowUpdate * fup){
	FlowDescriptor * fd = new FlowDescriptor;
	//adding the size of the first pkt
	fd->dOctets = fup->dOctets;
	//adding a copy of fid
	fd->fid = fup->fid;
	fd->increment_dPkts();
	fmap->insert(FlowMap::value_type(fd->fid,fd));
}

void HashTable::clearTable(void){
	for (FlowMap::const_iterator it = fmap->begin(); it != fmap->end(); ++it){
		     delete it->second;
	}
	fmap->clear();
}

FlowMap * HashTable::getFlowMap(void){
	return fmap;
}

unsigned int HashTable::getNumOfRecords(){
	return fmap->size();
}

unsigned int HashTable::getTableMaxSize(){
	return tableSize;
}
} /* namespace HashTable */
