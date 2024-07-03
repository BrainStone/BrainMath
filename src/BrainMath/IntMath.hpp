#ifndef BRAINMATH_INTMATH_HPP
#define BRAINMATH_INTMATH_HPP

#include "BrainMath/Concepts.hpp"

namespace BrainMath::IntMath {

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

}  // namespace BrainMath::IntMath

#endif  // BRAINMATH_INTMATH_HPP
