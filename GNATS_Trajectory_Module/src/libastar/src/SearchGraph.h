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
 * SearchGraph.h
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#ifndef SEARCHGRAPH_H_
#define SEARCHGRAPH_H_

#include "SearchNode.h"

#include <map>
#include <limits>
#include <vector>

#define DEFAULT_ALT 30000

using std::map;
using std::numeric_limits;
using std::vector;

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

namespace osi {

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   SearchGraph
 *
 * Description:
 *   Graph of search nodes to be used for A* and DFS searches.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class SearchGraph {
public:

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchGraph();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchGraph(const int& numNodes, const bool* const *const connectivity,
			    const double* const *const pathCosts=NULL,
			    const double* const heuristicCosts=NULL,
			    const string* const nodeNames=NULL);

	SearchGraph(const SearchGraph& that);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	virtual ~SearchGraph();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void addNode(const SearchNode& node);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 *   id   id of the node to remove,
	 *   cost cost to be set after link removal (default = infinity)
	 *   is_mult if the cost in the previous arg is to be multiplied to the link cost or not.
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void removeNode(const int& id,const double& set_cost = numeric_limits<double>::max(),
			const bool& is_multiplier = false, const int& st_hr = 0, const int& end_hr = 24);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void addLink(const int& sourceId, const int& sinkId);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void removeLink(const int& sourceId, const int& sinkId);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool hasLink(const int& sourceId, const int& sinkId) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchNode* getNode(const int& id) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const map<int, SearchNode*>& getNodes() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int size() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 * TODO:PARIKSHIT Changed from getPathCost()
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	double& getEdgeCost(const int& sourceId, const int& sinkId,const double& tval = -1.0) const;
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   getEdgeCostVector()
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
	 *   the edge cost vector between nodes with the specified ids
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */

	vector<double>& getEdgeCostVector(const int& sourceId, const int& sinkId) const;
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   getEdgeCostVectorSize()
	 *
	 * Description:
	 *   Get the edge cost size between nodes with the specified ids
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	unsigned int getEdgeCostVectorSize(const int& sourceId, const int& sinkId) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	double& getHeuristicCost(const int& id) const;
	double* getHeuristicCosts() const {return heuristicCosts;}

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 * TODO:PARIKSHIT changed from getMaxPathCost()
	 *   getMaxEdgeCost()
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const double& getMaxEdgeCost() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 * TODO:PARIKSHIT changed. Previously was setPathCost()
	 * setEdgeCost()
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setEdgeCost(const int& sourceId,
			         const int& sinkId,
			         const double& cost);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 * setEdgeCostVector()
	 *
	 * Description:
	 *   Set the edge cost between the nodes with the specified ids to the
	 *   specified cost
	 *
	 * Inputs:
	 *   sourceId   id of the start node
	 *   sinkId     id of the end node
	 *   cost       the new edge cost vector to set
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setEdgeCostVector(const int& sourceId,
			         const int& sinkId,
			         const vector<double>& cost	);

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
	map< std::pair<int,int>, double >::iterator edgeCostsBegin();

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
	map< std::pair<int,int>, double >::iterator edgeCostsEnd();
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   edgeCostsVectorBegin()
	 *
	 * Description:
	 *   Return a map< std::pair<int,int>, vector<double> > ::iterator that points
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
	map< std::pair<int,int>, vector<double>  >::iterator edgeCostsVectorBegin();

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
	map< std::pair<int,int>, vector<double> >::iterator edgeCostsVectorEnd();

	int numEdges();

	int getMaxNodes() {return max_nodes;}

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setHeuristicCost(const int& id, const double& cost);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool& isRemoved(const int& id) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   isMultiplied()
	 *
	 * Description:
	 *   Return whether or not a node has been marked as multiplied by a weight from the
	 *   search graph.
	 *
	 * Inputs:
	 *   id   id of the node to test
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   true if the node has been marked as multiplied by a weight from the graph,
	 *   false otherwise.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool& isMultiplied(const int& id1,const int&id2) const;

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
	 *	 val   value to set.
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 * 	none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setMultiplied(const int& id1,const int&id2, bool val) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void set(const int& numNodes, const bool* const *const connectivity,
		     const double* const *const pathCosts=NULL,
		     const double* const heuristicCosts=NULL,
		     const string* const nodeNames=NULL,
		     const double& maxCost=numeric_limits<double>::min(),
			 const unsigned int& num_wind_comp = 22);
	void set2(const int& numNodes, const bool* const *const connectivity,
		     const map< std::pair<int,int>, double >& edgeCosts,
		     const double* const heuristicCosts=NULL,
		     const string* const nodeNames=NULL,
		     const double& maxCost=numeric_limits<double>::min());
	void set3(const int& numNodes,
			const bool* const *const connectivity,
			const map< std::pair<int,int>, double >& edgeCosts,
			const map< std::pair<int,int>, vector<double> >& edgeCostsVector,
			const double* const heuristicCosts,
			const string* const nodeNames,
			const double& maxCost,
			const bool* const *const multipliedFlags);


	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchGraph& operator=(const SearchGraph& that);

private:

	/**
	 * -----------------------------------------------------------------------
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
	 * -----------------------------------------------------------------------
	 */
	void reset();

private:

	/*
	 * Mapping of all nodes, keyed by node id.
	 */
	map<int, SearchNode*> nodes;

	// size of these arrays corresponds to max num nodes.  this is
	// equal to the size of g_fixes:

	/*
	 * Connectivity matrix. dimension: max_nodes by max_nodes
	 */
	bool** connectivity;

	/*
	 * Edge cost matrix.  dimension: max_nodes by max_nodes
	 */
	double** pathCosts;
	map< std::pair<int,int>, double > edgeCosts;

	/*
	 * Edge cost tensor.  dimension: max_nodes by max_nodes by wind forecast time
	 * For variable cost edges.
	 * (fromNode,toNode)---> [altitude x time]
	 */

	map< std::pair<int,int>, vector<double> > edgeCostsVector;

	/*
	 * Heuristic cost array.  dimension: max_nodes
	 */
	double* heuristicCosts;

	/*
	 * Node names array.  dimension: max_nodes
	 */
	string* nodeNames;

	/*
	 * Array of removed flags.  dimension: max_nodes
	 */
	bool* removedFlags;

	/*
	 * Array of multiplied flags.  dimension: max_nodes
	 */
	bool** multipliedFlags;

	/*
	 * Maximum size of matrices and arrays.
	 */
	int max_nodes;

	/*
	 * Maximum edge cost
	 */
	double max_cost;
};

}

#endif /* SEARCHGRAPH_H_ */
