#ifndef BRAINMATH_TESTING_UTILS_HPP
#define BRAINMATH_TESTING_UTILS_HPP

#include <string>
#include <typeinfo>

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

#else

template <typename T>
std::string type_name() {
	return typeid(T).name();
}

#endif

#endif  // BRAINMATH_TESTING_UTILS_HPP
