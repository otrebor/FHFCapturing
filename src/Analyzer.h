/*
 * Analyzer.h
 *
 *  Created on: 28/set/2012
 *      Author: bob
 */

#ifndef ANALYZER_H_
#define ANALYZER_H_

#include <pthread.h>
#include <vector>
#include <stdexcept>
#include "ParallelStages.h"
#include "Exporter.h"
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <fstream>

namespace FHFcapturing {
using namespace FHFFarm;
using namespace FHFDataStructures;

class Analyzer {
public:
	Analyzer(std::vector<Worker *> * worker_set, Emitter * em , unsigned int measurementInterval, Exporter *exporter);
	virtual ~Analyzer();
	static void * start_thread(void *obj);
	void go(void);
	void stop(void);
	void do_work();
	void stopCapture(void);
#ifdef DEBUG_ANALYZER
	void elab();
	void elab_exit();
#endif
#ifdef _COMPUTE_STATS
	float getAVGLatency();
	unsigned long int getNumTasksExecuted();
	float getAVGWorkerWaitingTime();
#endif
private:
	std::vector<Worker *> * workers;
	Emitter * emitter;
	volatile bool m_stoprequested;
	volatile bool m_running;
	unsigned int intervalCounter;
	unsigned int samplingInterval;//seconds
	unsigned long int first_t_threshold , t_threshold , old_t_threshold;
	unsigned long int r_threshold , old_r_threshold; //early removal threshold t_threshold * r_mul
	unsigned int e1 , e2 , e3; //entry used 3 interval
	unsigned int entriesUsedAVG; //AVG of entries used of last 3 intervals
	unsigned int flowMemSize; //total dimension of HashTables
	unsigned int noVarIntervals; //number of intervals without changes on threshold
	unsigned long int filter_threshold;
	static const float r_mul = 0.2 ; //suggested value
	static const float adjustUp = 3 ;  //suggested value
	static const float adjustDown = 0.5;   //suggested value
	static const float target = 0.90;  //suggested value
pthread_t m_thread;
/*TimeStamp of last Interval*/
time_t startT, stopT;
Exporter * exporter;
#ifdef _COMPUTE_STATS
	unsigned long execTask;
	unsigned long total_time;
	unsigned long mwbt; //mean worker blocked time
#endif

/*collects interesting flow in results*/
//void do_analysis(std::vector<FlowDescriptor> * results);
void do_boundedAnalysis(std::vector<FlowDescriptor> * results);
void update_thresholds(void);
void checkCleanHashTable(FHFHashTable::HashTable *ht, std::vector<FlowDescriptor> * results);
void StopCapture(void);
void output_results(std::vector<FlowDescriptor>  *results);
};

} /* namespace FHFcapturing */
#endif /* ANALYZER_H_ */
