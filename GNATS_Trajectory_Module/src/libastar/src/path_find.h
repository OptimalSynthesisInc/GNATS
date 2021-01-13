/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
*/

/*
 * path_find.h
 *
 *  This file defines a C-syle API with C-linkage for invoking the
 *  path finding algorithms.
 *
 *  Created on: May 3, 2012
 *      Author: jason
 */

#ifndef PATH_FIND_H_
#define PATH_FIND_H_

#include <vector>
#include <deque>
#include <set>
#include <cstdlib>

using std::set;
using std::vector;
using std::deque;


namespace osi {

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C-style linked list of paths to use as output values for the path
 * finding algorithms.  We use this so that the user can use
 * this library from C and not have to build with g++ and use STL
 * vectors.
 */
typedef struct _path_t {
	unsigned int num_nodes;       /* number of nodes in the sequence      */
	unsigned int* node_sequence;  /* node sequence that defines this path */

	unsigned int num_paths;       /* number of paths in the linked list   */
	struct _path_t* next;         /* pointer to next path in linked list  */
} path_t;

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   delete_path_linked_list()
 *
 * Description:
 *   Delete the path linked list previously allocated using
 *   new_path_linked_list().  This will iterate over each path_t in
 *   the linked list and free the path.
 *
 * Inputs:
 *   head  the head of the path linked list to be deleted.
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void delete_path_linked_list(path_t* head);

/**
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   find_paths()
 *
 * Description:
 *   find all paths that start and end at the given node ids.  first,
 *   all paths that start at the specified id are found using DFS.
 *   then, the paths that do not end at the specified end id are removed
 *   from the result set.
 *
 * Inputs:
 *   connectivity   the connectivity matrix
 *   num_nodes      number of nodes in the connectivity matrix
 *   start_id       the id of the starting node
 *   end_id         the id of the ending node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   A pointer to the head of a linked list of paths.  The linked list is
 *   dynamically allocated in this function.  The user MUST free it using
 *   delete_path_linked_list().
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
path_t* find_paths(const bool* const *const connectivity,
		        const unsigned int num_nodes,
		        const int start_id,
		        const int end_id);


/**
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   find_shortest_path()
 *
 * Description:
 *   find the shortest path that starts and ends at the given node ids
 *   using the A* algorithm.
 *
 * Inputs:
 *   connectivity   the connectivity matrix
 *   path_costs     the link cost matrix
 *   num_nodes      number of nodes in the connectivity and cost matrices
 *   start_id       the id of the starting node
 *   end_id         the id of the ending node
 *
 * In/Out:
 *   path_out   the shortest path from start_id to end_id.  the path is
 *              defined by a sequence of node ids.
 *
 * Returns:
 *   A pointer to the head of a linked list of paths.  The linked list is
 *   dynamically allocated in this function.  The user MUST free it using
 *   delete_path_linked_list().  For this function, the linked list size
 *   is 1, the shortest path from start_id to end_id.
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
path_t* find_shortest_path(const bool* const *const connectivity,
		                const double* const *const path_costs,
		                const double* const heuristicCosts,
		                const unsigned int num_nodes,
		                const int start_id,
		                const int end_id);
}

#ifdef __cplusplus
} /* extern "c" */
#endif

#endif /* PATH_FIND_H_ */
