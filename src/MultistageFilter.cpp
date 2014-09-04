/*
 * MultistageFilter.cpp
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
#include "MultistageFilter.h"
#include <stdexcept>

namespace FHFMultistageFilter {

/**
 *
 * STAGE CONTAINER
 *
 * */
StageContainer::StageContainer(){
	countArray=NULL;
	hfun=NULL;
	threshold=stageSize=keyLenght=0;
};

StageContainer::StageContainer(unsigned int stageSize, unsigned int keyLenght, unsigned long int threshold) {
init(stageSize,keyLenght,threshold);

}

void StageContainer::init(unsigned int stageSize, unsigned int keyLenght, unsigned long int threshold){
	if(stageSize==0 || keyLenght ==0){
		throw std::invalid_argument("invalid Arguments on StageContainer init!");
	}
	this->stageSize=stageSize;
	this->keyLenght=keyLenght;
	this->hfun=UniversalHashFunction::functionGenerator(stageSize,keyLenght);
	this->countArray=new unsigned long int[stageSize];
	for(unsigned int i=0 ;i < stageSize; i++ ){
		countArray[i]=0;
	}
	this->threshold=threshold*filter_oversampling_factor;
}

StageContainer::~StageContainer() {
	delete hfun;
	delete[] countArray;
}

bool StageContainer::check(const FlowUpdate * fup,unsigned long int *fieldValue){
	/*hashing the flow id*/
	int index = hfun->Hash((unsigned char * )&(fup->fid));
	/*retrieve the counter calue*/
	*fieldValue=countArray[index];
	/*if the new value will be higher than the threshold return true*/
	return ((*fieldValue + fup->dOctets)>threshold);
}

/*update the counter value related to a specific flow*/
//TODO: evaluate the overflow time
void StageContainer::conservativeUpdate(FlowUpdate * fup, double max){
	int index = hfun->Hash((unsigned char * ) &(fup->fid));
	if(countArray[index]<threshold){
		/*newVal = max if (oldVal+increment > max) */
		unsigned long int val = countArray[index]+fup->dOctets ;
		countArray[index]=(val > max) ? max : val;
	}
	/*no increment if oldVal > max*/
}

/*reset the internal state of the counters */
void StageContainer::reset(){
	for(unsigned int i =0; i<stageSize;i++){
		countArray[i]=0;
	}
}
/*update the value of the threshold used by the stage*/
void StageContainer::updateThreshold(unsigned long int newThreshold){
	unsigned long int rth = (float)newThreshold*filter_oversampling_factor;
	threshold=(rth==0)?rth:1;
}

unsigned int StageContainer::getStageSize(){
	return stageSize;
}


/**
 * MULTISTAGEFILTER
 * */
MultistageFilter::MultistageFilter(unsigned int stageSize,unsigned int keyLenght , unsigned long int threshold, unsigned short int numOfStages){
	if(numOfStages==0){
			throw std::invalid_argument("invalid number of Stages! (cannot be 0) ");
		}
	stages = new StageContainer[numOfStages];
	stagesNumber=numOfStages;
	for(unsigned int i=0;i<stagesNumber;i++){
		stages[i].init(stageSize,keyLenght,threshold);
	}
}

MultistageFilter::~MultistageFilter(){
	delete[] stages;
}

void MultistageFilter::updateThreshold(unsigned long int newThreshold){
	for(unsigned int i = 0; i<stagesNumber ; i++){
			stages[i].updateThreshold(newThreshold);
		}
}

/*evaluate if the flow pass the filter and updates the filter state (conservative updates)*/
bool MultistageFilter::filter(FlowUpdate * fup){
	//check if the update pass each filter and collect the minimum value of each stage
	unsigned long int minVal=std::numeric_limits<unsigned long int>::max(), parVal=0;
	bool pass = true;
	for(unsigned int i = 0; i<stagesNumber ; i++){
		pass= pass && stages[i].check(fup,&parVal);
		/*find the minimum value of each counter*/
		minVal = (minVal<parVal)?minVal:parVal;
	}
	if(!pass){
		/*Only in the case of filter drop we must update the filter*/
		/*conservative updates impose that parVal (min + size) is the maximum value for the increment of counters
		 * each counter over this value must not be incremented */
		parVal=minVal+fup->dOctets;
		for(unsigned int i = 0; i<stagesNumber ; i++){
				stages[i].conservativeUpdate(fup,parVal);
		}
	}
	return pass;
}
/*reset filter's counters*/
void MultistageFilter::reset(){
	for(unsigned int i = 0; i<stagesNumber ; i++){
		stages[i].reset();
	}
}

unsigned int MultistageFilter::getTablesSize(){
	return stages[0].getStageSize();
}

} /* namespace MultistageFilter */
