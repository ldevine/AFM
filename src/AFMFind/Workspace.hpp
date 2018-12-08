/**
 * \class Workspace
 *
 * This class is a "scratchpad" for constructing analogical frames.
 * Methods are provided for testing parallelness and proportionality
 * constraints.
 */

#ifndef NREP_WORKSPACE_HPP
#define NREP_WORKSPACE_HPP


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Eigen/Dense"

#include "StringUtils.hpp"

#include "VSpace.hpp"

#include "Model.hpp"
#include "Matrix.hpp"
#include "Node.hpp"

#include "AFrame.hpp"


namespace nrep {


	class Workspace {

		nrep::Matrix<int> ids;
		nrep::Matrix<std::shared_ptr<Node>> nodes;

		int nRows;
		int nCols;
		int vecDim;

		float paraThres;

		std::shared_ptr<nrep::Model> m;

		Eigen::MatrixXf work;
		Eigen::MatrixXf difVecs;

		BitVector nnMask;

		std::shared_ptr<nrep::VSpace> vs1;
		std::shared_ptr<nrep::VSpace> vs2;

		vector<int> alts;

		BitVector *bv1, *bv2, *bv3, *bv4, *bv5, *bv6;

	public:


		Workspace() {
			paraThres = 0.5f;
		}


		Workspace(std::shared_ptr<nrep::Model> m_, int rows_, int cols_) : m(m_) {

			init(rows_, cols_);
		}

		~Workspace() {
			delete bv1;
			delete bv2;
			delete bv3;
			delete bv4;
			//cout << endl << "Destroying workspace ..." << endl;
		}

		void init(std::shared_ptr<nrep::Model> m_, int rows_, int cols_) {
			m = m_;
			init(rows_, cols_);
		}

		void init(int rows_, int cols_) {

			nRows = rows_;
			nCols = cols_;

			ids.resize(nRows, nCols);
			nodes.resize(nRows, nCols);

			clear();

			vecDim = static_cast<int>(m->vs1->mat.rows());

			// Allocate working matrix
			work.resize(vecDim, 10);

			// Need to allocate matrix for difference vectors
			difVecs.resize(vecDim, nRows*nCols);

			vs1 = m->vs1;
			vs2 = m->vs2;

			nnMask.resize(131072);

			bv1 = new BitVector(vs1->bitVectorDimension());
			bv2 = new BitVector(vs1->bitVectorDimension());
			bv3 = new BitVector(vs1->bitVectorDimension());
			bv4 = new BitVector(vs1->bitVectorDimension());
			bv5 = new BitVector(vs1->bitVectorDimension());
			bv6 = new BitVector(vs1->bitVectorDimension());
		}

		bool setFrame(std::shared_ptr<nrep::AFrame> f) {

			for (int i = 0; i < f->rows; i++) {
				for (int j = 0; j < f->cols; j++) {
					int id = f->ids.at(i, j);
					auto n = m->nStore.getNodeByIndex(id);
					if (!n->isInDomain) {
						return false;
					}
					else {
						setNode(i, j, id);
					}
				}
			}
			return true;
		}

		void getIds(vector<int> &idsVec) {
			for (int i = 0; i < nRows; i++) {
				if (ids.at(i, 0) == -1) break;
				for (int j = 0; j < nCols; j++) {
					if (ids.at(i, j) == -1) break;
					idsVec.push_back(ids.at(i, j));
				}				
			}
		}

		int maxX() {
			int max = 0;
			for (int i = 0; i < nRows; i++) {
				if (ids.at(i, 0) == -1) break;
				for (int j = 0; j < nCols; j++) {
					if (ids.at(i, j) == -1) break;
					if (j >= max) max = j;					
				}
			}
			return max;
		}

		int maxY() {
			int max = 0;
			for (int i = 0; i < nCols; i++) {
				for (int j = 0; j < nRows; j++) {
					if (ids.at(j, i) == -1) break;
					if (j >= max) max = j;
				}
			}
			return max;
		}

		void setNode(int row, int col, int id) {
			ids.set(row, col, id);

			if (id >= 0 && id < m->nStore.size()) {
				auto n = m->nStore.getNodeByIndex(id);
				nodes.set(row, col, n);
			}
		}

		std::shared_ptr<Node> getNode(int row, int col) {
			return nodes.at(row, col);
		}

		void clear() {
			ids.setAll(-1);
		}

		uint32_t hashVal(int row, int col) {
			return nodes.at(row, col)->hashVal;
		}

