/**
 * \class DictEntry
 *
 * A entry in the dictionary.
 *
 * Also contains data for building a HuffTree over DictEntry
 */

#ifndef NREP_DICT_ENTRY_H
#define NREP_DICT_ENTRY_H

#include <iostream>
#include <memory>
#include <cstring>

#define CODE_NUM_CHARS 40


namespace nrep {


	using std::shared_ptr;
	using std::cout;
	using std::endl;
	using std::string;

	struct DictEntry {

		std::string word;
		uint64_t cn;
		uint32_t id;

		float entropy;

		int *point;
		int codelen;
		char code[CODE_NUM_CHARS];

		shared_ptr<DictEntry> parent;

		DictEntry() {
			cn = 0;
			codelen = 0;
			entropy = 0.0f;
			setAllZero();
		}

		void setWord(const char* w) {
			word = w;
		}

		void setWord(const string & w) {
			word = w;
		}

		int codeAt(int i) {
			return ((int)code[i]);
		}

		void set(int i) {
			code[i] = 1;
		}

		void printCode() {
			cout << endl;
			if (codelen == 0) return;
			for (int i = 0; (i < codelen) && (i < CODE_NUM_CHARS); i++) {
				cout << codeAt(i);
			}
		}

		void setAllZero() {
			std::memset(code, 0, CODE_NUM_CHARS);
		}

		friend class comp;
		class comp {
		public:
			bool operator()(shared_ptr<DictEntry> a, shared_ptr<DictEntry> b) {
				return a->cn > b->cn;
			}
		};
	};

}


#endif 
