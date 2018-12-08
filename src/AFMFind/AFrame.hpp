/**
 * \class AFrame

 * This class contains a system of analogies which is generally represented
 * as a matirx of words.

 */



#ifndef A_FRAME_HPP
#define A_FRAME_HPP


#include <iostream>
#include <memory>
#include <vector>
#include <string> 
#include <fstream> 
#include <unordered_set>


#include "StringUtils.hpp"
#include "Matrix.hpp"



using std::endl;
using std::cout;
using std::vector;
using std::string;
using std::unordered_set;
using std::shared_ptr;


namespace nrep {


	struct AFrame {

		int id;
		int rows;
		int cols;

		float sc;

		Matrix<int> ids;

		vector<int> vIds;

		uint64_t iHash;

		bool prepared;

		AFrame(int rows_, int cols_) : rows(rows_), cols(cols_) {

			sc = -1.0f;
			prepared = false;
			ids.resize(rows, cols);
			ids.setAll(-1);			
		}

		AFrame(AFrame &f) {
				
			id = f.id;
			rows = f.rows;
			cols = f.cols;

			ids.assign(f.ids);
		}

		bool isDentical(shared_ptr<AFrame> f) {
			
			if (rows!=f->rows && rows != f->cols) return false;
			if (cols != f->rows && cols != f->cols) return false;

			if (iHash != f->iHash) return false;

			// Check rows and cols
			unordered_set<uint64_t> hset1;
			unordered_set<uint64_t> hset2;

			hashRows(hset1);
			hashCols(hset1);

			f->hashRows(hset2);
			f->hashCols(hset2);

			for (auto h : hset1) {
				if (hset2.count(h) == 0) return false;
			}

			return true;
		}





		void vectorizeIds() {
			vIds.clear();
			getIds(vIds);
		}

		/*
		void makeIdSet() {
			sIds.clear();
			for (int id : vIds) {
				sIds.insert(id);
			}
		}*/

		/*
		bool contains(shared_ptr<AFrame> frame) {

			//cout << endl << cols;

			if (frame->cols > cols) return false;

			vector<int> tIds;
			frame->getIds(tIds);
			for (int id : tIds) {
				if (sIds.count(id) == 0) return false;
			}
			return true;
		}*/

		void setId(int row, int col, int val) {
			ids.set(row, col, val);
			prepared = false;
		}
	
		void clear() {
			ids.setAll(-1);
			prepared = false;
		}

		bool isEmpty() {
			return (ids.at(0, 0) == -1);
		}

		void getIds(vector<int> &ids_) {
			
			for (int i = 0; i < cols; i++) {
				for (int j = 0; j < rows; j++) {
					ids_.push_back(ids.at(j, i));
				}
			}
		}

		void printFrame() {
			for (int i = 0; i < cols; i++) {
				cout << endl;
				for (int j = 0; j < rows; j++) {
					cout << ids.at(j, i) << " ";
				}
			}
			cout << endl;
		}

		void prepare() {
			//if (!prepared) {
				vectorizeIds();
				updateHash();
			//}
			prepared = true;
		}

		void updateHash() {

			uint64_t sum = 0;

			//vectorizeIds();
			//if (!prepared) vectorizeIds();

			for (int i = 0; i < vIds.size(); i++) sum += hash(vIds[i]);
			
			iHash = sum;
		}

		void hashRows(unordered_set<uint64_t> &hset) {

			uint64_t sum = 0;

			//if (!prepared) vectorizeIds();

			for (int i = 0; i < rows; i++) {
				sum = 0;
				for (int j = 0; j < cols; j++) {
					sum += hash(ids.at(i, j));
				}
				hset.insert(sum);
			}
		}

		void hashCols(unordered_set<uint64_t> &hset) {

			uint64_t sum = 0;

			//if (!prepared) vectorizeIds();

			for (int i = 0; i < cols; i++) {
				sum = 0;
				for (int j = 0; j < rows; j++) {
					sum += hash(ids.at(j, i));
				}
				hset.insert(sum);
			}
		}

	private:

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

	};
	

}


#endif