		std::shared_ptr<nrep::AFrame> getFrame() {

			vector<int> ids_;
			int nrows = 0, ncols = 0;

			for (int i = 0; i < nRows; i++) {
				if (ids.at(i, 0) == -1) break;
				for (int j = 0; j < nCols; j++) {
					if (ids.at(i, j) == -1) break;
					ids_.push_back(nodes.at(i, j)->id);
					if ((j + 1) > ncols) ncols = j + 1;
				}
				nrows++;
			}

			//cout << endl << nrows << " " << ncols;

			auto f = std::make_shared<AFrame>(nrows, ncols);
			int nCount = 0;
			for (int i = 0; i < nrows; i++) {
				for (int j = 0; j < ncols; j++) {
					f->setId(i, j, ids_[nCount]);
					nCount++;
				}
			}

			return f;
		}

		std::pair<int, int> getFrame(vector<string> &terms_) {

			int nrows = 0, ncols = 0;

			for (int i = 0; i < nRows; i++) {
				if (ids.at(i, 0) == -1) break;
				for (int j = 0; j < nCols; j++) {
					if (ids.at(i, j) == -1) break;
					terms_.push_back(nodes.at(i, j)->label);
					if ((j + 1) > ncols) ncols = j + 1;
				}
				nrows++;
			}
			return{ nrows, ncols };
		}

		void writeFrameToFile(std::ofstream &fout) {

			for (int i = 0; i < nRows; i++) {
				if (ids.at(i, 0) == -1) break;
				for (int j = 0; j < nCols; j++) {
					if (ids.at(i, j) == -1) break;
					//fout << m->vs1->vStore.getTermByIndex(nodes.at(i, j)->id)->word << endl;
					fout << nodes.at(i, j)->label << " ";
				}
				fout << endl;
			}
			fout << endl;
		}

		bool isParallel(std::shared_ptr<nrep::VSpace> vs, int id1, int id2, int id3, int id4, float thres) {

			work.col(0) = vs->mat.col(id2) - vs->mat.col(id1);
			work.col(0).normalize();
			work.col(1) = vs->mat.col(id4) - vs->mat.col(id3);
			work.col(1).normalize();

			if (work.col(0).dot(work.col(1)) > thres) return true;
			return false;
		}

		bool isParallelDif(std::shared_ptr<nrep::VSpace> vs1_, std::shared_ptr<nrep::VSpace> vs2_, int id1, int id2, int id3, int id4, float thres) {

			work.col(0) = vs2_->mat.col(id2) - vs1_->mat.col(id1);
			work.col(0).normalize();
			work.col(1) = vs2_->mat.col(id4) - vs1_->mat.col(id3);
			work.col(1).normalize();

			if (work.col(0).dot(work.col(1)) > thres) return true;
			return false;
		}

		bool isParallel_S1(int id1, int id2, int id3, int id4, float thres) {
			return isParallel(vs1, id1, id2, id3, id4, thres);
		}

		bool isParallel_S2(int id1, int id2, int id3, int id4, float thres) {
			return isParallel(vs2, id1, id2, id3, id4, thres);
		}
		
		bool isParallel(int row, int col) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col+1);
			id3 = ids.at(row+1, col);
			id4 = ids.at(row+1, col+1);

			if (!isParallel_S1(id1, id2, id3, id4, m->options.p)) return false;
			if (!isParallel_S2(id1, id3, id2, id4, m->options.p)) return false;

