/**
 * \class AFrameStore

 * This class stores and indexes frames.
 * There are also methods for performing many operations on frames
 * and frame stores.

 */

#ifndef A_FRAME_STORE_HPP
#define A_FRAME_STORE_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <string> 
#include <fstream> 
#include <unordered_map>
#include <unordered_set>

#include "AFrame.hpp"

#include "Dictionary.hpp"


using std::endl;
using std::cout;
using std::vector;
using std::string;
using std::unordered_map;


namespace nrep {

	using nrep::Dictionary;


	class AFrameStore {

		nrep::Dictionary vStore;


		vector<std::shared_ptr<AFrame>> frames;

		std::unordered_map<int, vector<int>> postings;


	public:

		AFrameStore() {


		}

		~AFrameStore() {
			//cout << endl << frames.size() << endl;
			//cout << endl << "Frame Store destroyed." << endl;

		}

		void reassignIds() {
			int count = 0;
			for (auto f : frames) {
				f->id = count;
				count++;
			}
		}

		void prepareFrames() {
			for (auto f : frames) {
				f->prepare();
			}
		}

		void clearAll() {
			vStore.removeAll();
			frames.clear();
			postings.clear();
		}

		//float averageFrameLengt

		float setIntersection(std::unordered_set<int> &set1, std::unordered_set<int> &set2) {

			int count = 0;
			for (auto i : set1) {
				if (set2.count(i) != 0) count++;
			}
			return ((float)count / set1.size());
		}

		void prune(AFrameStore &fStore, float minThres) {

			//float minThres = 0.9;
			std::unordered_set<int> toDiscard;

			vector<int> ids;
			std::unordered_set<int> set1;
			std::unordered_set<int> set2;

			// Assign ids to frames
			int count = 0;
			for (auto f1 : frames) {
				f1->id = count;
				count++;
			}

			int sz1, sz2;
			for (auto f1 : frames) {

				sz1 = f1->rows * f1->cols;

				set1.clear();
				ids.clear();
				f1->getIds(ids);
				for (int id : ids) set1.insert(id);

				for (auto f2 : frames) {
					if (f1->id >= f2->id) continue;
					sz2 = f2->rows * f2->cols;
					if (sz2 > sz1) continue;

					set2.clear();
					ids.clear();
					f2->getIds(ids);
					for (int id : ids) set2.insert(id);
					
					float overlap = setIntersection(set1, set2);
					//cout << endl << overlap;
					if (overlap > minThres) toDiscard.insert(f2->id);
				}
			}

			for (auto f : frames) {
				if (toDiscard.count(f->id) == 0) {
					fStore.addFrame(f);
				}
			}
			cout << endl << frames.size();
			cout << endl << toDiscard.size();
		}

		void deduplicate(AFrameStore &fStore) {

			std::unordered_set<int> toDiscard;

			for (auto f1 : frames) {
				if (f1->id % 1000 == 0) cout << f1->id;
				for (auto f2 : frames) {
					if (f1->id >= f2->id) continue;
					if (f1->iHash == f2->iHash) {
						if (f1->isDentical(f2)) toDiscard.insert(f2->id);
					}
				}
			}
			for (auto f : frames) {
				if (toDiscard.count(f->id) == 0) {
					fStore.addFrame(f);
				}
			}
		}

		void deduplicateWithIndex(AFrameStore &fStore) {

			std::unordered_set<int> toDiscard;

			for (auto f1 : frames) {
				//if (f1->id % 1000 == 0) cout << f1->id;

				unordered_set<int> others;

				for (int id : f1->vIds) {
					if (postings.count(id) != 0) {
						for (auto fid : postings[id]) {
							others.insert(fid);
						}
					}
				}
								
				for (auto fid : others) {
					if (f1->id >= frames[fid]->id) continue;
					if (f1->iHash == frames[fid]->iHash) {
						if (f1->isDentical(frames[fid])) toDiscard.insert(fid);
					}
				}

			}
			for (auto f : frames) {
				if (toDiscard.count(f->id) == 0) {
					fStore.addFrame(f);
				}
			}
		}

