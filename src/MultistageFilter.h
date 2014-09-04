/*
 * StageContainer.h
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

#ifndef MULTISTAGEFILTER_H_
#define MULTISTAGEFILTER_H_

#include "UniversalHashFunction.h"
#include "DataStructures.h"

using namespace FHFDataStructures;

namespace FHFMultistageFilter {



class StageContainer {
private:
	UniversalHashFunction *hfun;
	unsigned int stageSize, keyLenght;
	unsigned long int threshold;
	static const float filter_oversampling_factor = 100 ;
	unsigned long int *countArray;
public:
	StageContainer();
	StageContainer(unsigned int stageSize, unsigned int keyLenght , unsigned long int threshold);
	void init(unsigned int stageSize, unsigned int keyLenght, unsigned long int threshold);
	virtual ~StageContainer();
	/*check if a specific flow pass the filter*/
	bool check(const FlowUpdate * fup,unsigned long int *fieldValue);
	/*update the counter value related to a specific flow*/
	//TODO: evaluate the overflow time
	void conservativeUpdate(FlowUpdate * fup, double max);
	/*reset the internal state of the counters */
	void reset();
	/*update the value of the threshold used by the stage*/
	void updateThreshold(unsigned long int newThreshold);
	unsigned int getStageSize();
};

class MultistageFilter {
private:
	StageContainer * stages;
	unsigned int stagesNumber;
public:
	MultistageFilter(unsigned int stageSize,unsigned int keyLenght , unsigned long int threshold, unsigned short int numOfStages);
	virtual ~MultistageFilter();
	void updateThreshold(unsigned long int newThreshold);
	/*evaluate if the flow pass the filter and updates the filter state*/
	bool filter(FlowUpdate * fup);
	/*reset filter's counters*/
	void reset();
	unsigned int getTablesSize();
};


} /* namespace MultistageFilter */
#endif /* MULTISTAGEFILTER_H_ */
