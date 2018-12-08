#ifndef NREP_PROJECTION_HPP
#define NREP_PROJECTION_HPP


#include <string>
#include <random>

#include "Eigen/Dense"

#include "IOUtils.hpp"


using std::string;


namespace nrep {

	using Eigen::MatrixXf;
	using Eigen::VectorXf;


	class Projection {

		int rows;
		int cols;

		MatrixXf mat;


	public:


		Projection() {

		}

		Projection(string fileName) {
			load(fileName);
			this->rows = static_cast<int>(mat.rows());
			this->cols = static_cast<int>(mat.cols());
		}

		Projection(int rows, int cols) {
			this->rows = rows;
			this->cols = cols;
		}

		void init() {
			init(rows, cols);
		}

		void init(int r, int c) {
						
			mat.resize(r, c);

			std::mt19937 gen;
			float mean = 0.0f;
			float std = 1.0f;
			std::normal_distribution<float> normal(mean, std);

			for (int i = 0; i < c; i++) {
				for (int j = 0; j < r; j++) {
					mat(j, i) = normal(gen);
				}
				//cout << endl << mat.col(i);
			}

			mat.colwise().normalize();

		}

		uint32_t numRows() {
			return static_cast<uint32_t>(mat.rows());
		}

		uint32_t numCols() {
			return static_cast<uint32_t>(mat.cols());
		}

		// We assume that matrix and vector have correct size
		// and that the mat columns are normalized.
		void projectMatrixCol(MatrixXf &m, int col, VectorXf &vec) {

			for (int i = 0; i<mat.cols(); i++) {
				vec(i) = mat.col(i).dot( m.col(col) );
			}
		}

		// We assume that matrix and vector have correct size
		// and that the mat columns are normalized.
		void projectMatrix(MatrixXf &mIn, MatrixXf &mOut) {

			//cout << endl << "Projecting matrix ..." << endl;

			for (int i = 0; i < mIn.cols(); i++) {
				for (int j = 0; j < mat.cols(); j++) {
					mOut(j, i) = mat.col(j).dot(mIn.col(i));
				}
			}
		}

		// We assume that the vectors have correct size
		// and that the mat columns are normalized.
		void projectVector(VectorXf &v, VectorXf &vec) {

			for (int i = 0; i<mat.cols(); i++) {
				vec(i) = mat.col(i).dot(v);
			}
		}

		void save(string fileName) {
			cout << endl << "Saving projection matrix ...";
			nrep::writeVectorsSimpleBinary(fileName, mat);
		}

		void load(string fileName) {
			cout << endl << "Loading projection matrix ...";
			nrep::readVectorsSimpleBinary(fileName, mat);
			cout << endl << mat.rows() << " X " << mat.cols();

		}
	};

}


#endif






