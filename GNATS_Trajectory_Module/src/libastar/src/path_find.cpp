/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
*/

/*
 * path_find.cpp
 *
 *  Created on: May 3, 2012
 *      Author: jason
 */

#include "path_find.h"

#include "DepthFirstSearch.h"
#include "AstarSearch.h"

#include <vector>
#include <set>
#include <stack>
#include <deque>
#include <algorithm>

using std::vector;
using std::set;
using std::multiset;
using std::stack;
using std::deque;

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace osi {



//////////////////////////////////////////////////////////////////////////////
// Public API function implementations
//


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
void delete_path_linked_list(path_t* head) {
	path_t* path = head;
	while(path != NULL) {
		path_t* free_path = path;
		path = path->next;
		free(free_path->node_sequence);
		free(free_path);
		free_path = NULL;
	}
}

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
                const int end_id) {

	vector<SearchPath> paths;
	SearchGraph graph(num_nodes, connectivity);
	DepthFirstSearch dfs;
	dfs.findPaths(graph, start_id, end_id, &paths);

	// copy the vector of SearchPaths to the outout linked list
	// TODO: if the copying causes too much of a performance hit, we should
	// look into modifying or augmenting DFS to output directly to the
	// linked list.
	path_t* head = NULL;
	for(unsigned int i=0; i<paths.size(); ++i) {
		SearchPath* sp = &(paths.at(i));
		path_t* path = (path_t*)calloc(1, sizeof(path_t));
		path->num_nodes = sp->size();
		path->node_sequence = (unsigned int*)calloc(path->num_nodes, sizeof(unsigned int));
		path->num_paths = paths.size();

		// TODO: is there no other way other than to copy element by element?
		for(unsigned int j=0; j<path->num_nodes; ++j) {
			path->node_sequence[j] = sp->getNodes()[j];
		}

		path->next = head;
		head = path;
	}
	return head;
}

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
		                const double* const heuristic_costs,
		                const unsigned int num_nodes,
		                const int start_id,
		                const int end_id) {

	SearchPath sp;
	SearchGraph graph(num_nodes, connectivity, path_costs, heuristic_costs);
	AstarSearch astar;
	astar.findPath(graph, start_id, end_id, &sp);

	path_t* head = NULL;
	path_t* path = (path_t*)calloc(1, sizeof(path_t));
	path->num_nodes = sp.size();
	path->node_sequence = (unsigned int*)calloc(path->num_nodes, sizeof(unsigned int));
	path->num_paths = 1;

	// TODO: is there no other way other than to copy element by element?
	for(unsigned int j=0; j<path->num_nodes; ++j) {
		path->node_sequence[j] = sp.getNodes()[j];
	}

	path->next = head;
	head = path;

	return head;
}

}
