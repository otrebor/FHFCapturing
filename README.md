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

 This project uses the FastFlow library

 FastFlow is a C++ parallel programming framework advocating high-level, pattern-based parallel programming. It chiefly supports streaming and data parallelism, targeting heterogenous platforms composed of clusters of shared-memory platforms, possibly equipped with computing accelerators such as NVidia GPGPUs, Xeon Phi, Tilera TILE64.

The main design philosophy of FastFlow is to provide application designers with key features for parallel programming (e.g. time-to-market, portability, efficiency and performance portability) via suitable parallel programming abstractions and a carefully designed run-time support.