//============================================================================
// Name        : FHFcapturing.cpp
// Author      : Roberto Belli
// Version     : 0.1
// Copyright   : 
// Description : Find Heavy Flow by using a capturing device
// Further details on algorithm:
// C. Estan and G. Varghese, “New directions in traffic measurement and
// accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
//============================================================================
/*
 * COMPILE WITH -D __USE_LINUX if you are using a LINUX machine (BSD implementation not completed)
 * COMPILE WITH -D _DEBUG
 * COMPILE WITH -D _COMPUTE_STATS if you want stats about performance
 * COMPILE WITH -D TRACE_FASTFLOW in order to include FF Stats
 * COMPILE WITH -D _AVG_CASE_BALANCING if you prefer to don't allocate memory for the worst-case balancing of workers
 * */
#include <iostream>
#include <fstream>
#include <signal.h>
#include <ff/farm.hpp>
#include "ParallelStages.h"
#include "Analyzer.h"
#include "DataStructures.h"
#include "utils.hpp"
#include <stdint.h>

using namespace std;
using namespace FHFFarm;

namespace FHFcapturing {
//COSTANTI
const unsigned int KEY_LEN = sizeof(FlowID_t);

//GLOBAL VARIABLES
char *interface = NULL; //capturing interface name
char *bpfFilter = NULL; //filter exp
int c;
char *outputPath = NULL;
const char* stdoutf = "FHFcap_report.log";
int cnt = -1; //max num of analyzed pkt
int readTimeout = 5000; //5 sec of timeout on pcap
unsigned int parDegree = 0;
unsigned int hashTablesSize = 1201; //dimension of each stages's table and worker hash table
unsigned int numOfStages = 10;
unsigned int sampleInterval = 5; //seconds
float oversamplingFactor = 0.2;
bool noPromisc = false;
unsigned int nWorkers = 1;
unsigned long int linkCapacity = 0; //byte per second
double percentageInterestingFlow = 0.001;

//FUNCTIONS
void argParsing(int argc, char** argv);
void printHelp(char* progName);

void printHelp(char* progName) {
	fprintf(stderr,
			"\nusage: %s -i <captureInterface|pcap> [-b <bpf filter>] \n"
					"[-t <readTimeout>] [-w <parDegree>] [-s <oversamplingFactor>] [-c <cnt>]\n"
					"[-f <outputFile>] [-l <linkCapacity>] [-y <percentegeOfCapacity>] [-z <numberOfStages>] \n"
					"[-r] [-h]\n\n\n", progName);
	fprintf(stderr,
			"-i <captureInterface|pcap> | Interface name from which packets are captured, or .pcap file\n");
	fprintf(stderr,
			"[-b <bpf filter>]          | It specifies a bpf filter.\n");
	fprintf(stderr,
			"[-t <readTimeout>]         | It specifies the read timeout (milliseconds) when reading from\n"
					"                           | the pcap socket [default 30 seconds]\n");
	fprintf(stderr,
			"[-w <parDegree>]           | It specifies how many thread must be activated [default sequential execution].\n");
	fprintf(stderr,
			"[-s <oversamplingFactor>]  | It specifies the size of each hash table \n"
					"							| size= 1.0/percentageInterestingFlow)+(1.0/percentageInterestingFlow)*oversamplingFactor [0.2]\n");
	fprintf(stderr,
			"[-c <cnt>]                 | Cnt is the maximum number of packets to process before returning from reading, but is not a minimum\n"
					"                           | number. When  reading  a  live capture, only one bufferful of packets is read at a time,\n"
					"                           | so fewer than cnt packets may be  processed. If no packets are presents, read returns immediately.\n"
					"                           | A  value of -1 or 0 for cnt causes all the packets received in one buffer to be processed when\n"
					"                           | reading a live capture,  and  causes all the packets in the file to be processed when reading\n"
					"                           | a pcap file [default -1]\n");
	fprintf(stderr,
			"[-f <outputFile>]          | Print the flows in textual format on a file [default FHF_log.txt]\n");
	fprintf(stderr,
			"[-l <linkCapacity>]		| It specifies the capacity (in byte/second) of the capture interface\n");
	fprintf(stderr,
			"[-y <percentegeOfCapacity>]| It specify the percentage of bandwidth occupied by an \"heavy flow\". If a flow is under the specified percentage the flow\n"
					"                           | is emitted. [0.001]\n");
	fprintf(stderr,
			"[-q <Sampling Interval>]   | It specifies the sampling interval in seconds [5] .\n");
	fprintf(stderr,
			"[-z <numberOfStages>]      | It specifies the number of stages used by each Multistage Filter [default 10]\n");
	fprintf(stderr,
			"[-r]                       | Put the interface into 'No promiscous' mode\n");
	fprintf(stderr, "[-h]               | Prints this help\n");
}

void argParsing(int argc, char** argv) {
	/**Args parsing.**/
	while ((c = getopt(argc, argv, "i:b:t:w:s:c:f:l:y:q:z:rh")) != -1)
		switch (c) {
		case 'i':
			interface = optarg;
			break;
		case 'b':
			bpfFilter = optarg;
			break;
		case 't':
			readTimeout = atoi(optarg);
			break;
		case 'w':
			parDegree = atoi(optarg);
			break;
		case 's':
			oversamplingFactor = atof(optarg);
			break;
		case 'c':
			cnt = atoi(optarg);
			break;
		case 'f':
			outputPath = optarg ;
			break;
		case 'l':
			linkCapacity = atoi(optarg);
			break;
		case 'q':
			sampleInterval = atoi(optarg);
			break;
		case 'y':
			percentageInterestingFlow = atof(optarg);
			break;
		case 'z':
			numOfStages = atoi(optarg);
			break;
		case 'r':
			noPromisc = true;
			break;
		case 'h':
			printHelp(argv[0]);
			exit(1);
		case '?':
			printHelp(argv[0]);
			exit(1);
		default:
			fprintf(stderr, "Unknown option.\n");
			printHelp(argv[0]);
			exit(-1);
		}
	if (interface == NULL) {
		printf("ERROR: -i <interface> required.\n");
		exit(-1);
	}
	if (linkCapacity == 0) {
		printf("ERROR: -l <linkCapacityInByte/Second> required.\n");
		exit(-1);
	}
	if (parDegree > 3) {
		nWorkers = parDegree - 2;
	} else
		nWorkers = 1;

}
#ifdef _DEBUG
void tests(){
	/*TESTS
	 * hashTableTest();
	 * filterTest();
	 * AnalyzerTest::AnalyzerTest();
	 * float dim = (1.0/percentageInterestingFlow)+(1.0/percentageInterestingFlow)*oversamplingFactor;
	 * nextPrime(dim);
	 *
	 * exit(0);
	 */
}
#endif

}

