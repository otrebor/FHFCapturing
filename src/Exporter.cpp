/*
 * Exporter.cpp
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

#include "Exporter.h"


namespace FHFcapturing {

/**
 * FILE EXPORTER
 */
FileExporter::FileExporter(const char * path){
	fpath = path;
}

FileExporter::~FileExporter(){

}

void FileExporter::open(){
	if (!output.is_open()) {
			output.open(fpath);
			if (!output.is_open()) {
				perror("no open file");
				throw std::invalid_argument("invalid output file");
			}
		}
}
void FileExporter::exportFlow(const std::vector<FlowDescriptor> * fvec,const IntervalDescriptor * intervalDescr){
		 /* ok, proceed with output */
			output << "====================== Report interval n° " << intervalDescr->intervalNum <<" ===============================\n";
			output << "Interval StartingAt: "<< intervalDescr->startT << " during: " << difftime(intervalDescr->stopT,intervalDescr->startT) <<"\n";
			output << "Threshold: "<< intervalDescr->threshold << " r_threshold: "<< intervalDescr->r_threshold << " \n";
			output << "---------------------------------------------------------------------------\n";
			for(unsigned int i=0 ; i< fvec->size(); i++){
				printFlow(&(fvec->at(i)));
			}
			output << "===========================================================================\n";
			output.flush();

}
void FileExporter::close(){
	output.close();
}

void FileExporter::IPtoChar(u_int32_t UIntIP ){
    //output << (UIntIP & 0xff) << "." << ((UIntIP >> 8) & 0xff) << "." << ((UIntIP >> 16) & 0xff) << "." << (UIntIP >> 24);
	output << (UIntIP >> 24) << "." << ((UIntIP >> 16) & 0xff) << "." << ((UIntIP >> 8) & 0xff)  << "." << (UIntIP & 0xff) ;
}

void FileExporter::printFlow(const FlowDescriptor * it){
	output  << " [ ID: < DSTA:" ;
	IPtoChar(it->fid.dstaddr);
	output  << ", DSTP:" << it->fid.dstport << ", PROT:" << ((int)it->fid.prot) << ", SRCA:" ;
	IPtoChar(it->fid.srcaddr);
	output << ", SRCP:" << it->fid.srcport << "> --> oct:"<< it->dOctets << " pkt:" << it->dPkts <<" cnt:" << it->cycles <<"  ]\n";
}

} /* namespace FHFcapturing */
