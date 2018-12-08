/**
 * \class Matrix
 *
 * A simple generic matrix class.

 */

#ifndef NREP_MATRIX_HPP
#define NREP_MATRIX_HPP


#include <iostream>


namespace nrep {


	template <class T>
	class Matrix {

		vector<T> data;

		int nRows;
		int nCols;

		void resize_(int rows_, int cols_) {
			
			nRows = rows_;
			nCols = cols_;

			data.resize(nRows*nCols);
		}
		
	public:

		Matrix() : nRows(0), nCols(0) {

			resize_(nRows, nCols);
		}

		Matrix(int rows_, int cols_) : nRows(rows_), nCols(cols_) {

			resize_(nRows, nCols);
		}

		Matrix(Matrix &m) {
			nRows = m.rows();
			nCols = m.cols();
			data = m.data;
		}

		void assign(Matrix &m) {
			nRows = m.nRows;
			nCols = m.nRows;
			data = m.data;
		}

		int rows() {
			return nRows;
		}

		int cols() {
			return nCols;
		}

		void resize(int rows_, int cols_) {
			resize_(rows_, cols_);
		}

		// No bounds checking performed
		T at(int row, int col) {
			int idx = col * nRows + row;
			return data[idx];
		}

		// No bounds checking performed
		void set(int row, int col, T val) {

			int idx = col * nRows + row;
			data[idx] = val;
		}

		void setAll(T val) {
			data.assign(data.size(), val);
		}

	};

}


#endif


