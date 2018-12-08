/**
 * \class MathUtils
 *
 * Math utilities

 */

#ifndef NREP_MATH_UTILS_HPP
#define NREP_MATH_UTILS_HPP

#include <vector>
#include <iostream>
#include <random>

#include "Eigen/Dense"


namespace nrep {

	void normalizeColWiseSafe(Eigen::MatrixXf &mat) {

		for (int i = 0; i < mat.cols(); i++) {
			if (!mat.col(i).isZero()) {
				mat.col(i).normalize();
			}
		}
	}

	void normalRandomMatrix(Eigen::MatrixXf &mat) {

		std::mt19937 gen;
		float mean = 0.0;
		float std = 1.0;
		std::normal_distribution<float> normal(mean, std);

		for (int i = 0; i < mat.cols(); i++) {
			for (int j = 0; j < mat.rows(); j++) {
				mat(j, i) = normal(gen);
			}
		}
	}


}


#endif