using namespace FHFcapturing;

int main(int argc, char** argv) {
	//ASSSERTS
	/* Is fundamental that no padding is present in order to hash the ID CORRECTLY */
	assert(sizeof(FlowID_t)==(sizeof(u_int32_t)*2 +sizeof(u_int16_t)*2+sizeof(u_int8_t)));
	//init rand function
	srand((unsigned) time(0));
	/*SIG HANDLING*/
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
	if (sigaddset(&signal_set_gest, SIGALRM) < 0) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
	/**indico al main quali segnali bloccare e quali ignorare*/
	if (pthread_sigmask(SIG_BLOCK, &signal_set_gest, NULL)) {
		perror("Errors during SIGNAL handling");
		exit(-1);
	}
#ifdef _DEBUG
	/*TESTS*/
	//tests();
#endif

	/*arguments Parsing*/
	argParsing(argc, argv);
	/*set dimension of HashTables*/
	float dimH = (1.0/percentageInterestingFlow)+(1.0/percentageInterestingFlow)*oversamplingFactor;
	hashTablesSize = nextPrime(dimH);

	/*the maximum number of interesting flow must be under the dimension of the hash table size*/
	assert(FHFcapturing::hashTablesSize > (1.0 / percentageInterestingFlow));

	//creating Farm
	ff::ff_farm<LoadBalancer> farm;
	long int firstThreshold = (FHFcapturing::linkCapacity
			* FHFcapturing::percentageInterestingFlow) * sampleInterval;
	Emitter pktDispatcher(nWorkers, interface, noPromisc, bpfFilter, cnt,
			readTimeout, farm.getlb());

	farm.add_emitter(&pktDispatcher);
	/*Collection of Workers REF for FARM  */
	vector<ff::ff_node *> w;
	/*Collection of Workers REF for ANALYZER  */
	vector<Worker *> workers;
#ifdef _AVG_CASE_BALANCING
	/*dimensioning the hashTable of each worker (need prime number)*/
	unsigned int hashPart= ((float)hashTablesSize / (float)nWorkers) +1 ;
	unsigned int workerSize = nextPrime( hashPart+hashPart*oversamplingFactor );
#endif

	for (unsigned int i = 0; i < nWorkers; ++i) {
#ifdef _AVG_CASE_BALANCING
	/*if you prefer to allocate non worst-case hash table on workers */
	Worker * wr = new Worker(workerSize, KEY_LEN, numOfStages,firstThreshold);
#else
	Worker * wr = new Worker(hashTablesSize, KEY_LEN, numOfStages,firstThreshold);
#endif
		workers.push_back(wr);
		w.push_back(wr);
	}
	/*connecting workers to the farm*/
	farm.add_workers(w);
	//Exporter
	FHFcapturing::FileExporter *fex;
	if(outputPath==NULL){
		fex=new FHFcapturing::FileExporter(stdoutf);
	} else {
		fex=new FHFcapturing::FileExporter(outputPath);
	}
	//Analyzer
	FHFcapturing::Analyzer * analyze = new FHFcapturing::Analyzer(&workers, &pktDispatcher,
			sampleInterval, fex);
	analyze->go();
	//Starting Farm and Wait End
	if (farm.run_and_wait_end() < 0) {
		ff::error("running Farm");
		exit(-1);
	}
	analyze->stop();
#ifdef _COMPUTE_STATS
	std::cerr << "EMITTER-> AVGLatency: " << pktDispatcher.getAVGLatency() << " usec" <<std::endl;
	std::cerr << "EMITTER-> AVGHashLatency: " << pktDispatcher.getAVGHashLatency() << " usec" <<std::endl;
	double avgWorkersLatency = 0;
	for(unsigned int i = 0; i < workers.size(); i++){
		avgWorkersLatency += workers.at(i)->getAVGLatency();
		std::cerr << "WORKER("<< i <<")-> AVGLatency: " << workers.at(i)->getAVGLatency() << " usec" <<std::endl;
		std::cerr << "WORKER("<< i <<")-> ExecTask: " << workers.at(i)->getNumTasksExecuted() << " usec" <<std::endl;
	}
	avgWorkersLatency = avgWorkersLatency/workers.size();
	std::cerr << "ANALYZER-> AVGLatency: " << analyze->getAVGLatency() << " usec" <<std::endl;
	std::cerr << "ANALYZER-> AVGWorkerWaitingTime: " << analyze->getAVGWorkerWaitingTime() << " usec" <<std::endl;
	int nOptWorker = avgWorkersLatency/pktDispatcher.getAVGLatency() ;
	std::cerr << "GLOBAL-> Optimal parallel degree: " << ((nOptWorker>1) ? (nOptWorker + 2) : 3) << std::endl ;
	farm.ffStats(std::cerr);
	pktDispatcher.printPcapStats();
#endif

	//cleanUp
	for (unsigned int i = 0; i < nWorkers; ++i) {
			delete workers.at(i);
		}
	delete analyze;
	delete fex;
	std::cout << "Exit main"<< std::endl;

	exit(0);

}
//TODO: possible performance improvements
/*
 * + Thread pinning of workers, emitter (& analyzer) (FF supports pinning)
 * + implement a fast and contiguous space hash table (not using the libboost unordered map)
 *   with discard policy on flow add
 * + inline svc methods of emitter & workers
 * + change hashing functions in multistage filter and hash table and in dispatch policy of the emitter with more performant ones
 * + for Tasks allocation use the FastFlow Allocator
 * + is possible also to delegate to worker the checking of the hashtable and leave to the analyzer to calculate the threshold and collect results.
 *  (but the timing and the syncro in each interval could be more difficult)
 * + pipelining the emitter dividing capture from hashing & dispatch
 * + instantiate more pipeline emitter-worker for multiple hardware queue
 * */


