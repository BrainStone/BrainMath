#ifndef BRAINMATH_CONCEPTS_HPP
#define BRAINMATH_CONCEPTS_HPP

#include <type_traits>

namespace BrainMath::Concepts {

template <typename T>
concept Integer = std::is_integral_v<T>;

template <typename T>
concept SignedInteger = Integer<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedInteger = Integer<T> && std::is_unsigned_v<T>;

}  // namespace BrainMath::Concepts

#endif  // BRAINMATH_CONCEPTS_HPP
