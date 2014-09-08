#ifndef TIMEDIFF_HPP
#define TIMEDIFF_HPP

#include <chrono>

// TODO: Is unsigned long the right return type?
template<typename time>
unsigned long timediff(const time& start) {
	auto end = std::chrono::system_clock::now();

	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

#endif

