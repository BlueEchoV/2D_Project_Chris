#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <Windows.h>
#define arraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

/**********************************************
 *
 * Defer
 *
 ***************/

template <typename T>
struct ExitScope
{
	T lambda;
	ExitScope(T lambda) : lambda(lambda) { }
	~ExitScope() { lambda(); }
	// NO_COPY(ExitScope);
};

struct ExitScopeHelp
{
	template<typename T>
	ExitScope<T> operator+(T t) { return t; }
};

template <typename T>
T clamp(T value, T min, T max) {
	if (value < min) {
		return min;
	}
	if (value > max) {
		return max;
	}
    return value;
}

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()
#define REF(v) (void)v

void log(const char* format, ...);

void my_Memory_Copy(void* dest, const void* src, size_t count);
