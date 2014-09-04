/*
 * ParallelStages.cpp
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

#include "ParallelStages.h"
#include "utils.hpp"
#include <pcap/pcap.h>
#include <boost/functional/hash.hpp>
#include <signal.h>
#include <pthread.h>

/*workaround (avoid "Symbol 'PCAP_ERRBUF_SIZE' could not be resolved on OSX")*/
#ifndef PCAP_ERRBUF_SIZE
#define PCAP_ERRBUF_SIZE 256 //value taken from pcap/pcap.h
#endif

namespace FHFFarm {
/*
 * Class Emitter
 */
//I must init this static variable in order to avoid linker errors
unsigned int Emitter::datalinkOffset = 0;

Emitter::Emitter(unsigned int workers, const char* device, bool noPromisc,
		char* filter_exp, int cnt, unsigned int readTimeout,
		LoadBalancer * ldb) {
//init
	this->end = false;
	this->countPkt = cnt;
#ifdef _COMPUTE_STATS
	execTask = total_time = total_time_hash = 0;
	start_capturing=0;
#endif
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp; /*filter*/
	bpf_u_int32 mask, addr; /*netmask & addr of our sniffing device*/

	//check Parameters
	numWorkers = (workers != 0) ? workers : 1;
	//check LoadBalancer
	if (ldb == NULL) {
		fprintf(stderr, "LoadBalancer not specified");
		exit(-1);
	}
	this->lb = ldb;
	/*pcap input check */
	if (pcap_lookupnet(device, &addr, &mask, errbuf) == -1) {
		m_descr = pcap_open_offline(device, errbuf);
		offline = true;
		addr = htonl(0xFFFFFF00);
		mask = htonl(0xFFFFFF00);
	} else {
		int prom = noPromisc ? 0 : 1;
		//200 snaplen
		m_descr = pcap_open_live(device, 200, prom, readTimeout, errbuf);
		offline = false;
	}
	if (m_descr == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", device, errbuf);
		exit(-1);
	}
	/*setting up the pcap filters*/
	/**Accepts only ipv4 traffic.**/
	if (pcap_compile(m_descr, &fp, "ip", 0, addr) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp,
				pcap_geterr(m_descr));
		exit(-1);
	}
	if (pcap_setfilter(m_descr, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp,
				pcap_geterr(m_descr));
		exit(-1);
	}
	pcap_freecode(&fp);
	/**Sets the filter passed by user.**/
	if (filter_exp != NULL) {
		if (pcap_compile(m_descr, &fp, filter_exp, 0, addr) == -1) {
			fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp,
					pcap_geterr(m_descr));
			exit(-1);
		}
		if (pcap_setfilter(m_descr, &fp) == -1) {
			fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp,
					pcap_geterr(m_descr));
			exit(-1);
		}
		pcap_freecode(&fp);
	}
	//calculating the level 2 header offset
	int datalinkType = pcap_datalink(m_descr);
	//TODO Add other switch-case to add the support to other datalink's protocols.
	switch (datalinkType) {
	case 1:
		datalinkOffset = 14;
		break;
	default:
		fprintf(stderr, "Datalink offset for datalink type: %d unknown.",
				datalinkType);
		exit(-1);
	}

}

Emitter::~Emitter() {
	pcap_close(m_descr);
}

/*method called by farm in order to create pkt stream*/
void* Emitter::svc(void * task) {
	start_capturing = time(NULL);
	int res = pcap_loop(m_descr, countPkt, dispatchCallback, (u_char *)this);
	if(res == -2 ){
		//pcap_breakloop
		return NULL;
			}
	if(res == -1 ){
			  pcap_perror(m_descr, NULL);
			  throw std::runtime_error("pcap_loop!");
		}
#ifdef DEBUG_FARM
		std::cout << "Emitter : NULL TASK \n"<< std::endl;
#endif
		return NULL;
}

void Emitter::dispatchPolicy(Task *t) {
#ifdef _COMPUTE_STATS
	unsigned long t1 = ff::getusec();
#endif
	/* DISPATCH POLICY
	 * In order to avoiding load unbalancing between workers i try to combine the
	 * various field in the Flow ID in an hash signature used with the modulus operand
	 * to select an appropriate worker
	 * */
	FlowID_t * e = &(t->fd->fid);

	size_t seed = 0;
	boost::hash_combine(seed, e->dstaddr);
	boost::hash_combine(seed, e->dstport);
	boost::hash_combine(seed, e->prot);
	boost::hash_combine(seed, e->srcaddr);
	boost::hash_combine(seed, e->srcport);

	lb->set_victim(seed % numWorkers);
#ifdef _COMPUTE_STATS
		total_time_hash += (ff::getusec() - t1);
#endif
}

