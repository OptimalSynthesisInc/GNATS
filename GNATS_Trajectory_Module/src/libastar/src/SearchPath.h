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
 * SearchPath.h
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#ifndef SEARCHPATH_H_
#define SEARCHPATH_H_

#include "SearchNode.h"

#include <deque>

using std::deque;

namespace osi {

/*
 * Typedefs
 */
typedef deque<int>::iterator SearchPathIterator;
typedef deque<int>::const_iterator SearchPathConstIterator;

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class:
 *   SearchPath
 *
 * Description:
 *   This class contains the sequence of node IDs that define a path through
 *   a search graph.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
class SearchPath {
public:

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   SearchPath()
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
	SearchPath();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   SearchPath()
	 *
	 * Description:
	 *   Copy constructor
	 *
	 * Inputs:
	 *   that   the search path to copy into a new SearchPath
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchPath(const SearchPath& that);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   SearchPath()
	 *
	 * Description:
	 *   Construct a new search path from a subpath
	 *
	 * Inputs:
	 *   first  iterator of the start element in the subpath to copy
	 *   last   iterator of the last element in the subpath to copy
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchPath(SearchPathIterator& first, SearchPathIterator& last);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   ~SearchPath()
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
	virtual ~SearchPath();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   push_front()
	 *
	 * Description:
	 *   push a new node id to the front of the path
	 *
	 * Inputs:
	 *   node   id of the node to push to the front of the path
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void push_front(int node);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   push_back()
	 *
	 * Description:
	 *   push a new node id to the back of the path
	 *
	 * Inputs:
	 *   node   id of the node to push to the back of the path
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void push_back(int node);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   pop_front()
	 *
	 * Description:
	 *   pop a node id from the front of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   the id of the node popped from the front of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int pop_front();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   pop_back()
	 *
	 * Description:
	 *   pop a node id from the back of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   the id of the node popped from the back of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int pop_back();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   front()
	 *
	 * Description:
	 *   return a reference to the front of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   a reference to the id of the node at the front of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int& front();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   back()
	 *
	 * Description:
	 *   return a reference to the back of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   a reference to the id of the node at the back of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int& back();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   begin()
	 *
	 * Description:
	 *   return an iterator to the beginning of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   return an iterator to the beginning of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchPathIterator begin();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   end()
	 *
	 * Description:
	 *   return an iterator to the end of the path.  end points to one
	 *   past the last element.
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   return an iterator to the end of the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	SearchPathIterator end();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   clear()
	 *
	 * Description:
	 *   remove all elements from the path
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
	void clear();

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   erase()
	 *
	 * Description:
	 *   erase the sequence from iterator begin to iterator end.  This
	 *   erases the range [begin, end)
	 *
	 * Inputs:
	 *   start   iterator pointing to the start of the range to erase
	 *   end     iterator pointing to the end of the range to erase
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void erase(SearchPathIterator start, SearchPathIterator end);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   insert()
	 *
	 * Description:
	 *   insert range into deque of nodes
	 *
	 * Inputs:
	 *   pos   position to start inserting
	 *   begin  iterator to beginning of range to insert
	 *   end    iterator to end of range to insert
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void insert(SearchPathIterator pos, SearchPathIterator begin, SearchPathIterator end);


	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   firstIndexOf()
	 *
	 * Description:
	 *   find the index first instance of the specified id in the path
	 *
	 * Inputs:
	 *   id   id of the node to find the index of
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   the index of the first instance of the specified id in the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int firstIndexOf(const int id) const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   size()
	 *
	 * Description:
	 *   get the number of elements in the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   the number of elements in the path
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int size() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   getNodes()
	 *
	 * Description:
	 *   get a reference to the raw data of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   a constant reference to the raw data of the path stored as a
	 *   deque<int>.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const deque<int>& getNodes() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   getCostSequence()
	 *
	 * Description:
	 *   get a reference to the cost sequence of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   a constant reference to the raw cost sequence of the path stored as a
	 *   deque<double>.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const deque<double>& getCostSequence() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   setCostSequence()
	 *
	 * Description:
	 *   set a reference to the cost sequence of the path
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   None
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setCostSequence(const deque<double>& costSequence);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   operator[]()
	 *
	 * Description:
	 *   Overloaded operator[].  Returns the element at the ith position
	 *   in the deque
	 *
	 * Inputs:
	 *   i   index of the element in the deque
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   the element at the ith position in the deque.
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	int operator[](const int i);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   operator<()
	 *
	 * Description:
	 *   Overloaded operator<.  Compares pointer values
	 *
	 * Inputs:
	 *   that  the other SearchPath to compare
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   return -1 if this is < that, 0 if this==that, or 1 if this > that
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	bool operator<(const SearchPath& that) const;

	/*
	 *TODO:PARIKSHIT ADDER FOR TERMINAL AREA PROCEDURES.
	 */
	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   setSid()
	 *
	 * Description:
	 *   set the Sids
	 *
	 * Inputs:
	 *   SID description string
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setSid(const string& sid);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   setStar()
	 *
	 * Description:
	 *   set the Stars
	 *
	 * Inputs:
	 *   STAR description string
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setStar(const string& star);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   setApproach()
	 *
	 * Description:
	 *   set the Approach
	 *
	 * Inputs:
	 *   Approach description string
	 *
	 * In/Out:
	 *   none
	 *
	 * Returns:
	 *   none
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setApproach(const string& approach);

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *   getSid()
	 *
	 * Description:
	 *   get the Sids
	 *
	 * Inputs:
	 *   none
	 *
	 * In/Out:
	 *   SID string
	 *
	 * Returns:
	 *   SID string
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const string& getSid() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *  getStar()
	 *
	 * Description:
	 *   get the Star
	 *
	 * Inputs:
	 *   None
	 *
	 * In/Out:
	 *   STAR description
	 *
	 * Returns:
	 *   STAR desciption string
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const string& getStar() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *  getApproach()
	 *
	 * Description:
	 *   get the Approach
	 *
	 * Inputs:
	 *   None
	 *
	 * In/Out:
	 *   Approach description
	 *
	 * Returns:
	 *   Approach desciption string
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const string& getApproach() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *  getPathCost()
	 *
	 * Description:
	 *   get the  path cost
	 *
	 * Inputs:
	 *   None
	 *
	 * In/Out:
	 *   Path cost description
	 *
	 * Returns:
	 *   path cost
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	const double& getPathCost() const;

	/**
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * Function:
	 *  setPathCost(cost)
	 *
	 * Description:
	 *   set the path cost
	 *
	 * Inputs:
	 *   Path cost
	 *
	 * In/Out:
	 *   Set Path Cost
	 *
	 * Returns:
	 *   None
	 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 */
	void setPathCost(const double& path_cost) ;


private:

	/*
	 * sequence of node ids that defines this path
	 */
	deque<int> nodeSequence;

	/*
	 * sequence of cost for node ids that defines this path
	 */
	deque<double> costSequence;

	/*
	 * TODO:PARIKSHIT ADDER FOR TERMINAL PROCEDURES
	 * SID and STARs, Not actually needed, but can help in creating trx files
	 */
	string sid,star,approach;

	/*
	 * TODO:PARIKSHIT ADDER cost
	* SID and STARs, Not actually needed, but can help in creating trx files
	*/
	double path_cost;

};

}

#endif /* SEARCHPATH_H_ */
