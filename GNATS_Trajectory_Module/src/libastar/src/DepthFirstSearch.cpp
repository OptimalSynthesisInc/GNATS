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
 * DepthFirstSearch.cpp
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#include "DepthFirstSearch.h"
#include "SearchNode.h"
#include "SearchPath.h"
#include "SearchGraph.h"

#include <stack>
#include <set>
#include <algorithm>

using std::stack;
using std::set;

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace osi {

static void reconstruct(SearchNode* node, SearchPath* const path) {
	SearchNode* n = node;
	while(n) {
		path->push_front(n->id);
		n = n->parent;
	}
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
DepthFirstSearch::DepthFirstSearch() {

}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
DepthFirstSearch::~DepthFirstSearch() {

}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void DepthFirstSearch::findPaths(SearchGraph& graph,
		                         const int start_id,
		                         const int end_id,
		                         vector<SearchPath>* const paths) {

	// clear the output
	paths->clear();

	// obtain the start node
	SearchNode start;
	start.id = start_id;

	// obtain the goal node
	SearchNode goal;
	goal.id = end_id;

	// if the start node equals the goal node then add
	// a new path to the output
	if(goal.id == start.id) {
		SearchPath path;
		path.push_front(start.id);
		paths->push_back(path);
	}

	// initialize an empty set of explored nodes
	set<SearchNode*> explored;

	// initialize the frontier stack
	stack<SearchNode*> stack;
	stack.push(&start);

	// loop until the stack is empty, meaning we've explored everything
	while(!stack.empty()) {
		SearchNode* node = stack.top();
		stack.pop();

		// we don't mark the node as visited so that we will
		// continue to expand all children in order to find multiple
		// or redundant paths.
		//node->visited = true;

		multiset<SearchNode*, SearchNodeCostComparator> unvisited;
		getUnvisitedChildren(graph, node->id, &unvisited);
		//cout << "  num children: " << unvisited.size() << endl;
		set<SearchNode*, SearchNodeCostComparator>::iterator iter;
		for(iter=unvisited.begin(); iter!=unvisited.end(); ++iter) {
			SearchNode* child = *iter;
			child->parent = node;
			if(child->id == goal.id) {
				// reconstruct the path and add to output

				SearchPath path;
				reconstruct(child, &path);
				paths->push_back(path);
			}
			stack.push(child);
		}
	}


#if 0
	// first find all paths from start_id
	this->findPaths(graph, start_id, paths);

#ifndef NDEBUG
	cout << "found " << paths->size() << " paths starting at " << start_id << "." << endl;
#endif

	// find paths that contain end_id and chop of any path points
	// after end_id, or mark the path for removal if it doesn't
	// contain the end_id.
	vector< deque<int> >::iterator pathIter;
	vector<int> indicesToRemove;
	for(unsigned int i=0; i<paths->size(); ++i) {
		SearchPath* path = &(paths->at(i));
		SearchPathIterator found = find(path->begin(), path->end(), end_id);
		if(found == path->end()) {
			// mark for removal
			indicesToRemove.push_back(i);
		} else {
			// chop of elements after the found iterator
			path->erase(found+1, path->end());
		}
	}

	// remove paths that don't contain end_id
	vector<int>::iterator indexIter;
	for(indexIter=indicesToRemove.begin();
	    indexIter!=indicesToRemove.end();
	    ++indexIter) {

		paths->erase(paths->begin()+(*indexIter));
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
// private impl.

/**
 * -----------------------------------------------------------------------
 * Function:
 *   findNode()
 *
 * Description:
 *   Find a node by ID from the specified map of nodes
 *
 * Inputs:
 *   nodes   map containing the search nodes
 *   id      id of the node to mark as unvisited
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   Pointer to the resulting search node
 * -----------------------------------------------------------------------
 */
static SearchNode* findNode(const map<int, SearchNode*>& nodes, const int& id) {
	map<int, SearchNode*>::const_iterator iter = nodes.find(id);
	if(iter == nodes.end()) {
		// node not found
		return NULL;
	}
	return iter->second;
}

/**
 * ---------------------------------------------------------------------------
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
 * ---------------------------------------------------------------------------
 */
void DepthFirstSearch::getUnvisitedChildren(SearchGraph& graph, const int& id, CostOrderedSet* const unvisited) const {

	// find the node
	SearchNode* node = findNode(graph.getNodes(), id);
	if(node == NULL) return;

	unvisited->clear();

	set<SearchNode*, SearchNodeCostComparator>::iterator iter;
	for(iter=node->children.begin(); iter != node->children.end(); ++iter) {
		if(!(*iter)->visited) {
			unvisited->insert(*iter);
		}
	}
}

/**
 * ---------------------------------------------------------------------------
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
 * ---------------------------------------------------------------------------
 */
void DepthFirstSearch::markNodeVisited(SearchGraph& graph, const int& id) {
	// find the node
	SearchNode* node = findNode(graph.getNodes(), id);
	if(node == NULL) return;

	node->visited = true;
	openList.erase(node);
	closedList.insert(node);
}

/**
 * ---------------------------------------------------------------------------
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
 * ---------------------------------------------------------------------------
 */
void DepthFirstSearch::markNodeUnvisited(SearchGraph& graph, const int& id) {
	// find the node
	SearchNode* node = findNode(graph.getNodes(), id);
	if(node == NULL) return;

	node->visited = false;
	closedList.erase(node);
	openList.insert(node);
}

}
