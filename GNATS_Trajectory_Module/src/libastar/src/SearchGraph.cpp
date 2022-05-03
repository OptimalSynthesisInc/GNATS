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
 * SearchGraph.cpp
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#include "SearchGraph.h"
#include "SearchNode.h"

#include <cstring>
#include <cstdlib>
#include <map>
#include <limits>

#include <cassert>
#include <algorithm>
#include <float.h>
#include <math.h>

using std::numeric_limits;
using std::map;

#include <iostream>
using std::endl;
using std::cout;

namespace osi {
	double VALUE_DBL_MAX = DBL_MAX;
/**
 * ---------------------------------------------------------------------------
 * Function:
 *   findNode()
 *
 * Description:
 *   find the node with the specified id from the map of nodes
 *
 * Inputs:
 *   nodes   the map of nodes
 *   id      the id of the node to find
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   pointer to the node with the specified id, or NULL if not found in
 *   the map of nodes.
 * ---------------------------------------------------------------------------
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
 *   hasConnection()
 *
 * Description:
 *   Return whether or not there is a connection to or from the node with
 *   the specified id.
 *
 * Inputs:
 *   num_nodes      number of nodes in the connectivity matrix
 *   connectivity   the connectivity matrix
 *   id             the node id
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   true if there is a connection to or from the specified node.
 * ---------------------------------------------------------------------------
 */
static bool hasConnection(const unsigned int& num_nodes, const bool* const *const connectivity, const unsigned int& id) {
	for(unsigned int i=0; i<num_nodes; ++i) {
		if(i == id) continue;
		if(connectivity[i][id] == true) return true;
		if(connectivity[id][i] == true) return true;
	}
	return false;
}

/**
 * ---------------------------------------------------------------------------
 * Function:
 *   get_search_nodes()
 *
 * Description:
 *   Obtain the list of search nodes for the given connectivity matrix.
 *   The output list is sorted in order of ascending node id.
 *   used for dfs.
 *
 * Inputs:
 *   connectivity     connectivity matrix
 *   heuristicCosts   (unused)
 *   nodeNames        array of node names
 *   num_nodes        number of nodes (dimension of connectivity and nodeNames)
 *
 * In/Out:
 *   nodes   map of search nodes, keyed by id.
 *
 * Returns:
 *   0
 * ---------------------------------------------------------------------------
 */
static int get_search_nodes(const bool* const *const connectivity,
		                    const double* const heuristicCosts,
		                    const string* const nodeNames,
		                    const unsigned int num_nodes,
		                    map<int, SearchNode*>* const nodes) {

	(void)heuristicCosts;

	for(unsigned int i=0; i<num_nodes; ++i) {
		// see if the node has already been created.  if not, add a new one.
		// we need to check this because a search node may have been added
		// to the nodes vector in the check for children.
		// only add the node if it is connected to something.
		if(!hasConnection(num_nodes, connectivity, i)) continue;
		SearchNode* node = findNode(*nodes, i);
		if(node == NULL) {
			node = new SearchNode();
			node->id = i;
			node->cost = 0;//heuristicCosts[i];
			node->name = nodeNames[i];
			(*nodes)[i] = node;
		}

		// check for children
		for(unsigned int j=0; j<num_nodes; ++j) {
			bool value = connectivity[i][j];
			if(value) {
				SearchNode* child = findNode(*nodes, j);
				if(child == NULL) {
					child = new SearchNode();
					child->id = j;
					child->cost = 0;
					child->name = nodeNames[j];
					(*nodes)[j] = child;
				}
				child->parents.insert(node);
				node->children.insert(child);
			}
		}
	}
	return 0;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   SearchGraph()
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
SearchGraph::SearchGraph() :
	connectivity(NULL),
	pathCosts(NULL),
	edgeCosts(map< std::pair<int,int>, double >()),
	edgeCostsVector(map< std::pair<int,int>, vector<double> >()),
	heuristicCosts(NULL),
	nodeNames(NULL),
	removedFlags(NULL),
	multipliedFlags(NULL),
	max_nodes(0),
	max_cost(0) {
	connectivity = NULL;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   SearchGraph()
 *
 * Description:
 *   Construct a search graph with the given number of nodes and the
 *   given connectivity, path costs, heuristic costs, and node names.
 *
 * Inputs:
 *   numNodes         number of nodes in the graph
 *   connectivity     connectivity matrix
 *   pathCosts        edge cost matrix (optional, default=NULL)
 *   heuristicCosts   heuristic cost array (optional, default=NULL)
 *   nodeNames        node names array (optional, default=NULL)
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchGraph::SearchGraph(const int& numNodes,
		                 const bool* const * const connectivity,
		                 const double* const * const pathCosts,
		                 const double* const heuristicCosts,
		                 const string* const nodeNames) :
	nodes(map<int, SearchNode*>()),
	connectivity(NULL),
	pathCosts(NULL),
	edgeCosts(map< std::pair<int,int>, double >()),
	edgeCostsVector(map< std::pair<int,int>, vector<double> >()),
	heuristicCosts(NULL),
	nodeNames(NULL),
	removedFlags(NULL),
	multipliedFlags(NULL),
	max_nodes(0),
	max_cost(0) {

	if(numNodes > 0) {
		this->set(numNodes, connectivity, pathCosts, heuristicCosts, nodeNames, -1e16,0);
	}
}

SearchGraph::SearchGraph(const SearchGraph& that) :
	nodes(map<int, SearchNode*>()),
	connectivity(NULL),
	pathCosts(NULL),
	edgeCosts(map< std::pair<int,int>, double >()),
	edgeCostsVector(map< std::pair<int,int>, vector<double> >()),
	heuristicCosts(NULL),
	nodeNames(NULL),
	removedFlags(NULL),
	multipliedFlags(NULL),
	max_nodes(0),
	max_cost(0) {

	this->set3(that.size(), that.connectivity, that.edgeCosts, that.edgeCostsVector,
			that.heuristicCosts, that.nodeNames, that.max_cost,that.multipliedFlags);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   ~SearchGraph()
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
SearchGraph::~SearchGraph() {
	// free the connectivity, pathCosts, and heuristicCosts matrices
	reset();
}

/**
 * ---------------------------------------------------------------------------
 * Function:
 *   reset()
 *
 * Description:
 *   Reset the data members.  This will free/delete the arrays.
 *
 * Inputs:
 *   none
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * ---------------------------------------------------------------------------
 */
void SearchGraph::reset() {
	if(this->connectivity) {
		for(int i=0; i<max_nodes; ++i) {
			free(this->connectivity[i]);
		}
		free(this->connectivity);
	}

	if(this->heuristicCosts) {
		free(this->heuristicCosts);
	}
	if(this->nodeNames) {
		delete [] this->nodeNames;
	}
	if(this->removedFlags) {
		free(this->removedFlags);
	}

	if(this->multipliedFlags) {
		for(int i=0; i<max_nodes; ++i) {
			free(this->multipliedFlags[i]);
		}
		free(this->multipliedFlags);
	}
	// delete nodes from the mapping.
	map<int, SearchNode*>::iterator iter;
	for(iter=nodes.begin(); iter!=nodes.end(); ++iter) {
		SearchNode* node = iter->second;
		if(node) {
			delete node;
			node = NULL;
		}
	}
	nodes.clear();

	max_nodes = 0;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   set()
 *
 * Description:
 *   Set this graph's data members
 *
 * Inputs:
 *   numNodes         number of nodes in the graph
 *   connectivity     connectivity matrix
 *   pathCosts        edge cost matrix (optional, default=NULL)
 *   heuristicCosts   heuristic cost array (optional, default=NULL)
 *   nodeNames        node names array (optional, default=NULL)
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::set(const int& numNodes,
		              const bool* const *const connectivity,
	                  const double* const *const pathCosts,
	                  const double* const heuristicCosts,
	                  const string* const nodeNames,
	                  const double& maxCost,
					  const unsigned int& num_wind_comp) {
	if (numNodes <= 0) return;

	reset();

	max_nodes = numNodes;
	max_cost = maxCost;

	this->connectivity = (bool**)calloc(max_nodes, sizeof(bool*));
	for(int i=0; i<max_nodes; ++i) {
		this->connectivity[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memcpy(this->connectivity[i], connectivity[i], max_nodes*sizeof(bool));
	}

	edgeCosts.clear();
	if (pathCosts) {
		for (int i = 0; i < max_nodes; ++i) {
			for (int j = 0; j < max_nodes; ++j) {
				if (this->connectivity[i][j]) {
					edgeCosts.insert( std::pair< std::pair<int,int>, double >(std::pair<int,int>(i,j), pathCosts[i][j] ));
				}
			}
		}
	}

	edgeCostsVector.clear();
	if (pathCosts) {
		for(int i=0; i<max_nodes; ++i) {
			for(int j=0; j<max_nodes; ++j) {
				if(this->connectivity[i][j]) {
					vector<double> costsVec;costsVec.clear();
					if (num_wind_comp !=0 ){
						costsVec.resize(num_wind_comp);
						std::fill(costsVec.begin(),costsVec.end(),pathCosts[i][j]);
					}
					else
						costsVec.push_back(pathCosts[i][j]);
					edgeCostsVector.insert(
							std::pair< std::pair<int,int>, vector<double>  >
										(std::pair<int,int>(i,j), costsVec ));
				}
			}
		}
	}

	this->heuristicCosts = (double*)calloc(max_nodes, sizeof(double));
	if (heuristicCosts) {
		memcpy(this->heuristicCosts, heuristicCosts, max_nodes*sizeof(double));
	}

	this->nodeNames = new string[max_nodes];
	if (nodeNames) {
		for(int i = 0; i < max_nodes; ++i) {
			this->nodeNames[i] = nodeNames[i];
		}
	}

	this->removedFlags = (bool*)calloc(max_nodes, sizeof(bool));
	memset(this->removedFlags, 0, max_nodes*sizeof(bool));

	this->multipliedFlags = (bool**)calloc(max_nodes, sizeof(bool*));
	for (int i=0; i<max_nodes; ++i) {
		this->multipliedFlags[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memset(this->multipliedFlags[i], 0, max_nodes*sizeof(bool));
	}

	// create and add search nodes for the connectivity matrix
	get_search_nodes(this->connectivity,
			this->heuristicCosts,
			this->nodeNames,
			max_nodes,
			&nodes);
}

void SearchGraph::set2(const int& numNodes,
		              const bool* const *const connectivity,
	                  const map< std::pair<int,int>, double >& edgeCosts,
	                  const double* const heuristicCosts,
	                  const string* const nodeNames,
	                  const double& maxCost) {
	if(numNodes <= 0) return;

	reset();

	max_nodes = numNodes;
	max_cost = maxCost;

	this->connectivity = (bool**)calloc(max_nodes, sizeof(bool*));
	for(int i=0; i<max_nodes; ++i) {
		this->connectivity[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memcpy(this->connectivity[i], connectivity[i], max_nodes*sizeof(bool));
	}

	this->edgeCosts.clear();
	this->edgeCosts.insert(edgeCosts.begin(), edgeCosts.end());

	this->heuristicCosts = (double*)calloc(max_nodes, sizeof(double));
	if(heuristicCosts) {
		memcpy(this->heuristicCosts, heuristicCosts, max_nodes*sizeof(double));
	}

	this->nodeNames = new string[max_nodes];
	if(nodeNames) {
		for(int i=0; i<max_nodes; ++i) {
			this->nodeNames[i] = nodeNames[i];
		}
	}

	this->removedFlags = (bool*)calloc(max_nodes, sizeof(bool));
	memset(this->removedFlags, 0, max_nodes*sizeof(bool));

	this->multipliedFlags = (bool**)calloc(max_nodes, sizeof(bool*));
	for(int i=0; i<max_nodes; ++i) {
		this->multipliedFlags[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memset(this->multipliedFlags[i], 0, max_nodes*sizeof(bool));
	}

	// create and add search nodes for the connectivity matrix
	get_search_nodes(this->connectivity, this->heuristicCosts, this->nodeNames,
			max_nodes, &nodes);
}

void SearchGraph::set3(const int& numNodes,
		              const bool* const *const connectivity,
	                  const map< std::pair<int,int>, double >& edgeCosts,
					  const map< std::pair<int,int>, vector<double> >& edgeCostsVector,
	                  const double* const heuristicCosts,
	                  const string* const nodeNames,
	                  const double& maxCost,
					  const bool* const *const multipliedFlags) {
	if(numNodes <= 0) return;

	reset();

	max_nodes = numNodes;
	max_cost = maxCost;

	this->connectivity = (bool**)calloc(max_nodes, sizeof(bool*));
	for(int i=0; i<max_nodes; ++i) {
		this->connectivity[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memcpy(this->connectivity[i], connectivity[i], max_nodes*sizeof(bool));
	}

	this->edgeCosts.clear();
	this->edgeCosts.insert(edgeCosts.begin(), edgeCosts.end());

	this->edgeCostsVector.clear();
	this->edgeCostsVector.insert(edgeCostsVector.begin(), edgeCostsVector.end());


	this->heuristicCosts = (double*)calloc(max_nodes, sizeof(double));
	if(heuristicCosts) {
		memcpy(this->heuristicCosts, heuristicCosts, max_nodes*sizeof(double));
	}

	this->nodeNames = new string[max_nodes];
	if(nodeNames) {
		for(int i=0; i<max_nodes; ++i) {
			this->nodeNames[i] = nodeNames[i];
		}
	}

	this->removedFlags = (bool*)calloc(max_nodes, sizeof(bool));
	memset(this->removedFlags, 0, max_nodes*sizeof(bool));

	this->multipliedFlags = (bool**)calloc(max_nodes, sizeof(bool*));
	for(int i=0; i<max_nodes; ++i) {
		this->multipliedFlags[i] = (bool*)calloc(max_nodes, sizeof(bool));
		memcpy(this->multipliedFlags[i], multipliedFlags[i], max_nodes*sizeof(bool));
	}

	// create and add search nodes for the connectivity matrix
	get_search_nodes(this->connectivity, this->heuristicCosts, this->nodeNames,
			max_nodes, &nodes);
}
/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   addNode()
 *
 * Description:
 *   add a copy of the node to the graph.  this will
 *   add the node to the mapping of all nodes and to
 *   the open-list.
 *
 * Inputs:
 *   node   a reference to the node to add
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::addNode(const SearchNode& node) {
	// copy the node into a new node
	SearchNode* n = new SearchNode();
	*n = node;

	// add the new node to the mapping of all nodes and
	// to the openList
	nodes[n->id] = n;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   removeNode()
 *
 * Description:
 *   Remove a node from the search graph.
 *   NOTE: this does not actually remove the node from memory or from the
 *   data structure.  It simply sets the cost to and from the node to
 *   'infinity'
 *
 * Inputs:
 *   id   id of the node to remove
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::removeNode(const int& id,
							const double& set_cost,
							const bool& is_multiplier,
							const int& st_hr,
							const int& end_hr) {

    // don't remove if already removed.
    if(removedFlags[id] == true) {
        return;
    }

	// find the node
	SearchNode* node = findNode(nodes, id);
	if(!node) {
		cout << "ERROR: SearchGraph::removeNode - no node with id " << id << endl;
		return;
	}

	//if(id < 0 || id > (int)nodes.size()) {
	if(id < 0 || id > max_nodes) {
		cout << "ERROR: SearchGraph::removeNode invalid node id " << id << endl;
		return;
	}

	// remove all connectivity

	map<int, SearchNode*>::iterator iter;
	for(iter=nodes.begin(); iter!=nodes.end(); ++iter) {
		int otherId = iter->first;

		//Do not multiply if already multiplied
		if ( (this->multipliedFlags[id][otherId]|| this->multipliedFlags[otherId][id])
				&& is_multiplier)
			continue;

		//TODO:PARIKSHIT ADDER. KEY TO WEATHER REROUTES
		if (!this->hasLink(id,otherId))
			continue;

		double cost_to_assign1 = set_cost;
		if (is_multiplier)
			cost_to_assign1 = set_cost*edgeCosts[std::pair<int,int>(id,otherId)];

		edgeCosts[std::pair<int,int>(id,otherId)] =
				(edgeCosts[std::pair<int,int>(id,otherId)] > cost_to_assign1) ?
						edgeCosts[std::pair<int,int>(id,otherId)] : cost_to_assign1;

		vector<double> eCV1 = edgeCostsVector[std::pair<int,int>(id,otherId)];
		unsigned int sz = edgeCostsVector[std::pair<int,int>(id,otherId)].size();
		if (sz<=1)
			continue;

		for(unsigned int i = (unsigned int)st_hr;i< MIN(sz,(unsigned int)end_hr);++i){
			if (is_multiplier){
				edgeCostsVector[std::pair<int,int>(id,otherId)].at(i) =
						edgeCostsVector[std::pair<int,int>(id,otherId)].at(i)*set_cost;
				this->multipliedFlags[id][otherId] = true;
			}
			else{
				edgeCostsVector[std::pair<int,int>(id,otherId)].at(i) = set_cost;
			}
		}
	}
	if (set_cost >1e10)
		removedFlags[id] = true;


}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   addLink()
 *
 * Description:
 *   Add a link from the node with source id to the node with sink id.
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::addLink(const int& sourceId, const int& sinkId) {
	if(connectivity) {
		connectivity[sourceId][sinkId] = true;
	}
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   removeLink()
 *
 * Description:
 *   Remove a link from the node with source id to the node with sink id.
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::removeLink(const int& sourceId, const int& sinkId) {
	if(connectivity) {
		connectivity[sourceId][sinkId] = false;
	}
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   hasLink()
 *
 * Description:
 *   Returns whether or not there is a link between the specified nodes
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   1 if there is a connection from source to sink, 0 otherwise.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
bool SearchGraph::hasLink(const int& sourceId, const int& sinkId) const {
	if(connectivity) {
		return connectivity[sourceId][sinkId];
	}
	return 0;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getNode()
 *
 * Description:
 *   Get the specified node from the mapping of all nodes.
 *
 * Inputs:
 *   id   id of the node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   Pointer to the search node with the specified id.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchNode* SearchGraph::getNode(const int& id) const {
	return findNode(nodes, id);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getEdgeCost()
 *
 * Description:
 *   Get the edge cost between nodes with the specified ids
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   the edge cost between nodes with the specified ids
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double& SearchGraph::getEdgeCost(const int& sourceId, const int& sinkId,
		const double& tval) const {
	std::pair<int,int> key(sourceId, sinkId);

	if (tval < 0) {
		if (edgeCosts.find(key) != edgeCosts.end())
			return const_cast<double&>(edgeCosts.at(key));
		else
			return const_cast<double&>(VALUE_DBL_MAX);
	} else {
		vector<double>& ecosts = this->getEdgeCostVector(sourceId,sinkId);
		if (ecosts.size() <=1)
			return const_cast<double&>(edgeCosts.at(key));

		unsigned int idx = (unsigned int)( floor(tval/3600) )% ( ecosts.size() );
		return (ecosts.at(idx));
	}
}
/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getEdgeCostVector()
 *
 * Description:
 *   Get the edge cost vector between nodes with the specified ids
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   the edge cost between nodes with the specified ids
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
vector<double>& SearchGraph::getEdgeCostVector(const int& sourceId, const int& sinkId) const {
	std::pair<int,int> key(sourceId, sinkId);

	return const_cast<vector<double>&>( edgeCostsVector.at(key) );
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getEdgeCostVectorSize()
 *
 * Description:
 *   Get the edge cost vector size between nodes with the specified ids
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   the edge cost vector size between nodes with the specified ids
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
unsigned int SearchGraph::getEdgeCostVectorSize(const int& sourceId, const int& sinkId) const {
	unsigned int retValue = 0;

	std::pair<int,int> key(sourceId, sinkId);

	if (edgeCostsVector.find(key) != edgeCostsVector.end())
		retValue = edgeCostsVector.at(key).size();

	return retValue;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getHeuristicCost()
 *
 * Description:
 *   Get the stored heuristic cost of the node with the specified id
 *
 * Inputs:
 *   id   id of the desired node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   the stored heuristic cost of the specified node
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double& SearchGraph::getHeuristicCost(const int& id) const {
	return heuristicCosts[id];
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getMaxPathCost()
 *
 * Description:
 *   Get the maximum edge cost of the graph
 *
 * Inputs:
 *   none
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   the maximum edge cost of the graph
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
const double& SearchGraph::getMaxEdgeCost() const {
	return max_cost;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   setPathCost()
 *
 * Description:
 *   Set the edge cost between the nodes with the specified ids to the
 *   specified cost
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *   cost       the new edge cost to set
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::setEdgeCost(const int& sourceId, const int& sinkId, const double& cost) {
	if (edgeCosts.find(std::pair<int,int>(sourceId, sinkId)) != edgeCosts.end())
		edgeCosts.at(std::pair<int,int>(sourceId, sinkId)) = cost;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   setPathCost()
 *
 * Description:
 *   Set the edge cost between the nodes with the specified ids to the
 *   specified cost
 *
 * Inputs:
 *   sourceId   id of the start node
 *   sinkId     id of the end node
 *   cost       the new edge cost to set
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::setEdgeCostVector(const int& sourceId, const int& sinkId,
		const vector<double>& cost) {
	if (edgeCostsVector.find(std::pair<int,int>(sourceId, sinkId)) != edgeCostsVector.end())
		edgeCostsVector.at(std::pair<int,int>(sourceId, sinkId)) = cost;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   setHeuristicCost()
 *
 * Description:
 *   Set the heuristic cost of the node with the specified id
 *
 * Inputs:
 *   id   id of the node
 *   cost       the new heuristic cost to set
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::setHeuristicCost(const int& id, const double& cost) {
	heuristicCosts[id] = cost;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   isRemoved()
 *
 * Description:
 *   Return whether or not a node has been marked as removed from the
 *   search graph.
 *
 * Inputs:
 *   id   id of the node to test
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   true if the node has been marked as removed from the graph,
 *   false otherwise.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
bool& SearchGraph::isRemoved(const int& id) const {
	return removedFlags[id];
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   isMultiplied()
 *
 * Description:
 *   Return whether or not an edge has been marked as multiplied by a weight in the
 *   search graph.
 *
 * Inputs:
 *   id 1  id of node 1
 *	 id 2  id of node 2
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   true if the node has been marked as multiplied by a weight in the graph,
 *   false otherwise.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
bool& SearchGraph::isMultiplied(const int& id1,const int&id2) const {
	return multipliedFlags[id1][id2];
}
/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   setMultiplied()
 *
 * Description:
 *   Sets multipliedFlag for an edge
 *
 * Inputs:
 *   id 1  id of node 1
 *	 id 2  id of node 2
 *	 val value to set.
 * In/Out:
 *   none
 *
 * Returns:
 * 	none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchGraph::setMultiplied(const int& id1,const int&id2, bool val) const {
	this->multipliedFlags[id1][id2] = val;
}
/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   getNodes()
 *
 * Description:
 *   Get a constant reference to the map of search nodes
 *
 * Inputs:
 *   id   id of the node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   Constant reference to the map of search nodes
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
const map<int, SearchNode*>& SearchGraph::getNodes() const {
	return nodes;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   size()
 *
 * Description:
 *   Get the number of nodes in the graph
 *
 * Inputs:
 *   id   id of the node
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   number of nodes in the graph
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchGraph::size() const {
	return max_nodes;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   operator=()
 *
 * Description:
 *   overloaded assignment operator
 *
 * Inputs:
 *   that   constant reference to the graph to assign to this graph
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   A reference to this graph
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchGraph& SearchGraph::operator=(const SearchGraph& that) {
	if(this == &that) return *this;

	// free existing data
	reset();

	// set new data
	set3(that.max_nodes, that.connectivity,
		that.edgeCosts, that.edgeCostsVector,that.heuristicCosts, that.nodeNames, that.max_cost,
		that.multipliedFlags);

	return *this;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   edgeCostsBegin()
 *
 * Description:
 *   Return a map< std::pair<int,int>, double >::iterator that points
 *   to the beginning of the edgeCosts map.
 *
 * Inputs:
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
map< std::pair<int,int>, double >::iterator SearchGraph::edgeCostsBegin() {
	return edgeCosts.begin();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   edgeCostsEnd()
 *
 * Description:
 *   Return a map< std::pair<int,int>, double >::iterator that points
 *   to the end of the edgeCosts map.
 *
 * Inputs:
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
map< std::pair<int,int>, double >::iterator SearchGraph::edgeCostsEnd() {
	return edgeCosts.end();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   edgeCostsVectorBegin()
 *
 * Description:
 *   Return a map< std::pair<int,int>, vector<double> >::iterator that points
 *   to the beginning of the edgeCostsVector map.
 *
 * Inputs:
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
map< std::pair<int,int>, vector<double> >::iterator SearchGraph::edgeCostsVectorBegin() {
	return edgeCostsVector.begin();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   edgeCostsVectorEnd()
 *
 * Description:
 *   Return a map< std::pair<int,int>, vector<double> >::iterator that points
 *   to the end of the edgeCostsVector map.
 *
 * Inputs:
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
map< std::pair<int,int>, vector<double>  >::iterator SearchGraph::edgeCostsVectorEnd() {
	return edgeCostsVector.end();
}

int SearchGraph::numEdges() {
	assert(edgeCosts.size() == edgeCostsVector.size());
	return edgeCosts.size();
}

}
