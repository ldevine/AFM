// AFMFind.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>


#include "Model.hpp"
#include "TaskSearchFrames.hpp"


std::unordered_set<uint32_t> quads;


void searchSquaresThreads(std::shared_ptr<nrep::Model> m) {

	nrep::ThreadRing<nrep::TaskSearchFrames> tr(m->options.numThreads);

	nrep::Clock clock;

	cout << endl << "Searching frames ... " << endl;

	// Create tasks
	int countIdx = 0;
	int batchSize = 50;

	for (int i = 0; i < m->options.numThreads+2; i++) {
		auto w = std::make_shared<nrep::TaskSearchFrames>(m, 2000);

		w->toProcess.clear();

		for (int j = 0; j < batchSize; j++) {
			auto n = m->nStore.getNodeByIndex(countIdx);
			if (n != nullptr && n->isInDomain) {
				w->toProcess.push_back(countIdx);
			}
			countIdx++;
		}

		tr.submitNewTask(w);
	}

	// Do work
	int count = 0;
	std::shared_ptr<nrep::TaskSearchFrames> w;

	while (tr.size() > 0) {

		w = tr.nextTask();

		// Need to accumulate quads and frames
		quads.insert(w->foundQuads.begin(), w->foundQuads.end());
		w->quads = quads;
		m->fStore.mergeFrameStore(w->fStore);

		// More work ?
		if (countIdx < (m->options.maxVectors - 2 * batchSize)) {

			w->toProcess.clear();
			for (int j = 0; j < batchSize; j++) {

				auto n = m->nStore.getNodeByIndex(countIdx);
				if (n->isInDomain) {
					w->toProcess.push_back(countIdx);
				}

				countIdx++;
				if (countIdx < 1000 && countIdx % 100 == 0) {
					cout << endl << countIdx;
					m->fStore.writeFrames(m->options.frames);					
				}
				else if (countIdx % 1000 == 0) {
					cout << endl << countIdx;
					m->fStore.writeFrames(m->options.frames);
				}
			}

			tr.submitTask(w);
		}
	}

	// Complete remaining work
	if (countIdx < m->nStore.size()) {

		w->toProcess.clear();
		while (countIdx < m->options.maxVectors) {
			auto n = m->nStore.getNodeByIndex(countIdx);
			if (n->isInDomain) {
				w->toProcess.push_back(countIdx);
			}
			countIdx++;
			//if (countIdx % 1000 == 0) cout << endl << countIdx;
		}

		tr.submitTask(w);
		w = tr.nextTask();

		// Need to accumulate quads and frames
		quads.insert(w->foundQuads.begin(), w->foundQuads.end());
		w->quads = quads;
		m->fStore.mergeFrameStore(w->fStore);
	}

	cout << endl << endl << "Frames: " << m->fStore.size() << endl;

	m->fStore.writeFrames(m->options.frames);

	clock.report();

}



int main(int argc, const char **argv) {

	if (argc == 1) {
		return 0;
	}

	// Parsing options
	cout << endl << "Parsing options ... " << endl;
	nrep::Options options;
	options.parseOptions(argc, argv);

	// Model
	auto model = std::make_shared<nrep::Model>(options);
	model->init();

	searchSquaresThreads(model);
}

