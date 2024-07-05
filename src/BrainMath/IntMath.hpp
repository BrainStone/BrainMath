#ifndef BRAINMATH_INTMATH_HPP
#define BRAINMATH_INTMATH_HPP

#include <algorithm>
#include <cerrno>
#include <cfenv>
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
 * @brief Calculates the mean (<code>(a + b) / 2</code>) of the passed parameters in an overflow safe manner.
 *
 * @tparam T Integer type
 * @param a First value
 * @param b Second value
 * @return <code>(a + b) / 2</code> calculated in an overflow safe manner, so even if <code>a + b</code> would overflow,
 * their mean is still calculated correctly.
 */
template <Concepts::Integer T>
[[nodiscard]] constexpr inline T mean(const T a, const T b) {
	// (a + b) / 2 <=> (a / 2) + (b / 2)
	// However, since we are in integer space, if both a and b are odd, we need to add 1 to the average using this
	// method.
	return (a / 2) + (b / 2) + (a & b & 1);
}

/**
 * @brief Adds two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the addition and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> add_overflow(const T a, const T b) {
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
 * @brief Subtracts two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the subtraction and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> sub_overflow(const T a, const T b) {
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
 * @brief Multiplies two integers with overflow detection.
 *
 * @tparam T Integer type
 * @tparam useBuiltin Whether to use compiler builtin overflow detection (if available)
 * @param a First value
 * @param b Second value
 * @return A pair containing the result of the multiplication and a boolean indicating if overflow occurred.
 */
template <Concepts::Integer T, bool useBuiltin = hasBuiltinOverflow>
[[nodiscard]] constexpr inline std::pair<T, bool> mul_overflow(const T a, const T b) {
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

/**
 * @brief Calculates the integer square root of a given value.
 *
 * This function computes the square root of the input value, rounding down to the nearest integer. It handles negative
 * inputs by setting errno to EDOM and raising an FE_INVALID exception, similar to std::sqrt. However, instead of
 * returning NaN (integers don't have that value), we return -1.<br>
 * Several optimizations were put in place to ensure a
 * fast algorithm. It does not use floating point math or division (except for division through the constant 2, which
 * the compiler optimizes to a bitshift).
 *
 * @tparam T Integer type
 * @param val The value for which to calculate the square root.
 * @return The integer square root of val.
 */
template <BrainMath::Concepts::Integer T>
[[nodiscard]] constexpr inline T sqrt(const T val) {
	// Slightly too large max_value right can be. For unsigned values this is pretty precise, for signed values we
	// overshoot quite a bit, but it's not worth refining this
	constexpr T max_right = (std::numeric_limits<T>::max() >> (std::numeric_limits<T>::digits / 2)) + 1;
	constexpr T zero = T{0};
	constexpr T two = T{2};

	if (val < zero) [[unlikely]] {
		// Behave like std::sqrt would
		errno = EDOM;
		std::feraiseexcept(FE_INVALID);

		// If we can be below 0, we can return a negative value
		return -1;
	} else if (val == zero) [[unlikely]] {
		return zero;
	} else if (val < T{4}) [[unlikely]] {
		return 1;
	}

	// Narrow down the range after all the checks beforehand
	T left = two;
	// The square root can never be bigger than half the starting value (after having eliminated values smaller than 4),
	// which saves us one iteration.
	T right = val / two;
	// We can skip a lot of iterations by at most checking values up to the estimated (but guaranteed to be larger)
	// square root of the max value of the integer type
	right = std::min(right, max_right);
	T middle;
	std::pair<T, bool> square;

	while (true) {
		// Since this can never result in an overflow, we use the faster method here instead of the safe
		// BrainMath::IntMath::mean method.
		middle = (left + right) / two;

		square = mul_overflow(middle, middle);

		if (square.second || (square.first > val)) [[likely]] {
			right = middle;
		} else if (square.first < val) {
			left = middle;
		} else if (square.first == val) [[unlikely]] {
			return middle;
		}

		if (right - left < two) [[unlikely]] {
			return left;
		}
	}
}

}  // namespace BrainMath::IntMath

#endif  // BRAINMATH_INTMATH_HPP
