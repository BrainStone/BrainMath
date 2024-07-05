#ifndef BRAINMATH_INTMATH_HPP
#define BRAINMATH_INTMATH_HPP

#include <utility>

#include "BrainMath/Concepts.hpp"

namespace BrainMath::IntMath {

#ifdef __has_builtin
#if __has_builtin(__builtin_add_overflow) && __has_builtin(__builtin_sub_overflow) && \
    __has_builtin(__builtin_mul_overflow)
#define BRAINMATH_HAS_BUILTIN_OVERFLOW
#endif
#endif

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW
constexpr bool hasBuiltinOverflow = true;
#else
constexpr bool hasBuiltinOverflow = false;
#endif

/**
 * Calculates the mean (<code>(a + b) / 2</code>) of the passed parameters in an overflow safe manner.
 *
 * @tparam T Integer type
 * @param a First value
 * @param b Second value
 * @return <code>(a + b) / 2</code> calculated in an overflow safe manner, so even if <code>a + b</code> would overflow,
 * their mean is still calculated correctly.
 */
template <Concepts::Integer T>
[[nodiscard]] constexpr inline T mean(T a, T b) {
	// (a + b) / 2 <=> (a / 2) + (b / 2)
	// However, since we are in integer space, if both a and b are odd, we need to add 1 to the average using this
	// method.
	return (a / 2) + (b / 2) + (a & b & 1);
}

/**
 * Adds two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the addition and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> add_overflow(T a, T b) {
	T result;
	bool overflow;

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW
	if constexpr (useBuiltin) {
		overflow = __builtin_add_overflow(a, b, &result);

		return std::make_pair(result, overflow);
	}
#endif

	// Potentially UB here, but all sane compilers store the truncated result here. Additionally, if an overflow
	// happened, you shouldn't be reading this value anyway.
	result = a + b;

	if constexpr (std::is_signed_v<T>) {
		overflow = (b > 0 && a > std::numeric_limits<T>::max() - b) || (b < 0 && a < std::numeric_limits<T>::min() - b);
	} else {
		overflow = a > std::numeric_limits<T>::max() - b;
	}

	return std::make_pair(result, overflow);
}

/**
 * Subtracts two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the subtraction and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> sub_overflow(T a, T b) {
	T result;
	bool overflow;

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW
	if constexpr (useBuiltin) {
		overflow = __builtin_sub_overflow(a, b, &result);

		return std::make_pair(result, overflow);
	}
#endif

	// Potentially UB here, but all sane compilers store the truncated result here. Additionally, if an overflow
	// happened, you shouldn't be reading this value anyway.
	result = a - b;

	if constexpr (std::is_signed_v<T>) {
		overflow = (b < 0 && a > std::numeric_limits<T>::max() + b) || (b > 0 && a < std::numeric_limits<T>::min() + b);
	} else {
		overflow = a < b;
	}

	return std::make_pair(result, overflow);
}

/**
 * Multiplies two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the multiplication and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> mul_overflow(T a, T b) {
	T result;
	bool overflow;

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW
	if constexpr (useBuiltin) {
		overflow = __builtin_mul_overflow(a, b, &result);

		return std::make_pair(result, overflow);
	}
#endif

	// Potentially UB here, but all sane compilers store the truncated result here. Additionally, if an overflow
	// happened, you shouldn't be reading this value anyway.
	result = a * b;

	if constexpr (std::is_signed_v<T>) {
		if (a == 0 || b == 0) {
			overflow = false;
		} else if (a == -1 || b == -1) {
			overflow =
			    (a == -1 && b == std::numeric_limits<T>::min()) || (b == -1 && a == std::numeric_limits<T>::min());
		} else if (a < 0 && b < 0) {
			overflow = a < std::numeric_limits<T>::max() / b;
		} else if (a < 0) {
			overflow = a < std::numeric_limits<T>::min() / b;
		} else if (b < 0) {
			overflow = b < std::numeric_limits<T>::min() / a;
		} else {
			overflow = a > std::numeric_limits<T>::max() / b;
		}
	} else {
		overflow = (a != 0 && b != 0) && (a > std::numeric_limits<T>::max() / b);
	}

	return std::make_pair(result, overflow);
}

}  // namespace BrainMath::IntMath

#endif  // BRAINMATH_INTMATH_HPP
