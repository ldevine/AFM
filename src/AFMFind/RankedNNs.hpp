/**
 * \class RankedNNS
 *
 * This class constructs lists of approximate nearest neighbours using
 * multiple threads.

 */

#ifndef NREP_RANKED_NNs_HPP
#define NREP_RANKED_NNs_HPP


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Eigen/Dense"

#include "Clock.hpp"
#include "IOUtils.hpp"
#include "Dictionary.hpp"

#include "NearestList.hpp"
#include "StringUtils.hpp"

#include "ThreadRing.hpp"

#include "VSpace.hpp"


namespace nrep {


	struct NNTask {

		nrep::VSpace &vs;
		Eigen::MatrixXi &nnMat;
		
		nrep::NearestList nl;
		nrep::NearestList nlCandidates;

		int endId;
		int startId;

		int numNNs;
		int numBitVecList = 500;

		NNTask(nrep::VSpace &_vs, Eigen::MatrixXi &_nns) : vs(_vs), nnMat(_nns) {
			numNNs = static_cast<int>(_nns.rows());
			nlCandidates.resize(numBitVecList);
			nl.resize(numNNs+1);
		}

		void operator()() {

			float score;

			for (int i = startId; i < endId; i++) {

				vs.nearestBits(vs.bitVecs[i], nlCandidates);
				nl.reset();

				// Re-ranking
				for (auto sc : nlCandidates.getResults()) {
					if (sc->_id != i) {
						score = vs.mat.col(i).dot(vs.mat.col(sc->_id));
						nl.pushScore(sc->_id, score);
					}
				}

				int count = 0;
				for (auto sc : nl.getResults()) {
					if (count >= nnMat.rows()) break;
					nnMat(count, i) = sc->_id;
					count++;
				}
			}
		}
	};
	

	void computeNNs(nrep::VSpace &vs, Eigen::MatrixXi &nns) {

		nrep::ThreadRing<NNTask> tr(7);

		nrep::Clock clock;

		cout << endl << "Computing approximate nearest neighbours ... " << endl;

		// Create 10 tasks
		int countIdx = 0;
		for (int i = 0; i < 10; i++) {
			auto w = std::make_shared<NNTask>(vs, nns);
			
			w->startId = 0 + i * 10;
			w->endId = 10 + i * 10;

			tr.submitNewTask(w);
			countIdx += 10;
		}

		// Do work
		int count = 0;
		int batchSize = 10;
		std::shared_ptr<NNTask> w;
		while (tr.size() > 0) {
			w = tr.nextTask();
			// More work ?
			if (countIdx < (nns.cols() - 2*batchSize)) {
				w->startId = countIdx;
				w->endId = w->startId + batchSize;
				countIdx += batchSize;
				//if (countIdx % 1000 == 0) cout << endl << countIdx;
				tr.submitTask(w);
			}
		}

		// Complete remaining work
		if (countIdx < nns.cols()) {
			w->startId = countIdx;
			w->endId = static_cast<int>(nns.cols());
			tr.submitTask(w);
			w = tr.nextTask();
		}		

		clock.report();

	}


}

#endif

