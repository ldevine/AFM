/**
 * \class HuffTree
 *
 * This class defines a bottom-up Huffman Tree to be used
 * with DictEntry

 */

#ifndef NREP_HUFF_TREE_H
#define NREP_HUFF_TREE_H

#include "DictEntry.hpp"

#include <set>
#include <vector>
#include <queue>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;
using std::priority_queue;


namespace nrep {
	

	class HuffTree {

	protected:

		struct {
			bool operator()(shared_ptr<DictEntry> lhs, shared_ptr<DictEntry> rhs) const {
				return lhs->cn > rhs->cn;
			}
		} Compare;

	public:

		vector<shared_ptr<DictEntry>> nodes;
		std::priority_queue<shared_ptr<DictEntry>, vector<shared_ptr<DictEntry>>, decltype(Compare)> minHeap;

		HuffTree() {

		}

		~HuffTree() {
			//utils.purge(nodes);
		}

		void addNode(const char *str, int c) {
			auto n = std::make_shared<DictEntry>();
			n->setWord(str);
			n->cn = c;
			minHeap.push(n);
			nodes.push_back(n);
		}

		void addVocabWord(shared_ptr<DictEntry> n) {
			//vocab_word *n = new vocab_word();
			//n->word = str;
			//n->cn = c;
			minHeap.push(n);
			nodes.push_back(n);
		}

		void makeTree() {
			shared_ptr<DictEntry> parent, left, right;

			int nodeCount = static_cast<int>(nodes.size());
			//cout << endl << minHeap.size();

			cout << endl << "Building tree ...";

			while (true) {
				parent = std::make_shared<DictEntry>();

				nodeCount++;
				parent->id = nodeCount;

				left = minHeap.top();
				left->codelen = 1;
				left->set(0);
				minHeap.pop();

				right = minHeap.top();
				right->codelen = 1;
				//right->set(0);
				minHeap.pop();

				left->parent = parent;
				right->parent = parent;
				parent->cn = left->cn + right->cn;

				minHeap.push(parent);
				if (minHeap.size() == 1) {
					//cout << endl << "Finished building tree ...";
					break;
				}
			}
		}

		void assignCodes() {
			cout << endl << "Assigning codes ...";
			shared_ptr<DictEntry> leaf, n, p;
			int codePos;

			for (int i = 0; i < nodes.size(); i++) {
				leaf = nodes[i];
				leaf->codelen = 1;
				n = leaf->parent;
				codePos = 1;
				while ((p = n->parent) != 0) {
					if (n->codeAt(0)) leaf->set(codePos);
					leaf->codelen++;
					n = n->parent;
					codePos++;
				}
			}
		}

		void printCode(char *word) {
			for (auto v : nodes) {
				if (v->word.compare(word) == 0) {
					//cout << endl << "Count: " << v->cn;
					v->printCode();
				}
			}
		}

		void test() {
			DictEntry hn;

			hn.setAllZero();
			hn.set(2);
			hn.set(32);
			hn.set(23);

			hn.printCode();
			cout << endl << "Size of node: " << sizeof(hn);

			addNode("a", 1);
			addNode("b", 2);
			addNode("c", 3);
			addNode("d", 4);
			addNode("e", 5);

			makeTree();
			assignCodes();
						
			for (int i = 0; i < nodes.size(); i++) {
				auto n = nodes[i];
				cout << endl << endl << n->word;
				n->printCode();
				while (n->parent != 0) {
					cout << endl << n->cn;// << " : " << (int)n->cLength;
					n = n->parent;
				}
				cout << endl << n->cn;
			}

		}

	};


} // namespace nrep


#endif






