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
 * SearchNode.h
 *
 *  Created on: May 6, 2012
 *      Author: jason
 */

#ifndef SEARCHNODE_H_
#define SEARCHNODE_H_


#include <string>
#include <set>

using std::string;
using std::multiset;

namespace osi {

// Forward Declarations
class SearchNode;


/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   SearchNodeIdComparator
 *
 * Description:
 *   Compare Node IDs
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class SearchNodeIdComparator {
public:
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   operator()
	 *
	 * Description:
	 *   Overloaded () operator.
     *   return true if n1.id less than n2.id.  ascending or descending
     *   order doesn't really matter here.  the important thing is that
     *   the set or multiset is ordered so that we can do binary searching
     *
     * Inputs:
     *   n1   node 1
     *   n2   node 2
     *
     * In/Out:
     *   none
     *
     * Returns:
     *   true if n1.id less than n2.id, false otherwise.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool operator() (const SearchNode* n1,
			        const SearchNode* n2) const;
};
class SearchNodeIdComparator2 {
public:
    /**
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * Function:
     *   operator()
     *
     * Description:
     *   Overloaded () operator.
     *   return true if n1.id less than n2.id.  ascending or descending
     *   order doesn't really matter here.  the important thing is that
     *   the set or multiset is ordered so that we can do binary searching
     *
     * Inputs:
     *   n1   node 1
     *   n2   node 2
     *
     * In/Out:
     *   none
     *
     * Returns:
     *   true if n1.id less than n2.id, false otherwise.
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     */
    bool operator()(const SearchNode& n1,
                    const SearchNode& n2) const;
};

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   SearchNodeCostComparator
 *
 * Description:
 *   Compare Node Costs
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class SearchNodeCostComparator {
public:
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   operator()
	 *
	 * Description:
	 *   Overloaded () operator.
     *   return true if n1.cost greater than n2.cost so that the lowest
     *   cost node will be at the front of a priority queue.
     *   (STL priority queues put the largest value at the front if
     *   the less-than operator is used).
     *
     * Inputs:
     *   n1   node 1
     *   n2   node 2
     *
     * In/Out:
     *   none
     *
     * Returns:
     *   true if n1.cost greater than n2.cost, false otherwise.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool operator()(const SearchNode* n1,
			        const SearchNode* n2) const;
};

class SearchNodeCostComparator2 {
public:
    /**
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * Function:
     *   operator()
     *
     * Description:
     *   Overloaded () operator.
     *   return true if n1.cost greater than n2.cost so that the lowest
     *   cost node will be at the front of a priority queue.
     *   (STL priority queues put the largest value at the front if
     *   the less-than operator is used).
     *
     * Inputs:
     *   n1   node 1
     *   n2   node 2
     *
     * In/Out:
     *   none
     *
     * Returns:
     *   true if n1.cost greater than n2.cost, false otherwise.
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     */
    bool operator()(const SearchNode& n1,
                    const SearchNode& n2) const;
};

/*
 * Typedefs
 */
typedef multiset<SearchNode*, SearchNodeCostComparator> CostOrderedSet;
typedef multiset<SearchNode*, SearchNodeIdComparator> IdOrderedSet;


/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   SearchNode
 *
 * Description:
 *   A node of the SearchGraph
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class SearchNode {
public:

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   SearchNode()
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
	SearchNode();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   SearchNode()
	 *
	 * Description:
	 *   Copy constructor
	 *
	 * Inputs:
	 *   that   the search node to copy into the new node
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchNode(const SearchNode& that);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   ~SearchNode()
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
	virtual ~SearchNode();

public:

	/*
	 * Node ID
	 */
	int id;

	/*
	 * Node name
	 */
	string name;

	/*
	 * Flag indicating if the node has been visited
	 */
	bool visited;

	/*
	 * Node cost
	 */
	double cost;

	/*
	 * Set of parent nodes ordered by cost
	 */
	CostOrderedSet parents;

	/*
	 * Set of child nodes ordered by cost
	 */
	CostOrderedSet children;

	/**
	 * Parent node used to reconstruct the search path
	 */
	SearchNode* parent;
};

}

#endif /* SEARCHNODE_H_ */
