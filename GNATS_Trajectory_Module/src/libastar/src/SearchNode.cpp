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
 * SourceNode.cpp
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#include "SearchNode.h"

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace osi {

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
bool SearchNodeIdComparator::operator() (const SearchNode* n1,
        const SearchNode* n2) const {
	return n1->id < n2->id;
}
bool SearchNodeIdComparator2::operator() (const SearchNode& n1,
        const SearchNode& n2) const {
    return n1.id < n2.id;
}

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
bool SearchNodeCostComparator::operator() (const SearchNode* n1,
        const SearchNode* n2) const {
	return n1->cost > n2->cost;
}
bool SearchNodeCostComparator2::operator() (const SearchNode& n1,
        const SearchNode& n2) const {
    return n1.cost > n2.cost;
}

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
SearchNode::SearchNode() :
	id(0),
	name(""),
	visited(false),
	cost(0),
	parents(multiset<SearchNode*, SearchNodeCostComparator>()),
	children(multiset<SearchNode*, SearchNodeCostComparator>()),
	parent(NULL) {
}

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
SearchNode::SearchNode(const SearchNode& that) :
	id(that.id),
	name(that.name),
	visited(that.visited),
	cost(that.cost),
	parents(that.parents),
	children(that.children),
	parent(that.parent) {
#ifndef NDEBUG
	cout << "SearchNode copy constructor: id=" << this->id << endl;
#endif
}

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
SearchNode::~SearchNode() {
}

} /* namespace osi */