			return true;
		}

		bool isParallelBits(std::shared_ptr<nrep::VSpace> vs, int id1, int id2, int id3, int id4) {

			bv1->bw_xor(*vs->bitVecs[id1], *vs->bitVecs[id2]);
			bv2->bw_xor(*vs->bitVecs[id3], *vs->bitVecs[id4]);
			bv3->bw_xor(*vs->bitVecs[id1], *vs->bitVecs[id3]);

			bv5->bw_and(*bv1, *bv2);

			bv6->copy(bv3);
			bv6->invert();
			bv6->bw_and(*bv6, *bv5);

			float w1 = static_cast<float>(bv5->popCount());
			float w2 = static_cast<float>(bv6->popCount());
			float sc = w2 / w1;

			if (sc < paraThres) return false;
			else return true;
		}

		bool isParallelBits_S1(int id1, int id2, int id3, int id4) {
			return isParallelBits(vs1, id1, id2, id3, id4);
		}

		bool isParallelBits_S2(int id1, int id2, int id3, int id4) {
			return isParallelBits(vs2, id1, id2, id3, id4);
		}

		bool isParallelBits(int row, int col) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + 1);
			id3 = ids.at(row + 1, col);
			id4 = ids.at(row + 1, col + 1);

			if (!isParallelBits_S1(id1, id2, id3, id4)) return false;
			if (!isParallelBits_S2(id1, id3, id2, id4)) return false;

			return true;
		}

		bool isParallelBits(int row, int col, int roff, int coff) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + coff);
			id3 = ids.at(row + roff, col);
			id4 = ids.at(row + roff, col + coff);

			if (!isParallelBits_S1(id1, id2, id3, id4)) return false;
			if (!isParallelBits_S2(id1, id3, id2, id4)) return false;

			return true;
		}

		bool isParallel(int row, int col, int roff, int coff) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + coff);
			id3 = ids.at(row + roff, col);
			id4 = ids.at(row + roff, col + coff);

			if (!isParallel_S1(id1, id2, id3, id4, m->options.p)) return false;
			if (!isParallel_S2(id1, id3, id2, id4, m->options.p)) return false;

			return true;
		}

		bool isParallelDual(int row, int col) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + 1);
			id3 = ids.at(row + 1, col);
			id4 = ids.at(row + 1, col + 1);

			if (!isParallel(vs1, id1, id2, id3, id4, m->options.p)) return false;						
			if (!isParallelDif(vs1, vs2, id1, id3, id2, id4, m->options.p)) return false;

			return true;
		}

		bool isParallelDual(int row, int col, int roff, int coff) {

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + coff);
			id3 = ids.at(row + roff, col);
			id4 = ids.at(row + roff, col + coff);

			if (!isParallel(vs1, id1, id2, id3, id4, m->options.p)) return false;
			if (!isParallelDif(vs1, vs2, id1, id3, id2, id4, m->options.p)) return false;

			return true;
		}

		// Need to
		// Calculate score


		float sim(Eigen::VectorXf &a, Eigen::VectorXf &b) {
			return (a.dot(b) + 1.0f) / 2;
		}

		float sim(Eigen::VectorXf &a, Eigen::MatrixXf &mat, int id) {
			return (mat.col(id).dot(a) + 1.0f) / 2;
		}

		float calScore(std::shared_ptr<nrep::VSpace> vs1_, std::shared_ptr<nrep::VSpace> vs2_, int id1, int id2, int id3, int id4) {

			float sc;
			float s1, s2, s3, s4;

			work.col(1) = vs1_->mat.col(id4) - vs1_->mat.col(id3);
			work.col(1).normalize();

			work.col(4) = work.col(0) + work.col(1);
			work.col(4).normalize();

			work.col(3) = vs2_->mat.col(id4) - vs2_->mat.col(id2);
			work.col(3).normalize();

			work.col(5) = work.col(2) + work.col(3) + work.col(9);
			work.col(5).normalize();

			s1 = (vs1_->mat.col(id4).dot(vs1_->mat.col(id3)) + 1.0f) / 2.0f;
			s2 = (vs1_->mat.col(id4).dot(work.col(4)) + 1.0f) / 2.0f;
			s3 = (vs2_->mat.col(id4).dot(vs2_->mat.col(id2)) + 1.0f) / 2.0f;
			s4 = (vs2_->mat.col(id4).dot(work.col(5)) + 1.0f) / 2.0f;

			sc = s1 * s2 + s3 * s4;

			return sc;
		}

		float calScoreDual(std::shared_ptr<nrep::VSpace> vs1_, std::shared_ptr<nrep::VSpace> vs2_, int id1, int id2,
			int id3, int id4) {

			float sc;
			float s1, s2, s3, s4;

			//auto v = vs1_->mat.col(id4) - vs2_->mat.col(id4);
			//cout << endl << v.norm();

			//work.col(1) = vs1_->mat.col(id4) - vs1_->mat.col(id3);
			work.col(1) = vs2_->mat.col(id4) - vs2_->mat.col(id3);
			work.col(1).normalize();

			work.col(4) = work.col(0) + work.col(1);
			work.col(4).normalize();

			work.col(3) = vs2_->mat.col(id4) - vs1_->mat.col(id2);
			work.col(3).normalize();

			work.col(5) = work.col(2) + work.col(3);
			work.col(5).normalize();

			s1 = (vs1_->mat.col(id4).dot(vs1_->mat.col(id3)) + 1.0f) / 2.0f;
			s2 = (vs1_->mat.col(id4).dot(work.col(4)) + 1.0f) / 2.0f;
			s3 = (vs2_->mat.col(id4).dot(vs1_->mat.col(id2)) + 1.0f) / 2.0f;
			s4 = (vs2_->mat.col(id4).dot(work.col(5)) + 1.0f) / 2.0f;

			sc = s1 * s2 + s3 * s4;
			//sc = s3 * s4;

			//cout << endl << "" << (s1*s2) << "  " << (s3*s4) << endl;

			return sc;
		}

		void prepareAlts(vector<int> &alts, int id, int num) {

			int aid;
			for (int i = 0; i < num; i++) {
				aid = m->nnMat(i, id);
				
				if (!nnMask.isSet(aid)) {
					alts.push_back(aid);
					nnMask.set(aid);
				}
			}
		}

		bool pointCheck(int row, int col, int rOff, int cOff, int orient, bool dual) {

			int numAlts = 20;

			int id1, id2, id3, id4;

			id1 = ids.at(row, col);
			id2 = ids.at(row, col + cOff);
			id3 = ids.at(row + rOff, col);
			id4 = ids.at(row + rOff, col + cOff);

			nnMask.setAllBlocks(0);
			alts.clear();

			prepareAlts(alts, id2, numAlts);
			prepareAlts(alts, id3, numAlts);
			prepareAlts(alts, id4, numAlts);
			alts.push_back(-1);

			if (orient == 0) work.col(9).setZero();
			else if (orient == 1 || orient == 2) work.col(9) = work.col(7);
			else if (orient == 3 || orient == 4) work.col(9) = work.col(7) * -1;

			//cout << endl << alts[3];

			if (!dual) return pointCheckAlts(id1, id2, id3, id4, alts, orient);
			else return pointCheckAltsDual(id1, id2, id3, id4, alts, orient);
		}
		
		// Check the square with upper left corner given by row and col
		bool pointCheckAlts(int id1, int id2, int id3, int id4, vector<int> &alts, int orient) {

			int id5;
			bool result = true;
			float sc1, sc2;

			work.col(0) = vs1->mat.col(id2) - vs1->mat.col(id1);
			work.col(0).normalize();

			work.col(2) = vs2->mat.col(id3) - vs2->mat.col(id1);
			work.col(2).normalize();

			sc1 = calScore(vs1, vs2, id1, id2, id3, id4);

			for (int i = 0; i < alts.size(); i++) {
				id5 = alts[i];
				if (id5 == -1) break;
				if (id5 == id4) continue;

				sc2 = calScore(vs1, vs2, id1, id2, id3, id5);

				if (sc1 <= sc2) {
					result = false;
					break;
				}
			}

			return result;
		}

		// Check the square with upper left corner given by row and col
		bool pointCheckAltsDual(int id1, int id2, int id3, int id4, vector<int> &alts, int orient) {

			int id5;
			bool result = true;
			float sc1, sc2;

			//auto v = vs1->mat.col(id4) - vs2->mat.col(id4);
			//cout << endl << v.norm();

			work.col(0) = vs1->mat.col(id2) - vs1->mat.col(id1);
			//cout << endl << "--  " << work.col(0).norm();
			work.col(0).normalize();

			work.col(2) = vs2->mat.col(id3) - vs1->mat.col(id1);
			//cout << endl << work.col(2).norm();
			work.col(2).normalize();

			sc1 = calScoreDual(vs1, vs2, id1, id2, id3, id4);

			for (int i = 0; i < alts.size(); i++) {
				id5 = alts[i];
				if (id5 == -1) break;
				if (id5 == id4) continue;

				sc2 = calScoreDual(vs1, vs2, id1, id2, id3, id5);

				if (sc1 <= sc2) {
					result = false;
					break;
				}
			}

			return result;
		}

		bool squareCheck(int row, int col) {

			if (!pointCheck(row, col, 1, 1, 0, false)) return false;
			if (!pointCheck(row, col + 1, 1, -1, 0, false)) return false;
			if (!pointCheck(row + 1, col, -1, 1, 0, false)) return false;
			if (!pointCheck(row + 1, col + 1, -1, -1, 0, false)) return false;

			return true;
		}

		bool squareCheckDual(int row, int col) {

			if (!pointCheck(row, col, 1, 1, 0, true)) return false;
			if (!pointCheck(row, col + 1, 1, -1, 0, true)) return false;
			if (!pointCheck(row + 1, col, -1, 1, 0, true)) return false;
			if (!pointCheck(row + 1, col + 1, -1, -1, 0, true)) return false;

			return true;
		}

		bool squareCheck(int row, int col, int roff, int coff) {

			if (!pointCheck(row, col, roff, coff, 0, false)) return false;
			if (!pointCheck(row, col + coff, roff, -1 * coff, 0, false)) return false;
			if (!pointCheck(row + roff, col, -1 * roff, coff, 0, false)) return false;
			if (!pointCheck(row + roff, col + coff, -1 * roff, -1 * coff, 0, false)) return false;

			return true;
		}

		bool squareCheckDual(int row, int col, int roff, int coff) {

			if (!pointCheck(row, col, roff, coff, 0, true)) return false;
			if (!pointCheck(row, col + coff, roff, -1 * coff, 0, true)) return false;
			if (!pointCheck(row + roff, col, -1 * roff, coff, 0, true)) return false;
			if (!pointCheck(row + roff, col + coff, -1 * roff, -1 * coff, 0, true)) return false;

			return true;
		}

		bool squareCheckAv(int row, int col, int roff, int coff) {

			if (!pointCheck(row, col, roff, coff, 1, false)) return false;
			if (!pointCheck(row, col + coff, roff, -1 * coff, 2, false)) return false;
			if (!pointCheck(row + roff, col, -1 * roff, coff, 3, false)) return false;
			if (!pointCheck(row + roff, col + coff, -1 * roff, -1 * coff, 4, false)) return false;

			return true;
		}

		/*
		bool frameCheck() {

		}
		*/


		bool rightPairCheck(int row) {

			Eigen::VectorXf v;

			work.col(6).setZero();
			work.col(7).setZero();

			int mxX = maxX();

			std::pair<int, int> p;
						
			for (int i = 0; i < (mxX - 1); i++) {
				p.first = ids.at(row, i);
				p.second = ids.at(row+1, i);

				v = vs2->mat.col(p.second) - vs2->mat.col(p.first);
				v.normalize();

				work.col(7) += v;
			}

			return squareCheckAv(row, mxX-1, 1, 1);
		}

		bool leftRightCheck(int row) {

			Eigen::VectorXf v;

			work.col(6).setZero();
			work.col(7).setZero();

			int mxX = maxX();

			std::pair<int, int> p;

			for (int i = 0; i < (mxX - 1); i++) {
				p.first = ids.at(row, i);
				p.second = ids.at(row + 1, i);

				v = vs2->mat.col(p.second) - vs2->mat.col(p.first);
				v.normalize();

				work.col(7) += v;
			}

			if (!squareCheckAv(row, mxX - 1, 1, 1)) return false;
			if (!squareCheckAv(row, 0, 1, mxX)) return false;
			return true;
		}

		/*
		bool bottomPairCheck() {

		}

		bool frameAllCheck() {

			int mxX = maxX();
			int mxY = maxY();
			
			Eigen::VectorXf v;

			//cout << endl << mxX;

			if (mxX < 2) return false;
						
			int p1, p2;
			//int maxPair = pairs.size() - 1;
			//int idTop = pairs[maxPair].first;
			//int idBot = pairs[maxPair].second;

			std::pair<int, int> left;
			std::pair<int, int> right;

			work.col(6).setZero();

			int count = 0;

			for (int k = mxX; k > 0; k--) {
				
				right.first = ids.at(0, k);
				right.second = ids.at(1, k);

				//for (int i = k - 1; i >= 0; i--) {
				for (int i = 0; i <= (k-1); i++) {

					left.first = ids.at(0, i);
					left.second = ids.at(1, i);

					work.col(7).setZero();

					for (int j = 0; j <= mxX; j++) {
						if (j == mxX || j == i) continue;
												
						p1 = ids.at(0, j);
						p2 = ids.at(1, j);

						v = vsVert->mat.col(p2) - vsVert->mat.col(p1);
						v.normalize();

						work.col(7) += v;
					}

					if (!squareCheckAv(0, i, 1, k - i)) return false;
					//	count++;
					//	if (count > return false;
					//}


					//if (!squareCheck(0, i, 1, k - i)) return false;
					
					count++;
					
					if (count == 3) break;
					//if (count>2) cout << "_" << endl;
					//cout << i << " " << k - i << endl;

					//if (!frameCheck(vs, left.first, right.first, left.second, right.second)) return false;

				}
				break;
			}

			return true;
		}*/
		
	};


}


#endif