		// We assume we are using 2 x n frames
		void mergeSelfFrames() {

			string fileName = "merges.txt";
			std::ofstream fout(fileName);

			unordered_set<int> toRemove;

			int hits = 0;
			for (int i = 0; i < frames.size(); i++) {
			
				auto f1 = frames[i];

				unordered_set<int> topRow;

				int c = 0;
				int id1, id2;
				bool isHit;

				unordered_set<int> terms1, terms2;
				for (int j = 0; j < f1->ids.cols(); j++) {
					terms1.insert({ f1->ids.at(0, j) });
					terms2.insert({ f1->ids.at(1, j) });
				}

				unordered_set<int> hitFrames;
				for (int j = 0; j < f1->ids.cols(); j++) {
					id1 = f1->ids.at(0, j);
					isHit = false;
					
					vector<int> hitIds;
					for (int k = 0; k < postings[id1].size(); k++) {
						id2 = postings[id1][k];
						if (id2 == i || id2 < i) continue;
						if (frames[id2]->cols > 3) continue;
						int numT1 = 0, numT2 = 0, numT = 0;
						for (int p = 0; p < frames[id2]->cols; p++) {
							auto f = frames[id2];
							if (terms1.count(f->ids.at(0, p)) != 0 && terms2.count(f->ids.at(1, p)) != 0) numT++;
							else if (terms1.count(f->ids.at(1, p)) != 0 && terms2.count(f->ids.at(0, p)) != 0) numT++;
						}

						if (numT >= 3) {
							hitFrames.insert({ id2 });
						}
					}
				}

				if (hitFrames.size() > 0) {

					hits++;
					writeFrame(fout, f1);
					for (auto fid : hitFrames) {
						toRemove.insert(fid);
						writeFrame(fout, frames[fid]);
					}

					//cout << endl << hitFrames.size();
				}



				/*
				for (int id : f1->vIds) {

					if (postings[id].size()>1) hits++;

					/*
					for (auto fid : postings[id]) {
						
						others.insert(fid);
					}

					if (postings.count(id) > 0) {

						hits++;

					}
				}*/

			}

			cout << endl << "To remove: " << toRemove.size();
			cout << endl << "Hits: " << hits;
		}


		void assignFrameStore(AFrameStore &fs) {
			frames.clear();
			frames = fs.frames;
			
			//for (auto f : fs.frames) fs.addFrame(f);
		}

		void mergeFrameStore(AFrameStore &fs) {
			for (auto f : fs.frames) addFrame(f);
		}
		
		void clearFrames() {
			frames.clear();
			postings.clear();
		}

		int size() {
			return static_cast<int>(frames.size());
		}

		void difference(AFrameStore & fStore1, AFrameStore & fStore2) {
			
			// Assumed frames in this framestore are already indexed

			// Need to loop over frames
			for (int i = 0; i < fStore1.frames.size(); i++) {
				auto f1 = fStore1.frames[i];
				unordered_set<int> others;

				bool overlap = true;
				for (int id : f1->vIds) {
					if (postings.count(id) == 0) {
						fStore2.addFrame(f1);
						overlap = false;
						break;
					}
				}

				if (!overlap) continue;

				for (int id : f1->vIds) {
					for (auto fid : postings[id]) {
						others.insert(fid);
					}
				}

				bool isIdentical = false;
				for (auto fid : others) {
					//if (f1->id >= frames[fid]->id) continue;
					if (f1->iHash == frames[fid]->iHash) {
						if (f1->isDentical(frames[fid])) {
							isIdentical = true;
							break;							
						}
					}
				}
				if (!isIdentical) fStore2.addFrame(f1);

			}
		}

