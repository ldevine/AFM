/**
 * \class BitVector
 *
 * High performance bit vector
*
* Based on implementation from LMW-tree
* https://github.com/cmdevries/LMW-tree
*
*/

#ifndef NREP_BIT_VECTOR_HPP
#define NREP_BIT_VECTOR_HPP


#include <cstdint>
#include <string>
#include <iostream>
#include <vector>


#ifdef _MSC_VER
#include <intrin.h>
#endif


// Defines for bit vector
#define W_SIZE 64
#define BITS_WS 6
#define MASK 0x3f


// Typedef for bit vector
typedef uint64_t block_type;


namespace nrep  {


	using std::string;
	using std::vector;
	using std::cout;
	using std::endl;

	class BitVector {
		

	public:

		block_type* _data;

		int _numBlocks;
		int _length;
		string _id;
		int _intId;

	public:

		BitVector() {
			_length = 64;
			_numBlocks = _length >> BITS_WS;
			_data = new block_type[_numBlocks];
		}

		BitVector(int length) {
			_length = length;
			_numBlocks = _length >> BITS_WS;
			_data = new block_type[_numBlocks];
		}

		BitVector(char *bytes, int length) {
			//size_t numBytes = length / 8;
			_length = length;
			_numBlocks = _length >> BITS_WS;
			_data = (block_type*)bytes;
		}

		BitVector(BitVector &vec) {
			_length = vec._length;
			_numBlocks = vec._numBlocks;
			_data = new block_type[_numBlocks];

			// initialise bit vector
			for (int i = 0; i < _numBlocks; i++) {
				setBlock(i, vec._data[i]);
			}
		}

		BitVector(BitVector *vec) {
			_length = vec->_length;
			_numBlocks = vec->_numBlocks;
			_data = new block_type[_numBlocks];

			// initialise bit vector
			for (int i = 0; i < _numBlocks; i++) {
				setBlock(i, vec->_data[i]);
			}
		}

		~BitVector() {
			delete[] _data;
		}


		void resize(int length) {

			delete[] _data;

			_length = length;
			_numBlocks = _length >> BITS_WS;
			_data = new block_type[_numBlocks];
		}


		// Should have small +ve parameter "p"- less than number of blocks.
		// The temp BitVector is used for bookkeeping. Should be the same size as
		// this BitVector.
		void perm(BitVector *temp) {
			temp->copy(this);
			for (int i = 1; i < _numBlocks; i++) {
				setBlock(i-1, temp->_data[i]);
			}
			setBlock(_numBlocks-1, temp->_data[0]);
		}

		void setID(string& id) {
			_id = id;
		}

		const string& getID() {
			return _id;
		}

		size_t size() {
			return _length;
		}

		size_t getNumBlocks() {
			return _numBlocks;
		}

		block_type* getData() {
			return _data;
		}

		void setAllBlocks(block_type v) {
			for (size_t i = 0; i < _numBlocks; i++) {
				_data[i] = v;
			}
		}

		void copy(BitVector *vec) {
			if (this->_length != vec->_length) {
				cout << endl << "Incompatible bit vector lengths.";
				return;
			}
			// Copy data
			for (int i = 0; i < _numBlocks; i++) {
				setBlockEquals(i, vec->_data[i]);
			}
		}

		void setBlocks(int startBlock, BitVector *vec) {
			if (this->getNumBlocks() < vec->getNumBlocks() + startBlock) {
				cout << endl << "Inadequate bit vector length for setting blocks.";
				return;
			}
			// Copy data
			for (int i = 0; i < vec->getNumBlocks(); i++) {
				setBlockEquals(startBlock + i, vec->_data[i]);
			}
		}

		void setBlock(int i, block_type b) {
			_data[i] |= b;
		}

		void setBlockEquals(int i, block_type b) {
			_data[i] = b;
		}

		void set(size_t i) {
			_data[i >> BITS_WS] |= (1LL << (i & MASK));
		}

		void unset(size_t i) {
			_data[i >> BITS_WS] &= ~(1LL << (i & MASK));
		}

		int isSet(size_t i) {
			return ((_data[i >> BITS_WS] & (1LL << (i & MASK))) != 0);
		}

		int operator[](size_t i) {
			return at(i);
		}

		int at(size_t i) {
			return ((_data[i >> BITS_WS] & (1LL << (i & MASK))) != 0);
		}

