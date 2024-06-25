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

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

void log(const char* format, ...);
