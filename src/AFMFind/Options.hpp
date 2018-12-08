/**
 * \class Options
 *
 * This options for AFMFind

 */

#ifndef NREP_OPTIONS_HPP
#define NREP_OPTIONS_HPP


#include <iostream>
#include <string>

using std::string;


namespace nrep {


	struct Options {

		string vecs;
		string frames;

		int maxExtendNum;		
		int maxVectors;				
		int nns;
		int extNns;	
		int maxNns; // Max of nns and extNns
		float p;
		bool extend2;
		bool extend3;
		int numThreads;

		int samples;


		int argPos(const char *str, int argc, const char **argv) {
			int a;
			for (a = 1; a < argc; a++) {
				if (!strcmp(str, argv[a])) {
					if (a == argc - 1) {
						std::cout << "Argument missing for " << str << std::endl;
						exit(1);
					}
					return a;
				}
			}
			return -1;
		}

		void parseOptions(int argc, const char** argv) {

			int i;
			string modelName;

			// Inputs
			if ((i = argPos((char *)"-vecs", argc, argv)) > 0) vecs = argv[i + 1];
			if ((i = argPos((char *)"-frames", argc, argv)) > 0) frames = argv[i + 1];

			if ((i = argPos((char *)"-extend2", argc, argv)) > 0) extend2 = atoi(argv[i + 1]);
			if ((i = argPos((char *)"-extend3", argc, argv)) > 0) extend3 = atoi(argv[i + 1]);

			if ((i = argPos((char *)"-p", argc, argv)) > 0) p = static_cast<float>(atof(argv[i + 1]));

			if ((i = argPos((char *)"-nns", argc, argv)) > 0) nns = atoi(argv[i + 1]);
			if ((i = argPos((char *)"-ext-nns", argc, argv)) > 0) extNns = atoi(argv[i + 1]);

			if ((i = argPos((char *)"-max-ext", argc, argv)) > 0) maxExtendNum = atoi(argv[i + 1]);
			if ((i = argPos((char *)"-max-vecs", argc, argv)) > 0) maxVectors = atoi(argv[i + 1]);

			if ((i = argPos((char *)"-threads", argc, argv)) > 0) numThreads = atoi(argv[i + 1]);

			if (nns > extNns) maxNns = nns;
			else maxNns = extNns;

			cout << endl << "Parallel threshold: " << p << endl;
		}

		Options() {
			
			vecs = "";
			frames = "";

			nns = 12;
			extNns = 30;

			maxVectors = 20000;
			maxExtendNum = 20;

			numThreads = 4;

			p = 0.3f;
		}

		void print() {

		}
	};

} // namespace

#endif




