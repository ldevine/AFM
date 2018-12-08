/**
 * \class VSpace
 *
 * This class represents a vector space with both real and binary
*  vectors, with methods for computing approx. nearest neighbours.

 */

#ifndef NREP_VSPACE_H
#define NREP_VSPACE_H

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "NearestList.hpp"
#include "MathUtils.hpp"
#include "IOUtils.hpp"
#include "Projection.hpp"
#include "Dictionary.hpp"
#include "BitVector.hpp"

namespace nrep {


	class VSpace;

	using nrep::BitVector;


	struct Task_NN {

		std::shared_ptr<nrep::VSpace> vs;
		nrep::NearestList nl;
		
		int nns; // num nearest neighbours
		int endId;
		int startId;

		Eigen::VectorXf query;

		Task_NN(std::shared_ptr<nrep::VSpace> &vs_, int nns_) : vs(vs_), nns(nns_) {
			nl.resize(nns + 2);
		}
	};


	class VSpace {


	public:

		Dictionary vStore;

		Eigen::MatrixXf mat;
		Eigen::MatrixXi matNN;

		vector<BitVector*> bitVecs;
		
		Eigen::VectorXf query;

		bool bitVectorOwner;


		VSpace() {
			bitVectorOwner = true;
		}

		~VSpace() {
			if (bitVectorOwner) {
				for (auto b : bitVecs) {
					delete b;
				}
			}
		}

		void copy(VSpace & vs) {

			mat = vs.mat;
			vector<string> terms_;
			vs.vStore.getTerms(terms_);

			vStore.removeAll(); // Clear any existing data
			for (auto t : terms_) vStore.addTerm(t);
		}

		int size() {
			return static_cast<int>(vStore.size());
		}

		int realVectorDimension() {			
			return static_cast<int>(mat.rows());
		}

		int bitVectorDimension() {
			if (bitVecs.size() == 0) return 0;
			return static_cast<int>(bitVecs[0]->size());
		}

		void setIsBitVectorOwner(bool isOwner) {
			bitVectorOwner = isOwner;
		}

		void setVectors(Eigen::MatrixXf &m, vector<string> &terms_) {

			vStore.removeAll();
			vector<string> terms;			
			terms = terms_;
			mat = m;

			nrep::normalizeColWiseSafe(mat);

			int count = 0;
			for (string s : terms) {
				if (vStore.contains(s)) {
					cout << endl << s;
				}
				auto v = vStore.addTerm(s);
				v->id = count;
				count++;
			}
		}

		void normalizeVectors() {
			nrep::normalizeColWiseSafe(mat);
		}

		void loadVectors(string &fileName, int maxTerms=-1) {

			vector<string> terms;

			cout << endl << "Loading vectors ... ";

			readVectorsBinary(fileName, mat, terms, maxTerms);

			cout << endl << "Number of terms: " << terms.size() << endl;

			int count = 0;
			for (string s : terms) {
				if (vStore.contains(s)) {
					cout << endl << s;
				}
				auto v = vStore.addTerm(s);
				v->id = count;
				count++;
			}

			query.resize(mat.rows());
		}

		void saveVectors(string fileName) {

			vector<string> terms;
			vStore.getTerms(terms);

			// Need to write vectors
			nrep::writeVectorsBinary(fileName, mat, terms);
		}

		void clearVectors() {

			mat.resize(0, 0);
		}


		void loadBitVectors(string fileName) {

			vector <string> terms_;
			cout << endl << "Reading binary vectors ... " << fileName << endl;
			nrep::readBitVectors(fileName, bitVecs, terms_);
			cout << endl << "Number of bit vectors: " << bitVecs.size() << endl;

			// Need to do more checks - need to reconcile
			if (terms_.size() !=vStore.size()) {
				cout << endl << "Number of real vectors and binary vectors do not match." << endl;
			}
		}
		
		void saveBitVectors(string fileName) {
			vector<string> terms;
			vStore.getTerms(terms);

			// Save vectors
			nrep::writeBitVectors(fileName, bitVecs, terms);
		}
		
		void clearBitVectors() {

			// Need to clear bitvecs
			for (int i = 0; i < bitVecs.size(); i++) {
				delete bitVecs[i];
			}
			bitVecs.clear();
		}


		// Retrieves vectors which are closest to vector with id
		void nearestApproxBits(int id, NearestList &nl, NearestList &nlb) {

			query = mat.col(id);
			nearestBits(bitVecs[id], nlb);
			reRank(query, nl, nlb);
		}

		void nearestBits(BitVector* bVec, NearestList &nl) {

			nl.reset();

			float dim = static_cast<float>(bVec->size());
			float score;

			for (int i = 0; i < bitVecs.size(); i++) {
				auto dVec = bitVecs[i];
				score = (dim - bVec->hammingDistance(*dVec)) / dim;
				nl.pushScore(i, score);
			}
		}

		void reRank(Eigen::VectorXf &rVec, NearestList &nlr, NearestList &nlb) {

			nlr.reset();

			int id;
			float score;
			auto & scores = nlb.getResults();

			for (int i = 0; i < scores.size(); i++) {
				id = scores[i]->_id;
				score = rVec.dot(mat.col(id));
				nlr.pushScore(id, score);
			}
		}

		void randomProject(nrep::Projection &proj, Eigen::MatrixXf &mat1_, Eigen::MatrixXf &mat2_) {

			uint32_t dim = proj.numCols();

			mat2_.resize(dim, mat1_.cols());
			proj.projectMatrix(mat1_, mat2_);
			nrep::normalizeColWiseSafe(mat2_);
		}

		// Standard 1 bit quantization
		void quantizeVectors(Eigen::MatrixXf &m, vector<BitVector*> &bitVecs) {

			int dim = static_cast<int>(m.rows());

			BitVector *v;
			Eigen::VectorXf vec(m.rows());

			// Now iterate over vectors
			for (int i = 0; i < m.cols(); i++) {

				v = new BitVector(dim);
				v->setAllBlocks(0);
				bitVecs.push_back(v);

				for (int j = 0; j < dim; j++) {
					if (m(j, i) >= 0.0f) v->set(j);
				}
			}
		}

		void makeBitVecs(int vecDim) {

			clearBitVectors();

			Eigen::MatrixXf mProj;

			nrep::Projection proj;
			proj.init(static_cast<int>(mat.rows()), vecDim);

			cout << endl << "Preparing vectors ... " << endl;

			mProj.resize(vecDim, mat.cols());
			randomProject(proj, mat, mProj);			
			quantizeVectors(mProj, bitVecs);
		}

	};

}


#endif