		void intersection(AFrameStore & fStore1, AFrameStore & fStore2) {

			// Assumed frames in this framestore are already indexed

			// Need to loop over frames
			for (int i = 0; i < fStore1.frames.size(); i++) {
				auto f1 = fStore1.frames[i];
				unordered_set<int> others;

				bool overlap = true;
				for (int id : f1->vIds) {
					if (postings.count(id) == 0) {
						overlap = false;
						break;
					}
				}

				if (!overlap) continue;

				for (int id : f1->vIds) {
					for (auto fid : postings[id]) {
						others.insert(fid);
					}
				}
				
				for (auto fid : others) {
					//if (f1->id >= frames[fid]->id) continue;
					if (f1->iHash == frames[fid]->iHash) {
						if (f1->isDentical(frames[fid])) fStore2.addFrame(f1);
					}
				}
			}
		}

		void intersectionSets(AFrameStore & fStore1, AFrameStore & fStore2, AFrameStore & fStoreSet1, AFrameStore & fStoreSet2) {

			// Assumed frames in this framestore are already indexed
		
			fStore2.clearFrames();
			fStoreSet1.clearFrames();
			fStoreSet2.clearFrames();

			for (int i = 0; i < fStore1.frames.size(); i++) {
				auto f1 = fStore1.frames[i];

				unordered_set<int> others;

				bool overlap = true;
				for (int id : f1->vIds) {
					if (postings.count(id) == 0) {
						overlap = false;
						fStoreSet1.addFrame(f1);
						break;
					}
				}

				if (!overlap) continue;

				overlap = false;
				for (int id : f1->vIds) {
					for (auto fid : postings[id]) {
						others.insert(fid);
					}
				}

				for (auto fid : others) {
					//if (f1->id >= frames[fid]->id) continue;
					if (f1->iHash == frames[fid]->iHash && f1->isDentical(frames[fid])) {
						fStore2.addFrame(f1);
						overlap = true;
						break;
					} 
				}
				if (!overlap) fStoreSet1.addFrame(f1);
			}


			// Need to index intersection
			fStore2.indexFrames();
			//cout << " * " << fStore2.size() << " * ";

			// Need to loop over frames
			for (int i = 0; i < frames.size(); i++) {
				auto f1 = frames[i];

				unordered_set<int> others;

				bool overlap = true;
				for (int id : f1->vIds) {
					if (fStore2.postings.count(id) == 0) {
						overlap = false;
						fStoreSet2.addFrame(f1);
						break;
					}
				}

				if (!overlap) continue;

				
				overlap = false;
				for (int id : f1->vIds) {
					for (auto fid : fStore2.postings[id]) {
						others.insert(fid);
					}
				}

				for (auto fid : others) {
					
					if (f1->iHash == fStore2.frames[fid]->iHash && f1->isDentical(fStore2.frames[fid])) {
						overlap = true;
						//cout << " " << f1->iHash << " " << fStore2.frames[fid]->iHash << " ";
						break;
					}
				}
				if (!overlap) fStoreSet2.addFrame(f1);
				
			}

		}

		void indexFrames() {

			prepareFrames();

			postings.clear();

			// Need to loop over frames
			for (int i = 0; i < frames.size(); i++) {
				auto f = frames[i];			
				for (auto tid : f->vIds) {
					if (postings.count(tid) == 0) {
						postings.insert( { tid, vector<int>() } );
					}
					postings[tid].push_back(i);					
				}
			}

			//cout << endl << "Number of terms in index: " << postings.size() << endl;
			int pCount = 0;
			for (auto & p : postings) pCount += static_cast<int>(p.second.size());
			//cout << endl << "Number of postings: " << pCount << endl;
		}

		void head(int n) {
			frames.resize(n);
		}

		void addFrame(std::shared_ptr<nrep::AFrame> f) {
			frames.push_back(f);
		}
		
