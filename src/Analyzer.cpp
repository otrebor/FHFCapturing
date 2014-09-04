/*
 * Analyzer.cpp
 *
 *  Created on: 28/set/2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#include "Analyzer.h"
#include "utils.hpp"
#include <iostream>
#include <fstream>

#ifndef __USE_LINUX
#include <unistd.h>
#endif

namespace FHFcapturing {

extern unsigned long int linkCapacity;
extern double percentageInterestingFlow;

Analyzer::Analyzer(std::vector<Worker *> * worker_set, Emitter * em,
		 unsigned int measurementInterval, Exporter *exporter) {
	if (worker_set == NULL || em == NULL || worker_set->size() == 0
			|| worker_set->at(0)->getHashTableSize() == 0 || exporter == NULL) {
		throw std::invalid_argument("invalid Arguments at Analyzer init!");
	}
#ifdef _COMPUTE_STATS
		execTask=total_time=0;
		mwbt=0;
#endif
	samplingInterval = measurementInterval;
	workers = worker_set;
	emitter = em;
	m_stoprequested = false;
	m_running = false;
	noVarIntervals = 0;
	entriesUsedAVG = 0;
	intervalCounter=0;
#ifdef _AVG_CASE_BALANCING
	//TODO: check if it could be a legal choice
	flowMemSize = worker_set->at(0)->getHashTableSize()*worker_set->size()  ;
#else
	/*Each worker has an hash table able to record all the interesting flow
	 * potential unbalancing between workers */
	flowMemSize = worker_set->at(0)->getHashTableSize();
#endif
	/*trick to avoid threshold decreasing on first interval*/
	e1 = e2 = e3 = flowMemSize ;
	first_t_threshold = t_threshold = old_t_threshold = worker_set->at(0)->getThreshold();
	r_threshold = old_r_threshold = t_threshold * r_mul;
	//filters threshold differs from the table threshold by an oversampling factor set directly by filter
	filter_threshold = t_threshold;
	m_thread=0;
	startT=stopT=0;
	this->exporter=exporter;
}

Analyzer::~Analyzer() {
	// TODO Auto-generated destructor stub
	//is delegated to workers to destroy DS
}

void *Analyzer::start_thread(void *obj) {
	//All we do here is call the do_work() function
	reinterpret_cast<Analyzer *>(obj)->do_work();
	return NULL;
}

void Analyzer::go() {
	assert(m_running == false);
	m_running = true;
	/*open output exporter*/
	exporter->open();
	pthread_create(&m_thread, 0, &(Analyzer::start_thread), this);
}

void Analyzer::stop()
{
	assert(m_running == true);
	m_running = false;
	m_stoprequested = true;
	pthread_join(m_thread, 0);
	/*close output exporter*/
	exporter->close();
#ifdef DEBUG_ANALYZER
		std::cout << "JOIN ANALYZER \n"<< std::endl ;
#endif
}

