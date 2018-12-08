/**
 * \class TaskSearchFrames
 *
 * This class searches for frames. Instances of this class are intended
 * to be executed in parallel.
 */

#ifndef NREP_TASK_SEARCH_FRAMES_HPP
#define NREP_TASK_SEARCH_FRAMES_HPP


#include <iostream>

#include "Model.hpp"
#include "Workspace.hpp"



namespace nrep {


	class TaskSearchFrames {

		
		std::shared_ptr<nrep::Model> m;
		
		Workspace ws;
		Workspace ws2;
		unordered_set<int> members;
				
		int maxCols = 10;

	public:

		nrep::AFrameStore fStore;

		vector<int> toProcess;
		
		unordered_set<uint32_t> quads;
		unordered_set<uint32_t> foundQuads;

		TaskSearchFrames(std::shared_ptr<nrep::Model> _m, int maxCols_) : m(_m) {
			maxCols = maxCols_;
			ws.init(m, 3, maxCols+10);
			ws2.init(m, 3, maxCols + 10);
			fStore.setDictionary(m->vs1->vStore);
		}

		~TaskSearchFrames() {
			//cout << endl << "Destroying task ..... ....  .....  ....." << endl;
		}

		void addframe(Workspace &wsf) {

			auto f = wsf.getFrame();
			fStore.addFrame(f);

			// Make quad hashes
			for (int i = 0; i < (f->cols - 1); i++) {
				uint32_t hashVal = wsf.hashVal(0, i) + wsf.hashVal(0, i + 1) + wsf.hashVal(1, i) + wsf.hashVal(1, i + 1);
				foundQuads.insert(hashVal);
			}
			for (int i = 0; i < (f->cols - 2); i++) {
				uint32_t hashVal = wsf.hashVal(0, i) + wsf.hashVal(0, i + 2) + wsf.hashVal(1, i) + wsf.hashVal(1, i + 2);
				foundQuads.insert(hashVal);
			}
			if (wsf.maxY() > 1) {
				for (int i = 0; i < (f->cols - 1); i++) {
					uint32_t hashVal = wsf.hashVal(1, i) + wsf.hashVal(1, i + 1) + wsf.hashVal(2, i) + wsf.hashVal(2, i + 1);
					foundQuads.insert(hashVal);
				}

			}
		}

		bool extend() {

			unordered_set<int> members;
			
			vector<int> ids;
			ws.getIds(ids);
			for (auto i : ids) members.insert(i);

			int cols = ws.maxX();
			int rows = ws.maxY();
			int newCols;

			int sCol = 0;
			for (int extendCols = 0; extendCols < maxCols; extendCols++) {

				newCols = extendImp(cols, rows, members, sCol);
				if (newCols == cols) {
					sCol++;
					if (sCol>=(ws.maxX()-1) || sCol > 50 ) break;
				}
				cols = newCols;
			}

			return (cols != ws.maxX());
		}


		int extendImp(int cols, int rows, unordered_set<int> &members, int sCol) {

			int agreesThres = 2;

			cols++;

			// This is the equation which governs how the extension is made
			if (cols > 6) agreesThres = 3 + cols / 5;

			bool extended = false;

			int agrees = 0;
			auto n = ws.getNode(0, sCol);

			for (int i1 : n->nns) {

				if (members.count(i1) != 0) continue;
				auto n1 = m->nStore.getNodeByIndex(i1);
				if (!n1->isInDomain) continue;

				ws.setNode(0, cols, n1->id);

				for (int i2 : n1->nns) {
					if (members.count(i2) != 0) continue;
					ws.setNode(1, cols, i2);
					if (!ws.getNode(1, sCol)->isNN(ws.getNode(1, cols))) continue;

					auto hashVal = ws.hashVal(0, cols - 1) + ws.hashVal(0, cols) + ws.hashVal(1, cols - 1) + ws.hashVal(1, cols);
					if (quads.count(hashVal) != 0) continue;
					if (foundQuads.count(hashVal) != 0) continue;

					hashVal = ws.hashVal(0, cols - 2) + ws.hashVal(0, cols) + ws.hashVal(1, cols - 2) + ws.hashVal(1, cols);
					if (quads.count(hashVal) != 0) continue;
					if (foundQuads.count(hashVal) != 0) continue;

					hashVal = ws.hashVal(0, cols - 3) + ws.hashVal(0, cols) + ws.hashVal(1, cols - 3) + ws.hashVal(1, cols);
					if (quads.count(hashVal) != 0) continue;
					if (foundQuads.count(hashVal) != 0) continue;

					agrees = 0;

					if (!ws.isParallelBits(0, 0, 1, cols)) continue;
					if (!ws.isParallel(0, 0, 1, cols)) continue;

					for (int k = 0; k < cols; k++) {

						if (!ws.isParallelBits(0, k, 1, cols - k)) continue;
						if (!ws.isParallel(0, k, 1, cols - k)) continue;

						if (!ws.squareCheckAv(0, k, 1, cols - k)) continue;

						agrees++;
						if (agrees >= agreesThres) break;
					}

					if (agrees >= agreesThres) {
						//if (!ws.rightPairCheck(0)) continue;	
						if (!ws.leftRightCheck(0)) continue;

						// Successfully extended
						extended = true;
						members.insert(i1);
						members.insert(i2);
						break;
					}
				}
				if (extended) break;
			}
			if (!extended) {
				ws.setNode(0, cols, -1);
				ws.setNode(1, cols, -1);
				return cols - 1;
			}
			else return cols;
		}

