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
 * AstarSearch.h
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#ifndef ASTARSEARCH_H_
#define ASTARSEARCH_H_

#include "SearchGraph.h"
#include "SearchPath.h"
#include "SearchNode.h"

#include <set>
#include <queue>
#include <vector>
#include <list>

using std::list;
using std::vector;
using std::priority_queue;
using std::set;
using std::multiset;

namespace osi {

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   AstarSearch
 *
 * Description:
 *   Implements the A* search algorithm
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class AstarSearch {
public:
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   AstarSearch()
	 *
	 * Description:
	 *   Default constructor
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	AstarSearch();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   ~AstarSearch()
	 *
	 * Description:
	 *   Default destructor
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	virtual ~AstarSearch();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   findPath()
	 *
	 * Description:
	 *   Find the shortest path between the specified start and end nodes
	 *   in the search graph using the A* search algorithm.
	 *
	 * Inputs:
	 *   graph     the graph of nodes to search
	 *   startId   the id of the start node within the search graph
	 *   endId     the id of the goal node within the search graph
	 *
	 * In/Out:
	 *   path   Pointer to the SearchPath to hold the resulting shortest path
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void findPath(SearchGraph& graph, const int startId, const int endId,
			      SearchPath* const path, const bool& dyn_flag = false);

    /**
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * Function:
     *   findPath()
     *
     * Description:
     *   Find the shortest path between the specified start and end nodes
     *   in the search graph using the A* search algorithm.
     *
     * Inputs:
     *   graph     the graph of nodes to search
     *   h         aray of pre-computed heuristic costs for nodes
     *   startId   the id of the start node within the search graph
     *   endId     the id of the goal node within the search graph
     *
     * In/Out:
     *   path   Pointer to the SearchPath to hold the resulting shortest path
     *
     * Returns:
     *   none
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     */
    void findPath(const SearchGraph& graph, const double* const h,
                  const int startId, const int endId,
                  SearchPath* const path, const bool& dyn_flag = false);

private:

	/**
	 * -----------------------------------------------------------------------
	 * Function:
	 *   getUnexploredChildren()
	 *
	 * Description:
	 *   Obtain the vector of unexplored child nodes of the given search node.
	 *
	 * Inputs:
	 *   node   the search node for which the unexplored children should be
	 *          obtained
	 *
	 * In/Out:
	 *   unexplored   the pointer to an empty vector for holding the results
	 *
	 * Returns:
	 *   none
	 * -----------------------------------------------------------------------
	 */
	void getUnexploredChildren(SearchNode* node,
			                   vector<SearchNode*>* const unexplored);

    void getUnexploredChildren2(SearchNode* node,
                               vector<SearchNode*>* const unexplored);

private:

	/*
	 * Priority queue of open nodes ordered by the current path cost
	 */
	priority_queue<SearchNode*,
	               vector<SearchNode*>,
	               SearchNodeCostComparator> frontier;
    priority_queue<SearchNode,
                   vector<SearchNode>,
                   SearchNodeCostComparator2> frontier2;

	/*
	 * Open set of nodes to be explored
	 */
	set<SearchNode*, SearchNodeIdComparator> openSet;
	set<SearchNode, SearchNodeIdComparator2> openSet2;

	/*
	 * Closed set of nodes that have already been explored
	 */
	set<SearchNode*, SearchNodeIdComparator> closedSet;
	set<SearchNode, SearchNodeIdComparator2> closedSet2;
};

} /* namespace osi */

#endif /* ASTARSEARCH_H_ */