void Analyzer::do_work() {
	int sigRec;
	struct timespec timeout;
	sigset_t signal_set_gest;
	if (sigemptyset(&signal_set_gest) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	if (sigaddset(&signal_set_gest, SIGINT ) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	if (sigaddset(&signal_set_gest, SIGTERM) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
#ifndef __USE_LINUX
	if (sigaddset(&signal_set_gest, SIGALRM) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
#else
	siginfo_t info;
#endif
	/**set timeout for timedwait*/
	timeout.tv_sec = samplingInterval; /* seconds  */
	timeout.tv_nsec = 0; /* nanosecondi */

	/*Could be a benefit on performance to change dataStructure.
	 * we need one where is possible to set initial capacity
	 * that is 1/percentageOfInterest +  (1/percentageOfInterest)*oversamplingFactor */
	std::vector<FlowDescriptor> * results;
#ifdef DEBUG_ANALYZER
		std::cout << "ANALYZER : entering loop "<< std::endl ;
#endif

	while (!m_stoprequested) {
		/*is preferred to change data structure if it is too slow.
		 * there are no constructors that set the initial capacity without
		 * Instantiate all the objs */
		results = new std::vector<FlowDescriptor>;
#ifdef __USE_LINUX
		startT=time(NULL);
		sigRec = sigtimedwait(&signal_set_gest, &info, &timeout);
		stopT=time(NULL);
#else
		alarm(samplingInterval);
		startT=time(NULL);
		if (sigwait(&signal_set_gest, &sigRec) != 0) {
			perror("sigwait error");
			continue;
		}
		stopT=time(NULL);
#endif
#ifdef _COMPUTE_STATS
		unsigned long t1=ff::getusec();
#endif

		switch (sigRec) {
#ifdef __USE_LINUX

		case -1:
		if (errno != EAGAIN) {
			/**TIMER ERROR*/
			perror("sigtimedwait fallita");
			continue;
#ifndef DEBUG_ANALYZER
			exit(-1);
#endif
		} else {
		/**TIME IS UP, ANALYSIS*/
#ifdef DEBUG_ANALYZER
		std::cout << "ANALYZER : Time is up"<< std::endl ;
#endif
		do_boundedAnalysis(results);
		output_results(results);
		intervalCounter++;
		delete results;
		}
		break;
#else
		case SIGALRM:
			/**TIME IS UP, ANALYSIS*/
			do_boundedAnalysis(results);
			output_results(results);
			intervalCounter++;
			delete results;
			break;
#endif
		case SIGINT:
		case SIGTERM: /** CLOSE  */
#ifdef DEBUG_ANALYZER
		std::cout << "ANALYZER : SIGINT/TERM "<< std::endl ;
#endif
			stopCapture();
			do_boundedAnalysis(results);
			output_results(results);
			delete results;
			m_stoprequested = true;
			break;
		default:
			perror("SIGNAL NOT HANDLED");
			exit(-1);
			break;
		}
#ifdef _COMPUTE_STATS
			++execTask;
			total_time+=(ff::getusec()-t1);
#endif
	}
#ifdef DEBUG_ANALYZER
		std::cout << "ANALYZER : EXITING.\n"<< std::endl ;
#endif
}

void Analyzer::do_boundedAnalysis(std::vector<FlowDescriptor> * results) {
	e3 = e2;
	e2 = e1;
	e1 = 0;
	/*Block All the Workers*/
#ifdef _COMPUTE_STATS
	unsigned long t1=ff::getusec();
#endif
	for (unsigned int i = 0; i < workers->size(); i++) {
		workers->at(i)->lockWorker();
	}
	//originally the calculation of threshold was before the analysis trick
	old_t_threshold = t_threshold;
	for (unsigned int i = 0; i < workers->size(); i++) {
	/*check, cleanup & collect result in results*/
	checkCleanHashTable(workers->at(i)->getHashTable(), results);
	}

	/*Collect Stats on table's population*/
	e1 += results->size();
	/*we have to update the AVG of last 3 intervals*/
	entriesUsedAVG = ((e1 + e2 + e3) / 3) + 1;
	/* Updating thresholds*/
	update_thresholds();

	for (unsigned int i = 0; i < workers->size(); i++) {
	/*Update the filter threshold on this worker*/
	workers->at(i)->update_filter_threshold(filter_threshold);
	/*clean all the values of the counters inside the filter*/
	workers->at(i)->clean_filter();
	}
	for (unsigned int i = 0; i < workers->size(); i++) {
		workers->at(i)->unlockWorker();
	}
#ifdef _COMPUTE_STATS
	t1=( ff::getusec()- t1);
	mwbt+=(t1/workers->size())+t1;
#endif

}

/*
 *
 * void Analyzer::do_analysis(std::vector<FlowDescriptor> * results) {
	e3 = e2;
	e2 = e1;
	e1 = 0;
	//Block All the Workers
#ifdef _COMPUTE_STATS
	unsigned long t1=ff::getusec();
#endif
	for (unsigned int i = 0; i < workers->size(); i++) {
		workers->at(i)->lockWorker();
	}
	Collect Stats
	for (unsigned int i = 0; i < workers->size(); i++) {
		e1 += workers->at(i)->getHashTable()->getNumOfRecords();
	}
	for (unsigned int i = 0; i < workers->size(); i++) {
		workers->at(i)->unlockWorker();
	}
#ifdef _COMPUTE_STATS
	t1=( ff::getusec()- t1);
	unsigned long t2=0, sum=0;
#endif
	we have to update the AVG of last 3 intervals
	entriesUsedAVG = ((e1 + e2 + e3) / 3) + 1;
	 Updating thresholds
	update_thresholds();
	not precise but i think that could be too slow to block and check all the workers together
	for (unsigned int i = 0; i < workers->size(); i++) {
		workers->at(i)->lockWorker();
#ifdef _COMPUTE_STATS
	t2=ff::getusec();
#endif
		check, cleanup & collect result in results
		checkCleanHashTable(workers->at(i)->getHashTable(), results);
		Update the filter threshold on this worker
		workers->at(i)->update_filter_threshold(filter_threshold);
		clean all the values of the counters inside the filter
		workers->at(i)->clean_filter();
		workers->at(i)->unlockWorker();
#ifdef _COMPUTE_STATS
	t2=ff::getusec()-t2;
	sum+=t2;
#endif
	}
#ifdef _COMPUTE_STATS
	mwbt+=(sum/workers->size())+t1;
#endif

}
*/
void Analyzer::update_thresholds() {
#ifdef DEBUG_ANALYZER
	std::cout << "STATS: old_t_Thresh: " << t_threshold ;
#endif
	old_t_threshold = t_threshold;
	old_r_threshold = r_threshold;
	/*
	 ADAPTTHRESHOLD
	 usage = entriesused/flowmemsize
	 if (usage > target)
	 threshold = threshold ∗ (usage/target)^adjustup
	 else if (threshold did not increase for 3 intervals)
	 threshold = threshold ∗ (usage/target)^adjustdown
	 endif
	 endif
	 */
	double usage = (float)entriesUsedAVG / (float)flowMemSize;
	if (usage > target) {
		t_threshold = t_threshold * pow((usage / target), adjustUp);
#ifdef DEBUG_ANALYZER
	std::cout << " mulUp: " << pow((usage / target), adjustUp) ;
#endif
		if(t_threshold==0) t_threshold=1; //avoid 0 multiplication
		if(t_threshold > first_t_threshold) t_threshold=first_t_threshold; //limit the increasing of the threshold
		noVarIntervals = 0;
	} else if (noVarIntervals >= 3) {
		noVarIntervals = 0;
		t_threshold = t_threshold * pow((usage * target), adjustDown);
#ifdef DEBUG_ANALYZER
	std::cout << " mulDown: " << pow((usage * target), adjustUp) ;
#endif
	if(t_threshold==0) t_threshold=1; //avoid 0 multiplication
	if(t_threshold > first_t_threshold) t_threshold=first_t_threshold; //limit the increasing of the threshold
		r_threshold = t_threshold * r_mul;
	} else {
		noVarIntervals++;
	}
	// updating filters thresholds
	filter_threshold = t_threshold ;
#ifdef DEBUG_ANALYZER
	std::cout << "\n new_t_Thresh: " << t_threshold << " Usage: " << usage << " Target:" << target << std::endl ;
	std::cout << "	entriesUsedAVG: " << entriesUsedAVG << " flowMemSize: " << flowMemSize << std::endl ;
#endif
}

void Analyzer::checkCleanHashTable(FHFHashTable::HashTable * ht,
		std::vector<FlowDescriptor> * results) {
	FHFHashTable::FlowMap * fmap = ht->getFlowMap();
	FHFHashTable::FlowMap::const_iterator it = fmap->begin();
		while(it!=fmap->end()){
		if (it->second->cycles == 0 && it->second->dOctets > old_r_threshold
				&& it->second->dOctets <= old_t_threshold) {
			it->second->cycles++;
			it->second->dOctets=0;
#ifdef DEBUG_ANALYZER_CHECK
			std::cout << "ANALYZER: entry > r_th :" ;
			printFlow(it->second);
			std::cout << std::endl;
#endif
			++it;
			/*in this case the flow was added to the table in this time interval
			 * we have to leave it in (Early Removal)*/
		} else if (it->second->cycles >= 0 && it->second->dOctets > old_t_threshold) {
			//collect all the flow that are over the threshold and leave it on the table
			results->push_back(*(it->second));
			/*Preserving entries*/
			it->second->cycles++;
			/*i have to erase the value of last time interval*/
			it->second->dOctets=0;
#ifdef DEBUG_ANALYZER_CHECK
			std::cout << "ANALYZER: entry > t_th :" ;
			printFlow(it->second);
			std::cout << std::endl;
#endif
			++it;
		} else {
			/*delete entry*/
#ifdef DEBUG_ANALYZER_CHECK
			std::cout << "ANALYZER: delete entry :"  ;
			printFlow(it->second);
			std::cout << std::endl;
#endif
			delete it->second;
			it=fmap->erase(it);

		}
	}
}

void Analyzer::stopCapture(void) {
	emitter->stopCapturing();
}

void Analyzer::output_results(std::vector<FlowDescriptor>  *results) {
	/* passing to the exporter all the results and an interval description*/
	IntervalDescriptor * it = new IntervalDescriptor(intervalCounter, startT ,stopT, old_t_threshold, old_r_threshold);
	exporter->exportFlow(results,it);
	delete it;
#ifdef DEBUG_ANALYZER
			std::cout << "ANALYZER: result size :" << results->size() << std::endl << std::endl;
#endif

}


#ifdef _COMPUTE_STATS
	float Analyzer::getAVGLatency(){
		return (execTask!=0)?(float)total_time/(float)execTask:-1.0;
	}
	unsigned long int Analyzer::getNumTasksExecuted(){
		return execTask;
	}
	float Analyzer::getAVGWorkerWaitingTime(){
		return (execTask!=0)?(float)mwbt/(float)execTask:-1.0;
	}
#endif
#ifdef DEBUG_ANALYZER
void Analyzer::elab(){
	std::vector<FlowDescriptor> * results;
	unsigned int dim = (linkCapacity * percentageInterestingFlow) + 1;
	results = new std::vector<FlowDescriptor>(dim);
	do_boundedAnalysis(results);
	output_results(results);
	intervalCounter++;
	delete results;
}

void Analyzer::elab_exit(){
	std::vector<FlowDescriptor> * results;
	unsigned int dim = (linkCapacity * percentageInterestingFlow) + 1;
	results = new std::vector<FlowDescriptor>(dim);
	stopCapture();
	do_boundedAnalysis(results);
	output_results(results);
	delete results;
	m_stoprequested = true;
}

#endif

} /* namespace FHFcapturing */
