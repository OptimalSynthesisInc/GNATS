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
 * AstarSearch.cpp
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#include "AstarSearch.h"

#include <set>
#include <queue>
#include <vector>
#include <list>
#include <cstring>
using std::vector;
using std::priority_queue;
using std::set;
using std::multiset;
using std::list;

#include <iostream>
using std::cout;
using std::endl;

namespace osi {


/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
AstarSearch::AstarSearch() :
	frontier(std::priority_queue<SearchNode*, vector<SearchNode*>, SearchNodeCostComparator>()),
	openSet(set<SearchNode*, SearchNodeIdComparator>()),
	closedSet(set<SearchNode*, SearchNodeIdComparator>()) {
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
AstarSearch::~AstarSearch() {
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void AstarSearch::findPath(SearchGraph& graph,
						const int startId,
						const int endId,
						SearchPath* const path,
						const bool& dyn_flag) {
	// obtain the starting node
	SearchNode* startNode = graph.getNode(startId);
	if (!startNode) {
		return;
	}

	// obtain the goal node
	SearchNode* goalNode = graph.getNode(endId);
	if (!goalNode) {
		return;
	}

	// put the start node in the frontier
	frontier.push(startNode);
	openSet.insert(startNode);

	// map of the parent nodes that the lowest cost child node came from.
	// this will be used to reconstruct the shortest path
	map<int, int> cameFrom;

	unsigned int numNodes = graph.size();
	double f_score[numNodes];
	double g_score[numNodes];
	for(unsigned int i=0; i<numNodes; ++i) {
		f_score[i] = 0;
		g_score[i] = 0;
	}

	g_score[startNode->id] = 0;
	f_score[startNode->id] = g_score[startNode->id] + graph.getHeuristicCost(startNode->id);

	vector<SearchNode*> unexplored(200000);


	// the graph search loop

	while(frontier.size() > 0) {

		// select frontier node with the least cost: best first approach
		// since the frontier is ordered by cost, the lowest cost node
		// will be at the front.
		SearchNode* selectedNode = frontier.top();

		// check if we reached the goal
		if(selectedNode->id == goalNode->id) {
			break;
		}

		// did not reach the goal yet so expand the frontier:
		// remove the selectedNode from the frontier and add
		// the selectedNode's children (connected nodes)
		// to the frontier.  put the current selectedNode into the
		// open set.  update the current path cost at the
		// unexplored child nodes.  this is the current cost of the
		// selected node + pathCost from selectedNode to child +
		// child's heuristic cost.  keep track of which parent node
		// the child came from since a child can have multiple parent
		// nodes.
		selectedNode->visited = true;
		frontier.pop();

		openSet.erase(selectedNode);
		closedSet.insert(selectedNode);

		unexplored.clear();
		getUnexploredChildren(selectedNode, &unexplored);


		vector<SearchNode*>::iterator iter;
		for(iter=unexplored.begin(); iter!=unexplored.end(); ++iter) {
			SearchNode* child = *iter;

			set<SearchNode*, SearchNodeIdComparator>::iterator found = openSet.find(child);
			//CHANGED PARIKSHIT
			double g_temp = 0.0;
			if (dyn_flag)
				g_temp = g_score[selectedNode->id] +
					graph.getEdgeCost(selectedNode->id, child->id,g_score[selectedNode->id]);
			else
				g_temp = g_score[selectedNode->id] +
								graph.getEdgeCost(selectedNode->id, child->id);
			if((found == openSet.end()) || (g_temp < g_score[child->id])) {
				g_score[child->id] = g_temp;
				f_score[child->id] = g_score[child->id] + graph.getHeuristicCost(child->id);
				child->cost = f_score[child->id];

				cameFrom[child->id] = selectedNode->id;
				frontier.push(child);
				openSet.insert(child);
			}
		}
	}

	// reconstruct the path
	path->clear();
	int id = goalNode->id;
	map<int, int>::iterator iter;
	while (id != startNode->id) {
		path->push_front(id);

		iter = cameFrom.find(id);
		if (iter == cameFrom.end()) {
	        id = startNode->id;
			break;
		} else {
			id = iter->second;
		}
	}
	path->push_front(id);

	// clear the frontier and explored members so that subsequent
	// calls to this function will start with the initial state.
	frontier = std::priority_queue<SearchNode*, vector<SearchNode*>, SearchNodeCostComparator>();

	closedSet.clear();
	openSet.clear();
}

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
void AstarSearch::findPath(const SearchGraph& graph, const double* const h,
                           const int startId, const int endId,
                           SearchPath* const path, const bool& dyn_flag) {
    // obtain the starting node
    SearchNode* startNode = graph.getNode(startId);
    if(!startNode) return;

    // obtain the goal node
    SearchNode* goalNode = graph.getNode(endId);
    if(!goalNode) return;

    // put the start node in the frontier
    frontier.push(startNode);
    openSet.insert(startNode);

    // map of the parent nodes that the lowest cost child node came from.
    // this will be used to reconstruct the shortest path
    map<int, int> cameFrom;

    unsigned int numNodes = graph.size();
    double f_score[numNodes];
    double g_score[numNodes];

    for(unsigned int i=0; i<numNodes; ++i) {
        f_score[i] = 0;
        g_score[i] = 0;
    }

    g_score[startNode->id] = 0;
    f_score[startNode->id] = g_score[startNode->id] + h[startNode->id];

    vector<SearchNode*> unexplored(20000);

    // the graph search loop
    while(frontier.size() > 0) {

        // select frontier node with the least cost: best first approach
        // since the frontier is ordered by cost, the lowest cost node
        // will be at the front.
        SearchNode* selectedNode = frontier.top();

        // check if we reached the goal
        if(selectedNode->id == goalNode->id) {
            break;
        }

        // did not reach the goal yet so expand the frontier:
        // remove the selectedNode from the frontier and add
        // the selectedNode's children (connected nodes)
        // to the frontier.  put the current selectedNode into the
        // open set.  update the current path cost at the
        // unexplored child nodes.  this is the current cost of the
        // selected node + pathCost from selectedNode to child +
        // child's heuristic cost.  keep track of which parent node
        // the child came from since a child can have multiple parent
        // nodes.
        //selectedNode->visited = true;
        //visited[selectedNode->id] = true;

        frontier.pop();
        openSet.erase(selectedNode);
        closedSet.insert(selectedNode);

        for(unsigned int i=0; i<unexplored.size(); ++i) {
            if(unexplored[i]) delete unexplored[i];
        }
        unexplored.clear();

        getUnexploredChildren2(selectedNode, &unexplored);

        vector<SearchNode*>::iterator iter;
        for(iter=unexplored.begin(); iter!=unexplored.end(); ++iter) {
            SearchNode* child = (*iter);

            set<SearchNode*, SearchNodeIdComparator>::iterator found = openSet.find(child);
            //CHANGED PARIKSHIT
            double g_temp = 0.0;
			if (dyn_flag)
				g_temp = g_score[selectedNode->id] +
					graph.getEdgeCost(selectedNode->id, child->id,g_score[selectedNode->id]);
			else
				g_temp = g_score[selectedNode->id] +
								graph.getEdgeCost(selectedNode->id, child->id);
            if((found == openSet.end()) || (g_temp < g_score[child->id])) {
                g_score[child->id] = g_temp;
                f_score[child->id] = g_score[child->id] + h[child->id];
                child->cost = f_score[child->id];

                cameFrom[child->id] = selectedNode->id;
                frontier.push(child);

                openSet.insert(child);
            }
        }
    }

    // reconstruct the path
    path->clear();
    int id = goalNode->id;
    map<int, int>::iterator iter;
    while(id != startNode->id) {
        path->push_front(id);
        iter = cameFrom.find(id);
        if(iter == cameFrom.end()) {
            id = startNode->id;
            break;
        } else {
            id = iter->second;
        }
    }
    path->push_front(id);

    // clear the frontier and explored members so that subsequent
    // calls to this function will start with the initial state.
    frontier = std::priority_queue<SearchNode*, vector<SearchNode*>, SearchNodeCostComparator>();

    closedSet.clear();
    openSet.clear();
}

///////////////////////////////////////////////////////////////////////////////
// private impl

/**
 * ---------------------------------------------------------------------------
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
 * ---------------------------------------------------------------------------
 */
void AstarSearch::getUnexploredChildren(SearchNode* node, vector<SearchNode*>* const unexplored) {
	if(!unexplored) return;

	multiset<SearchNode*, SearchNodeCostComparator>::iterator iter;
	set<SearchNode*, SearchNodeIdComparator>::iterator is_closed;
	for(iter=node->children.begin(); iter!=node->children.end(); ++iter) {
		SearchNode* child = (*iter);
		is_closed = closedSet.find(child);
		if(is_closed == closedSet.end()) {
			// not found in closed set so add to unexplored
			unexplored->push_back(child);
		}
	}
}

void AstarSearch::getUnexploredChildren2(SearchNode* node, vector<SearchNode*>* const unexplored) {
    if(!unexplored) return;

    multiset<SearchNode*, SearchNodeCostComparator>::iterator iter;
    set<SearchNode*, SearchNodeIdComparator>::iterator is_closed;
    for(iter=node->children.begin(); iter!=node->children.end(); ++iter) {
        SearchNode* child = (*iter);
        is_closed = closedSet.find(child);
        if(is_closed == closedSet.end()) {
            // not found in closed set so add a COPY to unexplored
            SearchNode* copy = new SearchNode(*child);
            unexplored->push_back(copy);
        }
    }
}

}
