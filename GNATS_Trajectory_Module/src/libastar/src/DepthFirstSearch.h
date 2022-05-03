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
 * DepthFirstSearch.h
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#ifndef DEPTHFIRSTSEARCH_H_
#define DEPTHFIRSTSEARCH_H_

#include "SearchGraph.h"
#include "SearchPath.h"
#include "SearchNode.h"

#include <vector>

using std::vector;

namespace osi {


/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   DepthFirstSearch
 *
 * Description:
 *   This class implements a depth first search algorithm to find all paths
 *   in a search graph starting from a specified node.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class DepthFirstSearch {
public:

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   DepthFirstSearch()
	 *
	 * Description:
	 *   Default constructor.
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
	DepthFirstSearch();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   ~DepthFirstSearch()
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
	virtual ~DepthFirstSearch();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   findPaths()
	 *
	 * Description:
	 *   Find all paths within the search graph that start at the specified
	 *   node and end at the specified node.
	 *
	 * Inputs:
	 *   graph      the graph containing the nodes to search
	 *   start_id   the id of the starting node
	 *   end_id     the id of the ending node
	 *
	 * In/Out:
	 *   paths   Pointer to the empty vector to hold the resulting paths
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void findPaths(SearchGraph& graph,
			       const int start_id,
			       const int end_id,
			       vector<SearchPath>* const paths);

private:

	/**
	 * -----------------------------------------------------------------------
	 * Function:
	 *   getUnvisitedChildren()
	 *
	 * Description:
	 *   Get the set of unvisited children of the specified node.
	 *
	 * Inputs:
	 *   graph   graph containing the nodes to search
	 *   id      id of the node to obtain unvisited children for
	 *
	 * In/Out:
	 *   unvisited  pointer to an empty vector of SearchNodes to hold the
	 *              results
	 *
	 * Returns:
	 *   none
	 * -----------------------------------------------------------------------
	 */
	void getUnvisitedChildren(SearchGraph& graph, const int& id,
			  	              CostOrderedSet* const unvisited) const;

	/**
	 * -----------------------------------------------------------------------
	 * Function:
	 *   markNodeVisited()
	 *
	 * Description:
	 *   Mark a node as explored.  This will set the node's
	 *   visited member to true, remove the node from the
	 *   open-list and place it in the closed list.
	 *
	 * Inputs:
	 *   graph   graph containing the search nodes
	 *   id      id of the node to mark as visited
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * -----------------------------------------------------------------------
	 */
	void markNodeVisited(SearchGraph& graph, const int& id);

	/**
	 * -----------------------------------------------------------------------
	 * Function:
	 *   markNodeUnvisited()
	 *
	 * Description:
	 *   Mark a node as unexplored.  This will set the node's
	 *   visited member to false, remove the node from the
	 *   closed list and place it in the open list.
	 *
	 * Inputs:
	 *   graph   graph containing the search nodes
	 *   id      id of the node to mark as unvisited
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * -----------------------------------------------------------------------
	 */
	void markNodeUnvisited(SearchGraph& graph, const int& id);

private:

	/*
	 * list of nodes to search, ordered by node cost.
	 * this is the open-list or frontier
	 */
	CostOrderedSet openList;

	/*
	 * list of nodes that have been explored.
	 * this is the closed-list or explored set.
	 * ordered by node id to allow binary search
	 */
	IdOrderedSet closedList;
};

}

#endif /* DEPTHFIRSTSEARCH_H_ */