int Emitter::svc_init(void) {
	sigset_t signal_set_gest;
	if (sigemptyset(&signal_set_gest) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	if (sigaddset(&signal_set_gest, SIGINT) < 0) {
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
#endif
	/**indico al main quali segnali bloccare e quali ignorare*/
	if (pthread_sigmask(SIG_BLOCK, &signal_set_gest, NULL)) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	return 0;
}

void Emitter::svc_end(void) {

}

void Emitter::stopCapturing() {
	end = true;
	pcap_breakloop(m_descr);
}

void Emitter::handler(const struct pcap_pkthdr *phdr, const u_char *pdata){
#ifdef DEBUG_FARM
		std::cout << "Emitter : New task \n"<< std::endl;
#endif
#ifdef _COMPUTE_STATS
	unsigned long t1 = ff::getusec();
#endif
#ifdef DEBUG_FARM
	std::cout << "Emitter : taking task \n"<< std::endl;
#endif
		Task* t = new Task();//(Task*) user;

#ifdef __USE_LINUX
	//TODO: check for vlan or other header to skip
	const struct iphdr *ip_hdr = reinterpret_cast<const struct iphdr *>(pdata
			+ datalinkOffset);

	t->fd->dOctets = ntohs(ip_hdr->tot_len) ;//- datalinkOffset;

	//offset to transport protocol
	uint32_t ipHdrLen = (ip_hdr->ihl & 0x0f) * 4;

	uint32_t payloadOffset;
	int transportOffset = datalinkOffset + ipHdrLen;
	//TODO: check if conversion from network representation is needed
	if (ip_hdr->protocol == IPPROTO_TCP) {
		tcphdr* tcp = (struct tcphdr*) (pdata + transportOffset);
		t->fd->initFlowID(ntohl(ip_hdr->saddr), ntohl(ip_hdr->daddr), ntohs(tcp->source), ntohs(tcp->dest),
				ip_hdr->protocol);
		payloadOffset = transportOffset + 20;
	} else if (ip_hdr->protocol == IPPROTO_UDP) {
		udphdr* udp = (struct udphdr*) (pdata + transportOffset);
		t->fd->initFlowID(ntohl(ip_hdr->saddr), ntohl(ip_hdr->daddr), ntohs(udp->source), ntohs(udp->dest),
				ip_hdr->protocol);
		payloadOffset = transportOffset + 8;
	} else {
		//unrecognized Transport protocol
		t->fd->initFlowID(ntohl(ip_hdr->saddr), ntohl(ip_hdr->daddr), 0, 0, ip_hdr->protocol);
		payloadOffset = transportOffset;
	}
	t->fd->payloadOffset = payloadOffset;
#else
	const struct ip *ip_hdr = reinterpret_cast<const struct ip *>(pdata+datalinkOffset);
	//iphdr* ip = (struct iphdr*) (pdata + datalinkOffset);
	t->fd->dOctets= ip_hdr->ip_len - datalinkOffset;

	//offset to transport protocol
	uint32_t ipHdrLen = (ip_hdr->ip_hl & 0x0f) * 4;

	uint32_t payloadOffset;
	int transportOffset = datalinkOffset + ipHdrLen;

	if (ip_hdr->ip_p == IPPROTO_TCP) {
		tcphdr* tcp = (struct tcphdr*) (pdata + transportOffset);
		t->fd->initFlowID(ip_hdr->ip_src.s_addr, ip_hdr->ip_dst.s_addr, tcp->th_sport, tcp->th_dport,
				(u_int8_t)(ip_hdr->ip_p));
		payloadOffset = transportOffset + 20;
	} else if (ip_hdr->ip_p == IPPROTO_UDP) {
		udphdr* udp = (struct udphdr*) (pdata + transportOffset);
		t->fd->initFlowID(ip_hdr->ip_src.s_addr, ip_hdr->ip_dst.s_addr, udp->uh_sport, udp->uh_dport,
				ip_hdr->ip_p);
		payloadOffset = transportOffset + 8;
	} else {
		//unrecognized Transport protocol
		t->fd->initFlowID(ip_hdr->ip_src.s_addr, ip_hdr->ip_dst.s_addr, 0, 0, ip_hdr->ip_p);
		payloadOffset = transportOffset;
	}
	t->fd->payloadOffset = payloadOffset;
	t->fd->dOctets = phdr->len - datalinkOffset;
#endif
	//choosing worker
	dispatchPolicy(t);
	//send to the worker
	ff_send_out(t);
#ifdef _COMPUTE_STATS
		++execTask;
		total_time += (ff::getusec() - t1);
#endif
#ifdef DEBUG_FARM
		std::cout << "Emitter : deliver task \n"<< std::endl;
#endif
}
/**
 * The function called by pcap_dispatch when a packet arrive.
 * \param user A param passed by the emitter in order to get the packet.
 * \param phdr The header of the packet.
 * \param pdata The packet.
 */
void Emitter::dispatchCallback(u_char *user, const struct pcap_pkthdr *phdr,
		const u_char *pdata) {
	Emitter *em = reinterpret_cast<Emitter *>(user);
	em->handler(phdr,pdata);
}

#ifdef _COMPUTE_STATS
float Emitter::getAVGLatency() {
	return (execTask != 0) ? (float) total_time / (float) execTask : -1.0;
}

float Emitter::getAVGHashLatency(){
	return (execTask != 0) ? (float) total_time_hash / (float) execTask : -1.0;
}
unsigned long int Emitter::getNumTasksExecuted() {
	return execTask;
}

void Emitter::printPcapStats(){
	if(!offline && m_descr !=NULL){
	float partial_rate,perc=0;
	pcap_stat ps;
	time_t now = time(NULL);
	pcap_stats(m_descr, &ps);
	std::cout << "=============== Pcap Stats ==============" << std::endl;
	std::cout << "Packets received: " << ps.ps_recv << std::endl;
	std::cout << "Packets dropped by kernel: " << ps.ps_drop << std::endl;
	std::cout << "Packets dropped by NIC: " << ps.ps_ifdrop << std::endl;
	partial_rate=(float)(ps.ps_recv)/(now-start_capturing);
	std::cout << "Packets Rate: " << partial_rate << std::endl;
	if(ps.ps_recv!=0)
		perc=((float)ps.ps_drop/(float)(ps.ps_drop+ps.ps_recv))*100;
	std::cout << "[" << perc << "% packet loss]" << std::endl;
	std::cout << "=========================================" << std::endl;
	}
}
#endif
/*
 * Class Worker
 */

Worker::Worker(uint tableSize, uint keyLen, uint numberOfStages, uint threshold) :
		tableSize(tableSize), keyLen(keyLen), numberOfStages(numberOfStages), threshold(
				threshold) {
	if (tableSize == 0 || keyLen == 0 || numberOfStages == 0) {
		std::cerr << "Invalid Argument on Worker Constructor";
	}
#ifdef _COMPUTE_STATS
	execTask = total_time = 0;
#endif
	mfilter = new FHFMultistageFilter::MultistageFilter(tableSize,
			sizeof(FlowID_t), threshold, numberOfStages);
	ht = new FHFHashTable::HashTable(tableSize);
	pthread_mutex_init(&lock, NULL);

}

Worker::~Worker() {
	pthread_mutex_destroy(&lock);
	delete ht;
	delete mfilter;
}

void * Worker::svc(void * task) {
#ifdef DEBUG_FARM
	std::cout << "Worker : taking task \n"<< std::endl;
#endif
	if (task == NULL) {
		return NULL;
	}
	Task * t = (Task *) task;
	FlowUpdate * fup = t->fd;
#ifdef _COMPUTE_STATS
	/*
	 * i prefer to add to the AVGlatency also the waiting time for the analyzer
	 *  */
	unsigned long t1 = ff::getusec();
#endif
	/*lock the filter , Analyzer have to wait until processing is over*/
	pthread_mutex_lock(&lock);
	//shielding
	if (ht->lookupFlow(&(fup->fid))) {
		ht->updateTable(fup);
	} else if (mfilter->filter(fup)) {
		//filtering
		ht->insertFlow(fup);
	}
	pthread_mutex_unlock(&lock);
#ifdef _COMPUTE_STATS
	++execTask;
	total_time += (ff::getusec() - t1);
#endif
#ifdef DEBUG_FARM
	std::cout << "Worker : ending task \n"<< std::endl;
#endif
	delete t;
	return GO_ON ;
}

void Worker::svc_end(void) {

}

int Worker::svc_init(void) {
	sigset_t signal_set_gest;
	if (sigemptyset(&signal_set_gest) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	if (sigaddset(&signal_set_gest, SIGINT) < 0) {
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
#endif
	/**indico al main quali segnali bloccare e quali ignorare*/
	if (pthread_sigmask(SIG_BLOCK, &signal_set_gest, NULL)) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	return 0;
}

void Worker::update_filter_threshold(unsigned long int newThreshold) {
	this->mfilter->updateThreshold(newThreshold);
	threshold = newThreshold;
}

void Worker::clean_filter(void) {
	this->mfilter->reset();
}

FHFHashTable::HashTable * Worker::getHashTable() {
	return ht;
}

unsigned int Worker::getFilterTableSize() {
	return this->mfilter->getTablesSize();
}

unsigned int Worker::getHashTableSize() {
	return this->ht->getTableMaxSize();
}
unsigned long int Worker::getThreshold() {
	return threshold;
}

void Worker::lockWorker() {
	pthread_mutex_lock(&lock);
}

void Worker::unlockWorker() {
	pthread_mutex_unlock(&lock);
}

#ifdef _COMPUTE_STATS
float Worker::getAVGLatency() {
	return (execTask != 0) ? (float) total_time / (float) execTask : -1.0;
}
unsigned long int Worker::getNumTasksExecuted() {
	return execTask;
}

#endif
}
