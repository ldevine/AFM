/**
 * \class IOUtils
 *
 * IO utilities

 */

#ifndef NREP_IO_UTILS_H
#define NREP_IO_UTILS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>


#include "misc/tinyformat.h"

#include "Eigen/Dense"

#include "StringUtils.hpp"
#include "BitVector.hpp"



namespace nrep {

	using std::vector;
	using std::string;
	using std::ofstream;
	using std::ifstream;
	using std::cout;
	using std::endl;


	void loadStringLines(string fileName, vector<string> &lines) {

		ifstream fin(fileName);
		string str;
		string term;

		while (std::getline(fin, str)) {
			term = nrep::trim(str);
			if (term.size() > 0) {
				lines.push_back(term);
			}
		}
	}

	void writeStringLines(string fileName, vector<string> &lines) {

		ofstream fout(fileName);
		string str;
		string term;

		for (string &s : lines) {
			fout << s << endl;
		}
	}

	void writeStringSet(string fileName, std::unordered_set<string> &set) {

		ofstream fout(fileName);
		string str;
		string term;

		for (string s : set) {
			fout << s << endl;
		}
	}

	void loadIntIds(string fileName, vector<int> &ids) {

		ifstream fin(fileName);
		string str;
		string line;
		int id;

		while (std::getline(fin, str)) {
			line = nrep::trim(str);
			if (line.size() > 0) {
				id = atoi(line.c_str());
				ids.push_back(id);
			}
		}
	}

	std::pair<int, int> readVectorHeaderParams(string fileName) {

		const int maxChars = 20000;
		// 20000/4 is aaproximately max vector dimension
		char buffer[maxChars];

		int dim, numVecs;
		ifstream fin(fileName, std::ios::in | std::ios::binary);
		string str;
		string tok;

		// Get first line for parameters
		fin.getline(buffer, maxChars);

		vector<string> toks;
		split(toks, buffer, ' ');

		// Get number of vectors
		numVecs = stoi(toks[0]);

		// Get dimension
		dim = stoi(toks[1]);

		return std::make_pair(numVecs, dim);
	}

	void readVectorsBinary(string fileName, Eigen::MatrixXf &mat, vector<string> &terms, int maxTerms = -1) {

		const int maxChars = 50000;
		// 20000/4 is aaproximately max vector dimension
		char buffer[maxChars];

		int dim, numVecs;
		std::ifstream fin(fileName, std::ios::in | std::ios::binary);
		string str;
		string tok;

		// Get first line for parameters
		fin.getline(buffer, maxChars);

		vector<string> toks;
		split(toks, buffer, ' ');

		// Get number of vectors
		numVecs = stoi(toks[0]);

		// Get dimension
		dim = stoi(toks[1]);

		// May need to count number of vecs
		if (numVecs == -1) {
			numVecs = 0;
			while (fin) {
				// Get token
				fin.get(buffer, maxChars, ' ');
				// Get space after token
				fin.read(buffer, 1);
				// Get data for vector
				fin.read(buffer, dim * 4);
				// Get new line
				if (fin) fin.read(buffer, 1);
				numVecs++;
			}
			fin.clear();
			fin.seekg(0, std::ios::beg);
			fin.getline(buffer, maxChars);
		}

		// Allocate Eigen matrix
		if (numVecs > maxTerms && maxTerms!=-1) numVecs = maxTerms;
		mat.resize(dim, numVecs);

		int vecId = 0;

		while (fin && vecId < numVecs) {

			// Get token
			fin.get(buffer, maxChars, ' ');

			terms.push_back(buffer);

			// Get space after token
			fin.read(buffer, 1);

			// Get data for vector
			fin.read(buffer, dim * 4);

			// Copy data into matrix
			memcpy(mat.col(vecId).data(), buffer, dim * 4);

			// Get new line
			if (fin) fin.read(buffer, 1);

			vecId++;
		}
	}
	
	void writeVectorsBinary(string fileName, Eigen::MatrixXf &mat, vector<string> &terms, int maxTerms = -1) {

		int dim, numVecs;
		dim = static_cast<int>(mat.rows());
		numVecs = static_cast<int>(mat.cols());

		if (maxTerms > 0 && maxTerms < numVecs) numVecs = maxTerms;

		ofstream fout(fileName, std::ios::out | std::ios::binary);

		// First write the header
		// Construct header string
		std::string header = tfm::format("%d %d", numVecs, dim);
		fout.write(header.c_str(), header.size());

		// Write end of line
		fout << endl;

		int vecId = 0;

		while (fout && vecId<numVecs) {

			// Write token
			fout.write(terms[vecId].c_str(), terms[vecId].size());

			// Write space
			fout << " ";

			// Write data
			fout.write((char*)(mat.col(vecId).data()), dim * 4);

			// Write end of line
			fout << endl;

			vecId++;

			if (vecId % 10000 == 0) cout << vecId << endl;
		}

		fout.close();
	}