		bool extendDown() {

			ws2.clear();

			// Need to switch ids to new workspace
			ws2.setNode(0, 0, ws.getNode(0, 0)->id);
			ws2.setNode(1, 0, ws.getNode(0, 1)->id);
			ws2.setNode(2, 0, ws.getNode(0, 2)->id);

			ws2.setNode(0, 1, ws.getNode(1, 0)->id);
			ws2.setNode(1, 1, ws.getNode(1, 1)->id);
			ws2.setNode(2, 1, ws.getNode(1, 2)->id);

			unordered_set<int> members;

			vector<int> ids;
			ws2.getIds(ids);
			for (auto i : ids) members.insert(i);

			int cols = ws2.maxX();
			int rows = ws2.maxY();
			int agreesThres = 2;
			bool finished = false;
			bool extended = false;
			
			for (int extendCols = 0; extendCols < maxCols && !finished; extendCols++) {

				cols++;

				// This is the equation which governs how the extension is made
				if (cols > 6) agreesThres = 3 + cols / 5;

				extended = false;

				int agrees = 0;
				auto n = ws2.getNode(0, cols - 1);

				for (int i1 : n->nns) {

					if (members.count(i1) != 0) continue;
					auto n1 = m->nStore.getNodeByIndex(i1);
					if (!n1->isInDomain) continue;

					ws2.setNode(0, cols, n1->id);

					for (int i2 : n1->nns) {
						if (i1 == i2) continue;
						if (members.count(i2) != 0) continue;

						ws2.setNode(1, cols, i2);
						if (!ws2.getNode(1, cols - 1)->isNN(ws2.getNode(1, cols))) continue;

						auto n2 = m->nStore.getNodeByIndex(i2);
						if (!n2->isInDomain) continue;

						auto hashVal = ws2.hashVal(0, cols - 1) + ws2.hashVal(0, cols) + ws2.hashVal(1, cols - 1) + ws2.hashVal(1, cols);
						if (quads.count(hashVal) != 0) continue;
						if (foundQuads.count(hashVal) != 0) continue;

						hashVal = ws2.hashVal(0, cols - 2) + ws2.hashVal(0, cols) + ws2.hashVal(1, cols - 2) + ws2.hashVal(1, cols);
						if (quads.count(hashVal) != 0) continue;
						if (foundQuads.count(hashVal) != 0) continue;

						if (!ws2.isParallelBits(0, 0, 1, cols)) continue;
						if (!ws2.isParallelBits(0, 1, 1, cols - 1)) continue;

						if (!ws2.isParallel(0, 0, 1, cols)) continue;
						if (!ws2.isParallel(0, 1, 1, cols - 1)) continue;

						//if (!ws2.rightPairCheck(0)) continue;
						if (!ws2.leftRightCheck(0)) continue;

						for (int i3 : n2->nns) {
							if (i3 == i2 || i3 == i1) continue;
							if (members.count(i3) != 0) continue;

							ws2.setNode(2, cols, i3);

							if (!ws2.getNode(2, cols - 1)->isNN(ws2.getNode(2, cols))) continue;

							if (!ws2.isParallelBits(1, 0, 1, cols)) continue;
							if (!ws2.isParallelBits(1, 1, 1, cols - 1)) continue;

							if (!ws2.isParallel(1, 0, 1, cols)) continue;
							if (!ws2.isParallel(1, 1, 1, cols - 1)) continue;

							if (!ws2.leftRightCheck(1)) continue;

							// Successfully extended
							extended = true;
							members.insert(i1);
							members.insert(i2);
							members.insert(i3);

							break;
						}
						if (extended) break;
					}
					if (extended) break;
				}
				if (!extended) {
					ws2.setNode(0, cols, -1);
					ws2.setNode(1, cols, -1);
					ws2.setNode(2, cols, -1);
					finished = true;
				}
			}

			return (cols>2);
		}

		
		void run6(int i1) {

			int nnThres = m->options.nns;

			uint32_t hashVal;
			int count = 0;

			bool foundFrame = false;
						
			auto n1 = m->nStore.getNodeByIndex(i1);
			if (!n1->isInDomain) return;

			ws.clear();
			ws.setNode(0, 0, n1->id);

			int cnt2 = 0;
			for (int i2 : n1->nns) {
				if (i2 <= i1) continue;

				cnt2++;
				if (cnt2 > nnThres) break;

				ws.setNode(0, 1, i2);
				if (!ws.getNode(0, 1)->isNN(ws.getNode(0, 0))) continue;

				int cnt3 = 0;
				for (int i3 : n1->nns) {
					if (i3 <= i1) continue;
					if (i3 == i2) continue;

					cnt3++;
					if (cnt3 > nnThres) break;

					ws.setNode(1, 0, i3);
					if (!ws.getNode(1, 0)->isNN(ws.getNode(0, 0))) continue;

					auto ns = ws.getNode(1, 0);

					int cnt4 = 0;
					for (int i4 : ns->nns) {

						foundFrame = false;

						if (i4 == i3 || i4 == i2 || i4 == i1) continue;

						cnt4++;
						if (cnt4 > nnThres) break;

						ws.setNode(1, 1, i4);
						if (!ws.getNode(1, 1)->isNN(ws.getNode(1, 0))) continue;
						if (!ws.getNode(1, 1)->isNN(ws.getNode(0, 1))) continue;

						if (!ws.isParallelBits(0, 0)) continue;

						hashVal = ws.hashVal(0, 0) + ws.hashVal(0, 1) + ws.hashVal(1, 0) + ws.hashVal(1, 1);
						
						if (quads.count(hashVal) != 0) continue;
						if (foundQuads.count(hashVal) != 0) continue;

						if (!ws.isParallel(0, 0)) continue;
						if (!ws.squareCheck(0, 0)) continue;
						

						int cnt5 = 0;
						auto ns2 = ws.getNode(0, 1);
						for (int i5 : ns2->nns) {
							if (foundFrame) break;
							if (i5 == i3 || i5 == i4 || i5 == i1) continue;
							if (i5 <= i1 || i5 <= i2) continue;

							cnt5++;
							if (cnt5 > nnThres) break;

							ws.setNode(0, 2, i5);

							int cnt6 = 0;
							auto ns3 = ws.getNode(0, 2);
							for (int i6 : ns3->nns) {
								if (i6 == i3 || i6 == i4 || i6 == i1) continue;

								cnt6++;
								if (cnt6 > nnThres) break;

								ws.setNode(1, 2, i6);
								if (!ws.getNode(1, 2)->isNN(ws.getNode(1, 1))) continue;
								if (!ws.getNode(1, 2)->isNN(ws.getNode(1, 0))) continue;

								if (!ws.isParallelBits(0, 1)) continue;
								
								hashVal = ws.hashVal(0, 1) + ws.hashVal(0, 2) + ws.hashVal(1, 1) + ws.hashVal(1, 2);
								if (quads.count(hashVal) != 0) continue;
								if (foundQuads.count(hashVal) != 0) continue;

								hashVal = ws.hashVal(0, 0) + ws.hashVal(0, 2) + ws.hashVal(1, 0) + ws.hashVal(1, 2);
								if (quads.count(hashVal) != 0) continue;
								if (foundQuads.count(hashVal) != 0) continue;

								if (!ws.isParallel(0, 1)) continue;
								if (!ws.isParallel(0, 0, 1, 2)) continue;

								if (!ws.squareCheck(0, 1)) continue;
								if (!ws.squareCheck(0, 0, 1, 2)) continue;

								if (m->options.extend2) {
									if (!extend() && m->options.extend3) {
										if (!extendDown()) addframe(ws);
										else {
											addframe(ws2);
										}
									}
									else {
										addframe(ws);
									}
								}
								else {
									if (m->options.extend3) {
										if (!extendDown()) addframe(ws);
										else {
											addframe(ws2);
										}
									}
									else {
										addframe(ws);
									}
								}

								// Need to clear nodes in workspace
								for (int k = 3; k < maxCols; k++) {
									ws.setNode(0, k, -1);
									ws.setNode(1, k, -1);
								}

								// Need to clear nodes in workspace
								for (int k = 2; k < maxCols; k++) {
									ws2.setNode(0, k, -1);
									ws2.setNode(1, k, -1);
									ws2.setNode(2, k, -1);
								}

								foundFrame = true;

								count++;
								break;
							}
						}
					}
				}
			}
		}


		void operator()() {

			fStore.clearFrames();
			foundQuads.clear();			

			for (int i1 : toProcess) {

				run6(i1);

			}
		}
	};

}



#endif