		bool addFrame(vector<string> &strs, int numRows, int numCols) {
			
			bool isComplete = true;

			auto f = std::make_shared<AFrame>(numRows, numCols);
			
			int row = 0;
			int col = 0;

			//cout << endl << strs[0];

			for (int i = 0; i < strs.size(); i++) {
				if (vStore.contains(strs[i])) {
					int id = vStore.getTermIndex(strs[i]);
					f->setId(row, col, id);
				}
				else {
					isComplete = false;
					//cout << endl << "+";
					break;
				}
				col++;
				if (col == numCols) {
					col = 0;
					row++;
				}
			}

			f->id = static_cast<int>(frames.size());
			if (isComplete) frames.push_back(f);

			return isComplete;
		}

		std::shared_ptr<AFrame> getFrameByIndex(int idx) {
			return frames[idx];
		}


		void addToDictionaryFromFile(string fileName) {

			vector<string> toks;
			string line, trimmed;
			std::ifstream fin(fileName);

			while (std::getline(fin, line)) {
				trimmed = nrep::trim(line);
				if (trimmed.size() == 0) continue;
				toks.clear();
				nrep::split(toks, trimmed.c_str());
				for (auto t : toks) {
					vStore.addTerm(t);
				}
			}
		}
		
		void setDictionary(Dictionary &vStore_) {

			vStore.removeAll();
						
			for (int i = 0; i < vStore_.size(); i++) {
				string w = vStore_.getTermByIndex(i)->word;
				vStore.addTerm(w);
			}

			//cout << endl << vStore.size();
		}

		void writeFrame(std::ofstream &fout, std::shared_ptr<AFrame> f) {
			int id;

			for (int i = 0; i < f->rows; i++) {
				for (int j = 0; j < f->cols; j++) {
					id = f->ids.at(i, j);
					fout << vStore.getTermByIndex(id)->word << " ";
				}
				fout << endl;
			}
			fout << endl;
		}

		void writeFrames(string fileName) {

			string line, trimmed;
			vector<string> toks;
			vector<string> toksTemp;
			bool newLine = true;

			int numCols = 0;
			int numRows = 0;

			std::ofstream fout(fileName);

			cout << endl << "Number of frames: " << frames.size() << endl;

			for (auto f : frames) {
				writeFrame(fout, f);
			}

			fout.close();
		}


		void writeFrameTranspose(std::ofstream &fout, std::shared_ptr<AFrame> f) {
			int id;
			for (int i = 0; i < f->cols; i++) {
				for (int j = 0; j < f->rows; j++) {
					id = f->ids.at(j, i);
					fout << vStore.getTermByIndex(id)->word << " ";
				}
				fout << endl;
			}
			fout << endl;
		}

		void writeFramesTranspose(string fileName) {

			string line, trimmed;
			vector<string> toks;
			vector<string> toksTemp;
			bool newLine = true;

			int numCols = 0;
			int numRows = 0;

			std::ofstream fout(fileName);

			for (auto f : frames) {
				writeFrameTranspose(fout, f);
			}
		}

		void readFrames(string fileName) {

			string line, trimmed;
			vector<string> toks;
			vector<string> toksTemp;
			bool newLine = true;
			
			int numCols = 0;
			int numRows = 0;

			std::ifstream fin(fileName);

			while (std::getline(fin, line)) {
				trimmed = nrep::trim(line);
				if (trimmed.size() == 0) {
					newLine = true;
										
					// Need to check if need to add a new frame
					if (toks.size() > 0) {						
						addFrame(toks, numRows, numCols);
						
						toks.clear();
						numRows = 0;
						numCols = 0;
						//if (frames.size() % 1000 == 0) cout << frames.size() << endl;
					}
					continue;
				}
				else {		
					toksTemp.clear();
					nrep::split(toksTemp, trimmed.c_str());
					if (numCols < toksTemp.size()) numCols = static_cast<int>(toksTemp.size());
					numRows++;
					toks.insert(toks.end(), toksTemp.begin(), toksTemp.end());
					//cout << toks.size() << " ";
				}
			}
			// Need to check if need to add a new frame
			if (toks.size() > 0) {
				//cout << "-";
				addFrame(toks, numRows, numCols);
			}
		}

	};

}


#endif

