/**
 * \class StringUtils
 *
 * This class implemets some basic string prcessing functions.

 */


#ifndef NREP_STRING_UTILS_HPP
#define NREP_STRING_UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <functional> 
#include <cctype>
#include <cstring>
#include <locale>


namespace nrep {
	
	using std::vector;
	using std::string;


	std::string concat(vector<string> &strs) {

		string result;

		// First get length of new string
		size_t len = 0;
		for (auto & s : strs) len += s.length();
		len += (strs.size() + 1);

		char *buf = new char[len];
		char *ptr = buf;

		// Copy chars
		for (auto & s : strs) {
			if (ptr != buf) {
				ptr[0] = ' '; ptr++;
			}
			for (auto & c : s) {
				ptr[0] = c;
				ptr++;
			}
		}
		ptr[0] = '\0';

		// Assign string
		result.assign(buf);
		delete buf;

		return result;
	}

	// simple split
	// http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
	void split(std::vector<std::string> &toks, const char *str, char c = ' ') {		
		do
		{
			const char *begin = str;

			while (*str != c && *str) str++;
			toks.push_back(std::string(begin, str));

		} while (0 != *str++);
	}

	bool str_beginsWith(std::string &str, const std::string &prefix) {
		if (str.compare(0, prefix.size(), prefix) == 0) return true;
		else return false;
	}

	bool str_beginsWith(std::string &str, const char* prefix, size_t prefixSize) {
		if (str.compare(0, prefixSize, prefix) == 0) return true;
		else return false;
	}

	bool str_endsWith(std::string &str, const std::string &suffix) {
		if (suffix.length()>str.length()) return false;
		if (str.compare(str.length() - suffix.length(), suffix.length(), suffix)==0) return true;
		else return false;
	}

	void str_toLower(std::string &str) {
		std::transform(str.begin(), str.end(),
			str.begin(), ::tolower);
	}

	void str_toUpper(std::string &str) {
		std::transform(str.begin(), str.end(),
			str.begin(), ::toupper);
	}

	// trim from start
	static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
	}

	//-------------------------------------------------------------------------------------------
	// A version of getline for handling various line endings
	//  http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
	//
	// ----------------------------------------

	std::istream& safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
			}
		}
	}
	
}


#endif