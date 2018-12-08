/**
 * \class NearestList
 *
 * This class implements a sorted list of scored objects

 */

#ifndef NEAREST_LIST_HPP
#define NEAREST_LIST_HPP


#include <cstdint>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>
#include <limits>
#include <queue>


namespace nrep {

	using std::set;
	using std::string;
	using std::vector;
	using std::cout;
	using std::endl;


	struct ScoreResult {

		ScoreResult(int id, float score) : _id(id), _score(score) { }

		int _id;
		float _score;
	};

	typedef std::shared_ptr<ScoreResult> sresult;

	struct CompareByScore {
		bool operator()(sresult &lhs, sresult &rhs) const {
			return lhs->_score > rhs->_score;
		}
	};


	class NearestList {

		uint32_t _maxSize;

		int iter;
		int maxIter;

		vector<sresult> results;

		std::priority_queue<sresult, std::vector<sresult>, CompareByScore > list;


		void pushListToResultsVector() {

			//cout << endl << "Pushing list to results ...";

			if (results.size() < list.size()) {
				//cout << endl << "Results size not equal to list size.";
				resizeResults(static_cast<int>(list.size()));
			}

			int cursor = static_cast<int>(list.size()) - 1;
			while (!list.empty()) {
				const sresult& sr = list.top();
				results[cursor]->_id = sr->_id;
				results[cursor]->_score = sr->_score;
				list.pop();
				cursor--;
			}
		}


		void resizeResults(uint32_t newSize) {

			_maxSize = newSize;

			if (results.size() > _maxSize) {
				results.resize(_maxSize);
			}
			else if (results.size() < _maxSize) {
				while (results.size() < _maxSize) {
					results.push_back(std::make_shared<ScoreResult>(-1, (std::numeric_limits<float>::lowest)()));
				}
			}

			//cout << endl << "Results size now: " << results.size();
		}


	public:


		NearestList() {

		}

		NearestList(uint32_t msz) : _maxSize(msz) {
			resizeResults(msz);
		}

		~NearestList() {
			
		}


		void initIterator() {
			iter = 0;
			maxIter = static_cast<int>(list.size());
			pushListToResultsVector();
		}

		bool hasNext() {
			if (iter<maxIter) return true;
			else return false;
		}

		sresult next() {
			auto res = results[iter];
			iter++;
			return res;
		}
		
		int maxSize() {

			return _maxSize;
		}

		int numResults() {

			return static_cast<int>(list.size());
		}


		void resize(uint32_t msz) {

			resizeResults(msz);

			if (list.size() > _maxSize) {
				int sizeDif = static_cast<int>(list.size()) - _maxSize;
				for (int i = 0; i < sizeDif; i++) list.pop();
			}

			//cout << endl << "Queue size now: " << list.size();
		}

		vector<sresult>& getResults() {

			if (list.size() != 0) {
				pushListToResultsVector();
			}

			return results;
		}

		void pushScore(int id, float score) {
			sresult sr;
			float sc;
			if (list.size() < _maxSize) {
				list.push(std::make_shared<ScoreResult>(id, score));
			}
			else {
				sc = list.top()->_score;
				if (score > sc) {
					list.push(std::make_shared<ScoreResult>(id, score));
					list.pop();
				}
			}
		}

		void printScores() {

			pushListToResultsVector();

			for (const sresult& sr : results) {
				cout << endl << sr->_id << "\t" << sr->_score;
			}
		}

		void makeEmpty() {
			while (!list.empty()) {
				list.pop();
			}
		}

		void zeroReset() {
			makeEmpty();
		}

		void reset() {
			makeEmpty();
		}


		static void test() {
			NearestList l(5);
			
			l.pushScore(1, 0.1f);
			l.pushScore(2, 0.01f);
			l.pushScore(3, 0.1f);
			l.pushScore(4, 0.5f);
			l.pushScore(5, 0.2f);
			l.pushScore(6, 0.4f);
			l.pushScore(7, 0.25f);
			l.printScores();
		}

	};

}



#endif


