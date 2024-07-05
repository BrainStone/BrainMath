#ifndef BRAINMATH_TESTING_UTILS_HPP
#define BRAINMATH_TESTING_UTILS_HPP

#include <string>
#include <typeinfo>

// Check for GCC or Clang
#if defined(__GNUG__) || defined(__clang__)
#include <cxxabi.h>

#include <cstdlib>

template <typename T>
std::string type_name() {
	int status;
	std::string tname = typeid(T).name();
	char* demangled_name = abi::__cxa_demangle(tname.c_str(), nullptr, nullptr, &status);
	if (status == 0) {
		tname = demangled_name;
		std::free(demangled_name);
	}
	return tname;
}

// Check for MSVC
#elif defined(_MSC_VER)
#include <dbghelp.h>
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")

template <typename T>
std::string type_name() {
	std::string tname = typeid(T).name();
	char demangled_name[1024];

	if (UnDecorateSymbolName(tname.c_str(), demangled_name, sizeof(demangled_name), UNDNAME_COMPLETE)) {
		tname = demangled_name;
	}

	return tname;
}

// Fallback for other compilers
#else

template <typename T>
std::string type_name() {
	return typeid(T).name();
}

#endif

#endif  // BRAINMATH_TESTING_UTILS_HPP
