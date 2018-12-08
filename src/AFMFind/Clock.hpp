/**
 * \class Clock
 *
 * Simple clock for reporting execution times.
 *
 */

#ifndef NREP_CLOCK_HPP
#define NREP_CLOCK_HPP


#include <chrono>
#include <iostream>


namespace nrep {


class Clock {

	std::chrono::high_resolution_clock::time_point start;

public:

	Clock() {
		start = std::chrono::high_resolution_clock::now();
	}

	void report() {

		auto end = std::chrono::high_resolution_clock::now();
		auto diff_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << std::endl << "Time = " << ((float)diff_msec.count()) / 1000.0f << std::endl;
	}

};


}


#endif