	void readVectorsSimpleBinary(string fileName, Eigen::MatrixXf &mat) {

		const int maxChars = 20000;
		// 20000/4 is aaproximately max vector dimension
		char buffer[maxChars];

		int dim, numVecs;
		std::ifstream fin(fileName, std::ios::in | std::ios::binary);
		string str;
		string tok;

		// Get first line for parameters
		fin.getline(buffer, maxChars);

		vector<string> toks;
		split(toks, buffer, ' ');

		// Get number of vectors
		numVecs = stoi(toks[0]);

		// Get dimension
		dim = stoi(toks[1]);

		//cout << endl << "Vecs: " << numVecs << "  Dim: " << dim << endl;

		// Allocate Eigen matrix
		mat.resize(dim, numVecs);
		mat.setZero();

		int vecId = 0;

		while (fin && vecId < numVecs) {

			// Get data for vector
			fin.read(buffer, dim * 4);

			// Copy data into matrix
			memcpy(mat.col(vecId).data(), buffer, dim * 4);

			// Get end of line
			fin.read(buffer, 1);

			//cout << mat(vecId, 3) << endl;

			vecId++;

			//if (vecId % 1000 == 0) cout << vecId << endl;
		}
	}

	void writeVectorsSimpleBinary(string fileName, Eigen::MatrixXf &mat) {

		int dim = static_cast<int>(mat.rows());
		int numVecs = static_cast<int>(mat.cols());

		std::ofstream fout(fileName, std::ios::out | std::ios::binary);

		// First write the header
		// Construct header string
		std::string header = tfm::format("%d %d", numVecs, dim);
		fout.write(header.c_str(), header.size());

		// Write end of line
		fout << endl;

		int vecId = 0;

		while (fout && vecId < numVecs) {

			// Write data
			fout.write((char*)(mat.col(vecId).data()), dim * 4);

			// Write end of line
			fout << endl;

			vecId++;

			//if (vecId % 1000 == 0) cout << vecId << endl;
		}
	}

	void readBitVectors(string fileName, vector<BitVector*> &vecs, vector<string> &terms) {

		BitVector *vec;
		const int maxChars = 20000;
		// 20000/4 is aaproximately max vector dimension
		char buffer[maxChars];

		int dim, numVecs;
		ifstream fin(fileName, std::ios::in | std::ios::binary);
		string str;
		string tok;
		//float val;

		// Get first line for parameters
		fin.getline(buffer, maxChars);

		vector<string> toks;
		split(toks, buffer, ' ');

		// Get number of vectors
		numVecs = stoi(toks[0]);

		// Get dimension
		dim = stoi(toks[1]);

		cout << endl << "Vecs: " << numVecs << "  Dim: " << dim << endl;

		int vecId = 0;

		while (fin && vecId < numVecs) {

			// Get token
			fin.get(buffer, maxChars, ' ');
			terms.push_back(buffer);

			// Get space after token
			fin.read(buffer, 1);

			// Get data for vector

			fin.read(buffer, dim / 8);
			vec = new BitVector(dim);
			vec->_intId = static_cast<int>(vecs.size());
			vecs.push_back(vec);

			// Copy data into matrix
			memcpy(vec->getData(), buffer, dim / 8);

			// Get new line
			if (fin) fin.read(buffer, 1);

			vecId++;

			//if (vecId % 10000 == 0) cout << vecId << endl;
		}
	}

	void writeBitVectors(string fileName, vector<nrep::BitVector*> &vecs, vector<string> &terms, int maxTerms = -1) {

		if (vecs.size() == 0) return;

		int dim, numVecs;
		dim = static_cast<int>(vecs[0]->size());
		numVecs = static_cast<int>(vecs.size());

		if (maxTerms > 0 && maxTerms < numVecs) numVecs = maxTerms;

		cout << endl << dim << "  " << numVecs;

		cout << endl << "Writing bit vectors ...";

		std::ofstream fout(fileName, std::ios::out | std::ios::binary);

		// First write the header
		// Construct header string
		std::string header = tfm::format("%d %d", numVecs, dim);
		fout.write(header.c_str(), header.size());

		cout << endl << header;

		// Write end of line
		fout << endl;

		int vecId = 0;

		if (!fout.good()) {
			cout << endl << "Problem with file ...";
		}

		cout << endl << vecId << endl;
		cout << numVecs << endl;

		while (fout && (vecId < numVecs)) {

			//cout << vecId << endl;

			// Write token
			fout.write(terms[vecId].c_str(), terms[vecId].size());

			// Write space
			fout << " ";

			// Write data
			fout.write((char*)vecs[vecId]->getData(), dim / 8);

			// Write end of line
			fout << endl;

			vecId++;

			//if (vecId % 10000 == 0) cout << vecId << endl;

		}
	}
}


#endif 
