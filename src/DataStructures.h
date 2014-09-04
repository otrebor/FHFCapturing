/*
 * DataStructures.h
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

#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_
#include <netinet/in.h>
#include <boost/unordered_map.hpp>

namespace FHFDataStructures {
/* data type added to avoid the compiler padding */
struct FlowID_t {
//public:
	/*represents a 5-tuple Source Address, Destination Address, protocol, source port , destination port*/
	/*For simplicity we will use the IPv4 representation ONLY*/
	u_int32_t srcaddr; /* Source IP Address */
	u_int32_t dstaddr; /* Destination IP Address */
	u_int16_t srcport; /* TCP/UDP source port number (.e.g, FTP, Telnet, etc.,or equivalent) */
	u_int16_t dstport; /* TCP/UDP destination port number (.e.g, FTP, Telnet, etc.,or equivalent) */
	u_int8_t prot; /* IP protocol, e.g., 6=TCP, 17=UDP, etc... */
	/*Part of description of netflow v5*/
}__attribute__((packed)); /*avoid compiler padding*/


class FlowUpdate {
public:
	FlowID_t fid;
	u_int32_t dOctets;
	u_int32_t payloadOffset; /* Payload length*/
	FlowUpdate(FlowID_t id) :
			fid(id), dOctets(0), payloadOffset(0) {
	}
	FlowUpdate():dOctets(0), payloadOffset(0){initFlowID(0,0,0,0,0);}
	inline void initFlowID(u_int32_t srcaddr, u_int32_t dstaddr, u_int16_t srcport,u_int16_t dstport, u_int8_t prot)
		 {
		 this->fid.srcaddr=srcaddr;
		 this->fid.dstaddr=dstaddr;
		 this->fid.srcport=srcport;
		 this->fid.dstport=dstport;
		 this->fid.prot=prot;
	}
};

class FlowDescriptor {
public:
	FlowID_t fid;
	u_int32_t dPkts; /* Packets sent */
	u_int32_t dOctets; /* Octets sent */
	u_int32_t cycles;
	FlowDescriptor():dPkts(0),dOctets(0),cycles(0){}
	FlowDescriptor(const FlowDescriptor &fd):dPkts(fd.dPkts),dOctets(fd.dOctets),cycles(fd.cycles){
		fid.dstaddr = fd.fid.dstaddr;
		fid.dstport = fd.fid.dstport;
		fid.prot = fd.fid.prot;
		fid.srcaddr = fd.fid.srcaddr;
		fid.srcport = fd.fid.srcport;
	}
	inline void incrementd_0ctects(u_int32_t add) {
		dOctets += add;
	}
	inline void increment_dPkts(void) {
		dPkts++;
	}
};

class Task {
public:
	FlowUpdate *fd;
	Task(){ fd=new FlowUpdate(); }
	~Task(){delete fd;}
};

class IntervalDescriptor {
public:
	unsigned int intervalNum;
	time_t startT, stopT;
	unsigned long int threshold, r_threshold;
	IntervalDescriptor(unsigned int num, time_t start, time_t stop, unsigned long int threshold , unsigned int r_threshold):intervalNum(num),startT(start),stopT(stop),threshold(threshold),r_threshold(r_threshold){}
	~IntervalDescriptor(){}
};

/* hash function used by boost::unordered_map on keys*/
struct ihash: std::unary_function<FlowID_t, std::size_t> {
	std::size_t operator()(FlowID_t const& e) const {
		std::size_t seed = 0;
		boost::hash_combine(seed, e.dstaddr);
		boost::hash_combine(seed, e.dstport);
		boost::hash_combine(seed, e.prot);
		boost::hash_combine(seed, e.srcaddr);
		boost::hash_combine(seed, e.srcport);
		return seed;
	}
};

struct iequal_to: std::binary_function<FlowID_t, FlowID_t, bool> {
	bool operator()(FlowID_t const& l, FlowID_t const& r) const {
		return ((l.dstaddr == r.dstaddr) && (l.dstport == r.dstport)
				&& (l.prot == r.prot) && (l.srcaddr == r.srcaddr)
				&& (l.srcport == r.srcport));

	}
};

}
#endif /* DATASTRUCTURES_H_ */
