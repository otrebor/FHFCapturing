/*
 * Exporter.h
 *
 *  Created on: Oct 12, 2012
 *  Author      : Roberto Belli
 *  Version     : 0.1
 *  Copyright   :
 *  Description : Find Heavy Flow by using a capturing device
 *  Further details on algorithm:
 *  C. Estan and G. Varghese, “New directions in traffic measurement and
 *  accounting,” in Proc. ACM SIGCOMM, Oct. 2002,
 */

#ifndef EXPORTER_H_
#define EXPORTER_H_
#include <vector>
#include <fstream>
#include <stdexcept>
#include "DataStructures.h"

using namespace FHFDataStructures;

namespace FHFcapturing {

class Exporter {
public:
	virtual ~Exporter(){};
	virtual void open()=0;
	virtual void exportFlow(const std::vector<FlowDescriptor> * fvec , const IntervalDescriptor * intervalDescr)=0;
	virtual void close()=0;
};

class FileExporter:public Exporter {
public:
    FileExporter(const char * path);
    virtual ~FileExporter();
	virtual void open();
	virtual void exportFlow(const std::vector<FlowDescriptor> * fvec, const IntervalDescriptor * intervalDescr);
	virtual void close();
private:
	const char * fpath;
	std::ofstream output;
	void output_results(std::vector<FlowDescriptor> * results);
	void IPtoChar(u_int32_t UIntIP );
	void printFlow(const FlowDescriptor * it);
};

} /* namespace FHFcapturing */
#endif /* EXPORTER_H_ */