		int popCount() {

			int count = 0;

			for (int i = 0; i < _numBlocks; i++) count += static_cast<int>(popcnt64(_data[i]));

			return count;
		}

		void exclusiveor(BitVector& v1) {
			int count = 0;
			block_type t;
			for (int i = 0; i < _numBlocks; i++) {
				t = _data[i] ^ v1._data[i];
				_data[i] = t;
			}
		}

		void bw_xor(BitVector& v1, BitVector& v2) {
			int count = 0;
			block_type t;
			for (int i = 0; i < _numBlocks; i++) {
				t = v1._data[i] ^ v2._data[i];
				_data[i] = t;
			}
		}

		void bw_and(BitVector& v1, BitVector& v2) {
			int count = 0;
			block_type t;
			for (int i = 0; i < _numBlocks; i++) {
				t = v1._data[i] & v2._data[i];
				_data[i] = t;
			}
		}

		int hammingDistance(BitVector &other) {
			uint64_t count = 0;
			block_type exclusiveor;
			for (int i = 0; i < _numBlocks; ++i) {
				exclusiveor = _data[i] ^ other._data[i];
				count += popcnt64(exclusiveor);
			}
			return static_cast<int>(count);
		}


		void mean(BitVector *t1, vector<BitVector*> &objs, vector<int> &weights) {

			float total = 0.0f;

			t1->setAllBlocks(0);

			int *bitCountPerDimension = new int[t1->size()];
			memset(bitCountPerDimension, 0, t1->size());

			size_t halfCount = 0;

			if (weights.size() != 0) {
				for (size_t t = 0; t < objs.size(); t++) {
					for (size_t s = 0; s < t1->size(); s++) {
						bitCountPerDimension[s] += (objs[t]->at(s) * weights[t]);
					}
				}
				for (int w : weights) {
					halfCount += w;
				}
				halfCount /= 2;
			}
			else {
				for (size_t t = 0; t < objs.size(); t++) {
					for (size_t s = 0; s < t1->size(); s++) {
						bitCountPerDimension[s] += (objs[t]->at(s));
					}
				}
				halfCount = objs.size() / 2;
			}
			for (size_t s = 0; s < t1->size(); s++) {
				if (bitCountPerDimension[s] > halfCount) t1->set(s);
			}
		}

		void invert() {
			for (int i = 0; i < _numBlocks; i++) _data[i] = ~_data[i];
		}

		void mean(BitVector *t1, vector<BitVector*> &objs) {

			float total = 0.0f;

			t1->setAllBlocks(0);

			int *bitCountPerDimension = new int[t1->size()];
			memset(bitCountPerDimension, 0, t1->size()*4);

			size_t halfCount = 0;
			
			for (size_t t = 0; t < objs.size(); t++) {
				for (size_t s = 0; s < t1->size(); s++) {
					bitCountPerDimension[s] += (objs[t]->at(s));
				}
			}
			halfCount = objs.size() / 2;
			
			for (size_t s = 0; s < t1->size(); s++) {
				if (bitCountPerDimension[s] > halfCount) t1->set(s);
			}

			delete bitCountPerDimension;
		}

		void print() {
			size_t count = 0;

			for (int i = 0; i < _numBlocks; i++) {
				for (int j = W_SIZE - 1; j >= 0; j--) {
					if (_data[i] & (1LL << (j & MASK))) std::cout << '1';
					else std::cout << '0';
					count++;
					if (j % 16 == 0) std::cout << ' ';
				}
			}
		}

		static inline uint64_t popcnt64(block_type b64) {
#ifdef __GNUC__
			// uses POPCNT instruction if available, otherwise lookup table
			return __builtin_popcountll(b64);
#elif defined _MSC_VER

			return __popcnt64(b64);

#else
			block_type x(b64);
			x = x - ((x >> 1) & 0x5555555555555555ULL);
			x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
			x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
			x = (x * 0x0101010101010101ULL) >> 56;
			return int(x);
#endif
		}

		static int hammingDistance(BitVector &v1, BitVector &v2) {

			uint64_t count = 0;
			size_t numBlocks = v1.getNumBlocks();
			block_type *data1, *data2;
			block_type exor;

			data1 = v1.getData();
			data2 = v2.getData();

			for (int i = 0; i < numBlocks; ++i) {
				exor = data1[i] ^ data2[i];
				count += popcnt64(exor);
			}
			return static_cast<int>(count);
		}

	};


}


#endif
