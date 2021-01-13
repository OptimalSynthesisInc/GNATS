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
 * SearchPath.cpp
 *
 *  Created on: May 8, 2012
 *      Author: jason
 */

#include "SearchPath.h"

#include <deque>

#include <stdio.h>

using namespace std;

namespace osi {

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPath::SearchPath() :
	nodeSequence(deque<int>()),
	costSequence(deque<double>()),
	sid(""),
	star(""),
	approach(""),
	path_cost(0) {
	nodeSequence.clear();
	costSequence.clear();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPath::SearchPath(const SearchPath& that) :
	nodeSequence(that.nodeSequence),
	costSequence(that.costSequence),
	sid(that.sid),
	star(that.star),
	approach(that.approach),
	path_cost(that.path_cost) {
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPath::SearchPath(SearchPathIterator& first, SearchPathIterator& last) :
	nodeSequence(deque<int>()),
	costSequence(deque<double>()),
	sid(""),
	star(""),
	approach(""),
	path_cost(0) {
	nodeSequence.clear();
	nodeSequence.insert(nodeSequence.begin(), first, last);

	costSequence.clear();
	costSequence.insert(costSequence.begin(), first, last);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPath::~SearchPath() {
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchPath::push_front(int node) {
	nodeSequence.push_front(node);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchPath::push_back(int node) {
	nodeSequence.push_back(node);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchPath::pop_front() {
	if(nodeSequence.size() < 1) return -1;
	int node = nodeSequence.front();
	nodeSequence.pop_front();
	return node;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchPath::pop_back() {
	if(nodeSequence.size() < 1) return -1;
	int node = nodeSequence.back();
	nodeSequence.pop_back();
	return node;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int& SearchPath::front() {
	return nodeSequence.front();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int& SearchPath::back() {
	int defaultValue = -1;
	int& tmp_ref = defaultValue;

	if (nodeSequence.size() > 0) {
		return nodeSequence.back();
	} else {
		return tmp_ref;
	}
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPathIterator SearchPath::begin() {
	return nodeSequence.begin();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
SearchPathIterator SearchPath::end() {
	return nodeSequence.end();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchPath::size() const {
	return nodeSequence.size();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchPath::clear() {
	nodeSequence.clear();
	costSequence.clear();
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void SearchPath::erase(SearchPathIterator start, SearchPathIterator end) {
	nodeSequence.erase(start, end);
}

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
void SearchPath::insert(SearchPathIterator pos, SearchPathIterator begin,
    SearchPathIterator end) {
	nodeSequence.insert(pos, begin, end);
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchPath::firstIndexOf(const int node) const {
	deque<int>::const_iterator iter;
	int index = 0;
	for(iter=nodeSequence.begin(); iter!=nodeSequence.end(); ++iter) {
		if(*iter == node) {
			return index;
		}
		index++;
	}
	return -1;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
const deque<int>& SearchPath::getNodes() const {
	return nodeSequence;
}

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
const deque<double>& SearchPath::getCostSequence() const{
	return costSequence;
}

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
void SearchPath::setCostSequence(const deque<double>& costSequence){
	this->costSequence = costSequence;
}

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int SearchPath::operator[](int i) {
	return nodeSequence.at(i);
}

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
bool SearchPath::operator<(const SearchPath& that) const {
	return (this < &that);
}

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
void SearchPath::setSid(const string& sid){
	this->sid = sid;
}

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
void SearchPath::setStar(const string& star){
	this->star = star;
}


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
void SearchPath::setApproach(const string& approach){
	this->approach = approach;
}

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
const string& SearchPath::getSid() const{
	return this->sid;
}

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
const string& SearchPath::getStar() const{
	return this->star;
}

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
const string& SearchPath::getApproach() const{
	return this->approach;
}

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
const double& SearchPath::getPathCost() const{
	return this->path_cost;
}

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
void SearchPath::setPathCost(const double& path_cost) {
	this->path_cost = path_cost;
}

}
