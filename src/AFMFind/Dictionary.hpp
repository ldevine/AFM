/**
 * \class Dictionary
 * 
 * A vocab set with many helpful methods.
 *
 * Also contains a HuffTree for ordering the vocab.
 *
 */

#ifndef NREP_DICTIONARY_H
#define NREP_DICTIONARY_H


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include "DictEntry.hpp"
#include "HuffTree.hpp"


namespace nrep {

	using std::string;
	using std::cout;
	using std::shared_ptr;


	class Dictionary {


	protected:

		HuffTree tree;
		std::vector<shared_ptr<DictEntry>> _vocab;
		std::unordered_map<std::string, shared_ptr<DictEntry>> _vocabMap;

	public:

		Dictionary() {
			_vocabMap.reserve(200000);
		}

		~Dictionary() {
			removeAll();
			//cout << endl << "Dictionary destroyed." << endl;
		}

		void removeAll() {
			_vocab.clear();
			_vocabMap.clear();
		}

		void getTerms(std::vector<std::string> &terms) {
			for (int i = 0; i < _vocab.size(); i++) {
				terms.push_back(_vocab[i]->word);
			}
		}

		bool contains(std::string &str) {
			if (_vocabMap.count(str) == 0) return false;
			else return true;
		}

		bool contains(const string &str) {
			if (_vocabMap.count(str) == 0) return false;
			else return true;
		}

		std::shared_ptr<DictEntry> getTerm(std::string &str) {
			if (_vocabMap.count(str) == 0) return nullptr;
			else return _vocabMap[str];
		}

		shared_ptr<DictEntry> getTerm(const string &str) {
			if (_vocabMap.count(str) == 0) return NULL;
			else return _vocabMap[str];
		}

		shared_ptr<DictEntry> getAddTerm(std::string &str) {
			if (_vocabMap.count(str) == 0) {
				return addTerm(str);
			}
			else return _vocabMap[str];
		}

		shared_ptr<DictEntry> getAddTerm(const string &str) {
			if (_vocabMap.count(str) == 0) {
				return addTerm(str);
			}
			else return _vocabMap[str];
		}

		int getTermIndex(string & str) {
			if (_vocabMap.count(str) == 0) return -1;
			else return _vocabMap[str]->id;
		}

		int getTermIndex(const string &str) {
			if (_vocabMap.count(str) == 0) return -1;
			else return _vocabMap[str]->id;
		}

		shared_ptr<DictEntry> getTermByIndex(int idx) {
			// Need to put checks in here !!!
			return _vocab[idx];
		}

		shared_ptr<DictEntry> addTerm(char *str) {
			shared_ptr<DictEntry> w;
			if (_vocabMap.count(str) == 0) {
				w = std::make_shared<DictEntry>();
				w->word = str;
				w->id = static_cast<int>(_vocab.size());
				_vocab.push_back(w);
				_vocabMap[w->word] = w;
			}
			else {
				w = _vocabMap[str];
			}
			return w;
		}

		shared_ptr<DictEntry> addTerm(string &str) {
			shared_ptr<DictEntry> w;
			if (_vocabMap.count(str) == 0) {
				w = std::make_shared<DictEntry>();
				w->word = str;
				w->id = static_cast<int>(_vocab.size());
				_vocab.push_back(w);
				_vocabMap[w->word] = w;
			}
			else {
				w = _vocabMap[str];
			}
			return w;
		}

		shared_ptr<DictEntry> addTerm(const string &str) {
			shared_ptr<DictEntry> w;
			if (_vocabMap.count(str) == 0) {
				w = std::make_shared<DictEntry>();
				w->word = str;
				w->id = static_cast<int>(_vocab.size());
				_vocab.push_back(w);
				_vocabMap[w->word] = w;
			}
			else {
				w = _vocabMap[str];
			}
			return w;
		}

		void setCountsZero() {
			for (auto v : _vocab) {
				v->cn = 0;
			}
		}

		void sortVocab() {
			auto itBegin = _vocab.begin();
			// Sort
			std::sort(itBegin, _vocab.end(), DictEntry::comp());
		}

		void reidentifyVocab() {
			for (int i = 0; i < _vocab.size(); i++) {
				_vocab[i]->id = i;
			}
		}

		void sortReduce(int minCount) {

			sortVocab();

			// Reduce
			uint32_t i;
			int toRemove = 0;
			std::vector<shared_ptr<DictEntry>> temps;

			for (i = 0; i < _vocab.size(); i++) {
				if (_vocab[i]->cn < minCount) {
					_vocabMap.erase(_vocab[i]->word);
				}
				else temps.push_back(_vocab[i]);
			}
			_vocab.clear();
			for (i = 0; i < temps.size(); i++) _vocab.push_back(temps[i]);

			reidentifyVocab();
		}

		void sortReduce(int minCount, std::unordered_set<string> & wordsToRetain) {

			sortVocab();

			// Reduce
			uint32_t i;
			int toRemove = 0;
			vector<std::shared_ptr<DictEntry> > temps;

			for (i = 0; i < _vocab.size(); i++) {
				if (_vocab[i]->cn < minCount && wordsToRetain.count(_vocab[i]->word) == 0) {
					_vocabMap.erase(_vocab[i]->word);
				}
				else temps.push_back(_vocab[i]);
			}
			_vocab.clear();
			for (i = 0; i < temps.size(); i++) _vocab.push_back(temps[i]);

			reidentifyVocab();
		}
		

		size_t size() {
			return _vocab.size();
		}

		void printStats() {
			cout << endl << "Size of vocab: " << _vocab.size() << endl;
			cout << "First 5 terms ..." << endl;
			for (int i = 0; i < 5; i++) {
				std::cout << _vocab[i]->word << " " << _vocab[i]->cn << std::endl;
			}
		}

		uint64_t totalCounts() {
			uint64_t sum = 0;
			for (auto& v : _vocab) {
				sum += v->cn;
			}
			return sum;
		}

		void getCounts(std::vector<uint64_t>& counts) {
			for (auto& v : _vocab) {
				counts.push_back(v->cn);
			}
		}

		void buildTree() {

			for (int i = 0; i < _vocab.size(); i++) {
				auto v = _vocab[i];
				tree.addVocabWord(v);
			}

			tree.makeTree();
			tree.assignCodes();
		}

		void saveVocab(string & fileName) {

			std::ofstream fout(fileName);

			string str;
			string term;

			for (auto v : _vocab) {
				fout << v->word << " " << v->cn << endl;
			}
		}

	};


}


#endif 
