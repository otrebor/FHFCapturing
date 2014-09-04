/*
 * ParallelStages.h
 *
 *  Created on: 21/set/2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#ifndef PARALLELSTAGES_H_
#define PARALLELSTAGES_H_
#include <pcap/pcap.h>
#include <ff/farm.hpp>
#include <stdexcept>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "UniversalHashFunction.h"
#include "DataStructures.h"
#include "MultistageFilter.h"
#include "HashTable.h"

using namespace FHFDataStructures;

namespace FHFFarm {
class LoadBalancer : public ff::ff_loadbalancer {
protected:
	inline int selectworker() { return victim; }
public:
	LoadBalancer(int max_num_workers) : ff::ff_loadbalancer(max_num_workers){ victim = 0; }
	inline void set_victim(int target){
		victim = target;
	}
private:
	int victim;
};


class Emitter : public ff::ff_node {
public:
	Emitter(unsigned int workers, const char* device, bool noPromisc,char* filter_exp,int cnt, unsigned int readTimeout, LoadBalancer * ldb);
	virtual ~Emitter();
	void * svc (void * task);
	int svc_init(void);
	void svc_end(void);
	void stopCapturing();
#ifdef _COMPUTE_STATS
	float getAVGLatency();
	unsigned long int getNumTasksExecuted();
	float getAVGHashLatency();
	void printPcapStats();
#endif
private:
	pcap_t *m_descr;
	LoadBalancer *lb;
	int countPkt;
	unsigned int numWorkers;
	bool offline, end;
	static unsigned int datalinkOffset;
	void handler(const struct pcap_pkthdr *h, const u_char *user);
	static void dispatchCallback(u_char *user, const struct pcap_pkthdr *phdr, const u_char *pdata );
	void dispatchPolicy(Task *t);
#ifdef _COMPUTE_STATS
	unsigned long execTask;
	unsigned long total_time;
	unsigned long total_time_hash;
	time_t start_capturing;
#endif

};


class Worker : public ff::ff_node {
public:
	Worker(uint tableSize, uint keyLen, uint numberOfStages, uint threshold);
	virtual ~Worker();
	void * svc(void * task);
	int svc_init(void);
	void svc_end(void);
	void update_filter_threshold(unsigned long int newThreshold);
	void clean_filter(void);
	void lockWorker();
	void unlockWorker();
	unsigned int getFilterTableSize();
	unsigned int getHashTableSize();
	FHFHashTable::HashTable * getHashTable();
	unsigned long int getThreshold();
#ifdef _COMPUTE_STATS
	float getAVGLatency();
	unsigned long int getNumTasksExecuted();
#endif
private:
	pthread_mutex_t lock ;
	unsigned int tableSize, keyLen, numberOfStages , threshold ;
	FHFMultistageFilter::MultistageFilter * mfilter;
	FHFHashTable::HashTable * ht;
#ifdef _COMPUTE_STATS
	unsigned long execTask;
	unsigned long total_time;
#endif
};

}
#endif /* PARALLELSTAGES_H_ */
