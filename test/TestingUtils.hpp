#ifndef BRAINMATH_TESTING_UTILS_HPP
#define BRAINMATH_TESTING_UTILS_HPP

#include <coroutine>
#include <random>
#include <string>
#include <typeinfo>

#include "BrainMath/Concepts.hpp"

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
// clang-format off
// Import order matters here!
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#pragma comment(lib, "dbghelp.lib")

// Remove evil preprocessor directives
#undef min
#undef max

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

// Coroutine generator for C++20
template <typename T>
struct Generator {
	struct promise_type {
		T current_value;
		std::suspend_always yield_value(T value) {
			current_value = value;
			return {};
		}
		std::suspend_always initial_suspend() {
			return {};
		}
		std::suspend_always final_suspend() noexcept {
			return {};
		}
		Generator get_return_object() {
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}
		void return_void() {}
		void unhandled_exception() {
			std::terminate();
		}
	};

	using handle_type = std::coroutine_handle<promise_type>;
	handle_type handle;

	explicit Generator(handle_type h) : handle(h) {}
	~Generator() {
		if (handle) handle.destroy();
	}

	T operator()() {
		const T val = handle.promise().current_value;
		handle.resume();

		return val;
	}
};

// Coroutine to generate uniformly distributed random integers
template <BrainMath::Concepts::Integer T>
Generator<T> uniform_random_number_generator() {
	constexpr T min = std::numeric_limits<T>::min();
	constexpr T max = std::numeric_limits<T>::max();

	// NOLINTNEXTLINE: We WANT consistent results!
	std::mt19937 gen(123);
#ifdef _MSC_VER
	using AtLeast16BitInt = typename std::common_type<T, std::int16_t>::type;
	// MSC doesn't allow 8 bit ints here
	std::uniform_int_distribution<AtLeast16BitInt> dis(min, max);
#else
	std::uniform_int_distribution<T> dis(min, max);
#endif

	while (true) {
		co_yield static_cast<T>(dis(gen));
	}
}

// Coroutine to generate Gaussian-distributed random integers
template <BrainMath::Concepts::Integer T>
Generator<T> gaussian_random_number_generator() {
	constexpr double mean = 0.0;
	constexpr double stddev = static_cast<double>(std::numeric_limits<T>::max()) / 4.0;  // A rough estimate

	// NOLINTNEXTLINE: We WANT consistent results!
	std::mt19937 gen(321);
	std::normal_distribution<double> dis(mean, stddev);

	while (true) {
		double value = dis(gen);

		if constexpr (std::is_signed<T>::value) {
			co_yield static_cast<T>(std::round(value));
		} else {
			co_yield static_cast<T>(std::round(std::abs(value)));
		}
	}
}

#endif  // BRAINMATH_TESTING_UTILS_HPP
