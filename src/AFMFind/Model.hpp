/**
 * \class Model
 *
 * This class is the primary class for holding the algorithm state.
 * Multiple threads will read from this object.

 */

#ifndef NREP_MODEL_H
#define NREP_MODEL_H

#include <random>

#include "Eigen/Eigen"

#include "Dictionary.hpp"
#include "MathUtils.hpp"
#include "VSpace.hpp"
#include "Node.hpp"
#include "RankedNNs.hpp"
#include "AFrameStore.hpp"
#include "Options.hpp"



namespace nrep {


	class Model {


	public:

		unordered_set<uint32_t> tris;
		unordered_set<uint32_t> tris2;

		int domainSize;
		vector<string> domainList;

		Eigen::MatrixXi nnMat;
		
		std::shared_ptr<nrep::VSpace> vs1;
		std::shared_ptr<nrep::VSpace> vs2;
		
		NodeStore nStore;

		nrep::AFrameStore fStore;
		nrep::AFrameStore fStore2;

		Options &options;
		
		Model(Options &opts) : options(opts) {

	
		}

		~Model() {

		}


		void init() {
			
			if (options.vecs != "") {
				vs1 = std::make_shared<VSpace>();
				vs1->loadVectors(options.vecs);
				vs1->normalizeVectors();
				vs1->makeBitVecs(1024);
			}

			vs2 = vs1;

			// Initialise frame store
			fStore.setDictionary(vs1->vStore);
			fStore2.setDictionary(vs1->vStore);
		
			cout << endl;
			cout << "Vector dimensionality: " << vs1->mat.rows() << endl;
			cout << "Max vectors to process: " << options.maxVectors << endl;

			prepareNodes();
			prepareDomain();

			// Initialize NodeStore
			domainSize = nStore.size();
			nStore.initialize();			
		}

		void prepareNodes() {
			for (int i = 0; i < vs1->vStore.size(); i++) {
				string s = vs1->vStore.getTermByIndex(i)->word;
				nStore.addNode(s);
			}			
		}

		void prepareDomain() {

			// Prepare the domain list
			for (int i = 0; i < options.maxVectors; i++) {
				string s = vs1->vStore.getTermByIndex(i)->word;
				domainList.push_back(s);
				auto n = nStore.getNode(s);
				n->isInDomain = true;
				//cout << endl << n->id;
			}

			nnMat.resize( options.maxNns, options.maxVectors );

			// Create domain nodes
			auto vsDomain = std::make_shared<VSpace>();
			vsDomain->mat.resize(vs1->mat.rows(), options.maxVectors);
			vsDomain->setIsBitVectorOwner(false);
			for (int i = 0; i < options.maxVectors; i++) {
				string s = vs1->vStore.getTermByIndex(i)->word;
				vsDomain->vStore.addTerm(s);
				vsDomain->mat.col(i) = vs1->mat.col(i);
				vsDomain->bitVecs.push_back(vs1->bitVecs[i]);				
			}

			computeNNs(*vsDomain, nnMat);

			for (int i = 0; i < domainList.size(); i++) {
				auto n = nStore.getNode(domainList[i]);
				int id1 = n->id;
				
				for (int j = 0; j < nnMat.rows(); j++) {
					int id2 = nnMat(j, id1);
					auto n2 = nStore.getNodeByIndex(id2);
					if (n2->isInDomain) {
						//fout << endl << n2->label;
						n->setAsNN(n2);
						n->nns.push_back(id2);
						if (n->nns.size()>= options.maxNns) break;
					}
				}
			}
		}
	};

}

#endif

