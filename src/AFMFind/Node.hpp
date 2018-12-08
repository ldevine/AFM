#ifndef NREP_NODE_H
#define NREP_NODE_H

#include <unordered_set>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


#include "BitVector.hpp"


using nrep::BitVector;


struct Node;


struct NodeFilter {

	int sz;
	BitVector bv;

	NodeFilter();

	NodeFilter(int sz_);

	~NodeFilter();
	
	void resize(int sz_);
	void clear();
	int size();

	void addNode(std::shared_ptr<Node> n);
	bool hasNode(std::shared_ptr<Node> n);

	uint32_t nextPowerOfTwo(uint32_t n);
};


struct Node {

	bool isInDomain;

	bool modified;

	std::string label;

	int id; // Domain id
	//int gid; // Global id

	int domainSize;

	std::vector<int> nns;

	NodeFilter nf;

	uint32_t hashVal;

	uint32_t h1;
	uint32_t h2;
	uint32_t h3;

	Node(int _id, std::string _label, int filterSize) : id(_id), label(_label) {

		nf.resize(filterSize);

		isInDomain = false;

		hashVal = hash(id);

		h1 = hash(id) % nf.size();
		h2 = hash(id + 13133171) % nf.size();
		h3 = hash(id + 17323353) % nf.size();
	}

	~Node() {
		//cout << endl << "Destroying node ..." << endl;
		//delete nnb;
		//delete dnb;
	}

	void initialize(int _domainSize) {
		domainSize = _domainSize;
	}

	// Robert Jenkin's 32 bit hash
	uint32_t hash(uint32_t a)
	{
		a = (a + 0x7ed55d16) + (a << 12);
		a = (a ^ 0xc761c23c) ^ (a >> 19);
		a = (a + 0x165667b1) + (a << 5);
		a = (a + 0xd3a2646c) ^ (a << 9);
		a = (a + 0xfd7046c5) + (a << 3);
		a = (a ^ 0xb55a4f09) ^ (a >> 16);
		return a;
	}


	bool isNN(std::shared_ptr<Node> n) {

		return nf.hasNode(n);
	}

	void setAsNN(std::shared_ptr<Node> n) {

		return nf.addNode(n);
	}


private:

	uint32_t nextPowerOfTwo(uint32_t n) {
		n--;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;

		return n + 1;
	}
};


NodeFilter::NodeFilter() {
		sz = 0;
}

NodeFilter::NodeFilter(int sz_) {
		resize(sz_);
}

NodeFilter::~NodeFilter() {
		
}


void NodeFilter::resize(int sz_) {
	sz = nextPowerOfTwo(sz_ - 1);
	bv.resize(sz);
}

void NodeFilter::clear() {
	bv.setAllBlocks(0);
}

int NodeFilter::size() {
	return sz;
}

void NodeFilter::addNode(std::shared_ptr<Node> n) {

		bv.set(n->h1);
		bv.set(n->h2);
		bv.set(n->h3);
}

bool NodeFilter::hasNode(std::shared_ptr<Node> n) {

	if (!bv.isSet(n->h1)) return false;
	if (!bv.isSet(n->h2)) return false;
	if (!bv.isSet(n->h3)) return false;

	return true;
}



uint32_t NodeFilter::nextPowerOfTwo(uint32_t n) {

	n--;

	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;

	return n + 1;
}



struct NodeStore {

	std::vector<std::shared_ptr<Node> > nodeList;
	std::unordered_map<std::string, std::shared_ptr<Node> > nodeMap;
	
	~NodeStore() {
		//cout << endl << "NodeStore destroyed." << endl;
	}

	void addNode(std::string _str) {
		auto n = std::make_shared<Node>( size(), _str, 4096);		
		n->id = size();
		nodeList.push_back(n);
		nodeMap.insert({ _str, n });
	}

	void initialize() {
		for (auto n : nodeList) n->initialize(size());
	}

	// Need to use some of the functions from the dictionary class
	std::shared_ptr<Node> getNodeByIndex(int idx) {
		// Need to put checks in here !!!
		return nodeList[idx];
	}

	bool containsNode(std::string &s) {
		if (nodeMap.count(s) != 0) return true;
		else return false;
	}

	std::shared_ptr<Node> getNode(std::string &s) {

		if (nodeMap.count(s) != 0) return nodeMap[s];
		else {
			std::shared_ptr<Node> ptr; // return nulptr
			return ptr;
		}
	}

	int size() {
		return static_cast<int>(nodeList.size());
	}

};





#endif


