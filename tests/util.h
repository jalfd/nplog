#ifndef NPLOG_UTIL_H
#define NPLOG_UTIL_H

namespace {
	template <typename F>
	inline void repeat(const F& func, int iterations = 1000)
	{
		for (int i = 0; i < iterations; ++i) {
			func();
		}
	}
}

#endif
