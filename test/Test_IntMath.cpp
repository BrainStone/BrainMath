#include <cmath>
#include <cstdint>
#include <limits>
#include <tuple>

#include "BrainMath/IntMath.hpp"
#include "gtest/gtest.h"
#include "TestingUtils.hpp"

template <BrainMath::Concepts::Integer T>
void mean_test() {
	constexpr T max = std::numeric_limits<T>::max();
	constexpr T min = std::numeric_limits<T>::min();

	for (const auto& [a, b, expected] : {std::tuple<T, T, T>{0, 0, 0},
	                                     {1, 0, 0},
	                                     {1, 1, 1},
	                                     {0, max, max / 2},
	                                     {1, max, max / 2 + 1},
	                                     {max, max, max},
	                                     {0, min, min / 2},
	                                     {1, min, min / 2},
	                                     {min, min, min}}) {
		EXPECT_EQ(BrainMath::IntMath::mean(a, b), expected);
		EXPECT_EQ(BrainMath::IntMath::mean(b, a), expected);
	}
}

template <BrainMath::Concepts::Integer T>
void add_overflow_test(
    std::function<std::pair<T, bool>(T, T)> add_overflow = BrainMath::IntMath::add_overflow<T, false>) {
	constexpr bool isSigned = std::is_signed_v<T>;
	constexpr T max = std::numeric_limits<T>::max();
	constexpr T min = std::numeric_limits<T>::min();

	for (const auto& [a, b, expected] : {std::tuple<T, T, bool>{0, 0, false},
	                                     {0, max, false},
	                                     {1, max, true},
	                                     {max, max, true},
	                                     {max / 2, max / 2, false},
	                                     {max / 2, max / 2 + 1, false},
	                                     {max / 2 + 1, max / 2 + 1, true},
	                                     {min, min, isSigned},
	                                     {min, max, false},
	                                     {min / 2, min / 2, false},
	                                     {min / 2 - 1, min / 2, isSigned}}) {
		EXPECT_EQ(add_overflow(a, b).second, expected) << type_name<T>() << ": Case " << a << " + " << b;
		EXPECT_EQ(add_overflow(b, a).second, expected) << type_name<T>() << ": Case " << b << " + " << a;
	}
}

template <BrainMath::Concepts::Integer T>
void sub_overflow_test(
    std::function<std::pair<T, bool>(T, T)> sub_overflow = BrainMath::IntMath::sub_overflow<T, false>) {
	constexpr bool isSigned = std::is_signed_v<T>;
	constexpr T max = std::numeric_limits<T>::max();
	constexpr T min = std::numeric_limits<T>::min();

	for (const auto& [a, b, expected] : {std::tuple<T, T, bool>{0, 0, false},
	                                     {0, 1, !isSigned},
	                                     {0, max, !isSigned},
	                                     {max, max, false},
	                                     {max / 2, max / 2, false},
	                                     {max / 2, max / 2 + 1, !isSigned},
	                                     {max / 2 + 1, max / 2, false},
	                                     {min, 0, false},
	                                     {min, 1, true},
	                                     {min / 2, min / 2, false},
	                                     {min / 2, min / 2 + 1, !isSigned},
	                                     {min / 2, max / 2, !isSigned},
	                                     {min / 2, max / 2 + 1, !isSigned},
	                                     {min / 2, max / 2 + 2, true}}) {
		EXPECT_EQ(sub_overflow(a, b).second, expected) << type_name<T>() << ": Case " << a << " - " << b;
	}
}

template <BrainMath::Concepts::Integer T>
void mul_overflow_test(
    std::function<std::pair<T, bool>(T, T)> mul_overflow = BrainMath::IntMath::mul_overflow<T, false>) {
	constexpr bool isSigned = std::is_signed_v<T>;
	constexpr T max = std::numeric_limits<T>::max();
	constexpr T min = std::numeric_limits<T>::min();

	for (const auto& [a, b, expected] : {std::tuple<T, T, bool>{0, 0, false},
	                                     {1, 1, false},
	                                     {0, max, false},
	                                     {1, max, false},
	                                     {2, max, true},
	                                     {2, max / 2, false},
	                                     {3, max / 2, true},
	                                     {2, max / 2 + 1, true}}) {
		EXPECT_EQ(mul_overflow(a, b).second, expected) << type_name<T>() << ": Case " << a << " * " << b;
		EXPECT_EQ(mul_overflow(b, a).second, expected) << type_name<T>() << ": Case " << b << " * " << a;
	}

	if constexpr (isSigned) {
		for (const auto& [a, b, expected] :
		     {std::tuple<T, T, bool>{-1, -1, false}, {1, min, false}, {2, min, true}, {-1, min, true}}) {
			EXPECT_EQ(mul_overflow(a, b).second, expected) << type_name<T>() << ": Case " << a << " * " << b;
			EXPECT_EQ(mul_overflow(b, a).second, expected) << type_name<T>() << ": Case " << b << " * " << a;
		}
	}
}

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW
template <BrainMath::Concepts::Integer T>
void random_overflow_parity_test(std::function<std::pair<T, bool>(T, T)> builtin,
                                 std::function<std::pair<T, bool>(T, T)> custom) {
	constexpr std::size_t count = 100'000;
	T a, b;

	auto uniform = uniform_random_number_generator<T>();
	auto gaussian = gaussian_random_number_generator<T>();

	for (std::size_t i = 0; i < count; ++i) {
		a = uniform();
		b = uniform();

		EXPECT_EQ(builtin(a, b), custom(a, b)) << type_name<T>() << ": Uniform case " << a << ", " << b;

		a = gaussian();
		b = gaussian();

		EXPECT_EQ(builtin(a, b), custom(a, b)) << type_name<T>() << ": Gaussian case " << a << ", " << b;
	}
}
#endif

template <BrainMath::Concepts::Integer T>
void sqrt_test() {
	constexpr bool isSigned = std::is_signed_v<T>;
	constexpr T max = std::numeric_limits<T>::max();

	if constexpr (isSigned) {
		errno = 0;
		std::feclearexcept(FE_ALL_EXCEPT);

		EXPECT_EQ(BrainMath::IntMath::sqrt<T>(-1), -1) << type_name<T>();
		EXPECT_EQ(errno, EDOM) << type_name<T>();
		EXPECT_TRUE(std::fetestexcept(FE_INVALID)) << type_name<T>();
	}

	for (const auto& [val, expected] :
	     {std::tuple<T, T>{0, 0}, {1, 1}, {3, 1}, {4, 2}, {max, static_cast<T>(sqrtl(max))}}) {
		EXPECT_EQ(BrainMath::IntMath::sqrt<T>(val), expected) << type_name<T>() << ": Case " << val;
	}

	constexpr std::size_t count = 100'000;
	T val;

	auto uniform = uniform_random_number_generator<T>();
	auto gaussian = gaussian_random_number_generator<T>();

	for (std::size_t i = 0; i < count; ++i) {
		val = uniform();
		// Force values to be positive (std::abs has issues in edge cases)
		if constexpr (isSigned) val &= max;

		EXPECT_EQ(BrainMath::IntMath::sqrt<T>(val), static_cast<T>(sqrtl(val)))
		    << type_name<T>() << ": Uniform case " << val;

		val = gaussian();
		// Force values to be positive (std::abs has issues in edge cases)
		if constexpr (isSigned) val &= max;

		EXPECT_EQ(BrainMath::IntMath::sqrt<T>(val), static_cast<T>(sqrtl(val)))
		    << type_name<T>() << ": Gaussian case " << val;
	}
}

TEST(IntMath, mean) {
	mean_test<std::int8_t>();
	mean_test<std::uint8_t>();
	mean_test<std::int16_t>();
	mean_test<std::uint16_t>();
	mean_test<std::int32_t>();
	mean_test<std::uint32_t>();
	mean_test<std::int64_t>();
	mean_test<std::uint64_t>();
}

TEST(IntMath, add_overflow) {
	add_overflow_test<std::int8_t>();
	add_overflow_test<std::uint8_t>();
	add_overflow_test<std::int16_t>();
	add_overflow_test<std::uint16_t>();
	add_overflow_test<std::int32_t>();
	add_overflow_test<std::uint32_t>();
	add_overflow_test<std::int64_t>();
	add_overflow_test<std::uint64_t>();
}

TEST(IntMath, sub_overflow) {
	sub_overflow_test<std::int8_t>();
	sub_overflow_test<std::uint8_t>();
	sub_overflow_test<std::int16_t>();
	sub_overflow_test<std::uint16_t>();
	sub_overflow_test<std::int32_t>();
	sub_overflow_test<std::uint32_t>();
	sub_overflow_test<std::int64_t>();
	sub_overflow_test<std::uint64_t>();
}

TEST(IntMath, mul_overflow) {
	mul_overflow_test<std::int8_t>();
	mul_overflow_test<std::uint8_t>();
	mul_overflow_test<std::int16_t>();
	mul_overflow_test<std::uint16_t>();
	mul_overflow_test<std::int32_t>();
	mul_overflow_test<std::uint32_t>();
	mul_overflow_test<std::int64_t>();
	mul_overflow_test<std::uint64_t>();
}

#ifdef BRAINMATH_HAS_BUILTIN_OVERFLOW

TEST(IntMath, add_overflow_parity) {
	auto test_parity = []<BrainMath::Concepts::Integer T>(T a, T b) {
		const std::pair<T, bool> builtin = BrainMath::IntMath::add_overflow<T, true>(a, b);
		const std::pair<T, bool> custom = BrainMath::IntMath::add_overflow<T, false>(a, b);

		EXPECT_EQ(builtin, custom);

		return custom;
	};

	// Critical values
	add_overflow_test<std::int8_t>(test_parity);
	add_overflow_test<std::uint8_t>(test_parity);
	add_overflow_test<std::int16_t>(test_parity);
	add_overflow_test<std::uint16_t>(test_parity);
	add_overflow_test<std::int32_t>(test_parity);
	add_overflow_test<std::uint32_t>(test_parity);
	add_overflow_test<std::int64_t>(test_parity);
	add_overflow_test<std::uint64_t>(test_parity);

	// Random values
	random_overflow_parity_test<std::int8_t>(BrainMath::IntMath::add_overflow<std::int8_t, true>,
	                                         BrainMath::IntMath::add_overflow<std::int8_t, false>);
	random_overflow_parity_test<std::uint8_t>(BrainMath::IntMath::add_overflow<std::uint8_t, true>,
	                                          BrainMath::IntMath::add_overflow<std::uint8_t, false>);
	random_overflow_parity_test<std::int16_t>(BrainMath::IntMath::add_overflow<std::int16_t, true>,
	                                          BrainMath::IntMath::add_overflow<std::int16_t, false>);
	random_overflow_parity_test<std::uint16_t>(BrainMath::IntMath::add_overflow<std::uint16_t, true>,
	                                           BrainMath::IntMath::add_overflow<std::uint16_t, false>);
	random_overflow_parity_test<std::int32_t>(BrainMath::IntMath::add_overflow<std::int32_t, true>,
	                                          BrainMath::IntMath::add_overflow<std::int32_t, false>);
	random_overflow_parity_test<std::uint32_t>(BrainMath::IntMath::add_overflow<std::uint32_t, true>,
	                                           BrainMath::IntMath::add_overflow<std::uint32_t, false>);
	random_overflow_parity_test<std::int64_t>(BrainMath::IntMath::add_overflow<std::int64_t, true>,
	                                          BrainMath::IntMath::add_overflow<std::int64_t, false>);
	random_overflow_parity_test<std::uint64_t>(BrainMath::IntMath::add_overflow<std::uint64_t, true>,
	                                           BrainMath::IntMath::add_overflow<std::uint64_t, false>);
}

TEST(IntMath, sub_overflow_parity) {
	auto test_parity = []<BrainMath::Concepts::Integer T>(T a, T b) {
		const std::pair<T, bool> builtin = BrainMath::IntMath::sub_overflow<T, true>(a, b);
		const std::pair<T, bool> custom = BrainMath::IntMath::sub_overflow<T, false>(a, b);

		EXPECT_EQ(builtin, custom);

		return custom;
	};

	// Critical values
	sub_overflow_test<std::int8_t>(test_parity);
	sub_overflow_test<std::uint8_t>(test_parity);
	sub_overflow_test<std::int16_t>(test_parity);
	sub_overflow_test<std::uint16_t>(test_parity);
	sub_overflow_test<std::int32_t>(test_parity);
	sub_overflow_test<std::uint32_t>(test_parity);
	sub_overflow_test<std::int64_t>(test_parity);
	sub_overflow_test<std::uint64_t>(test_parity);

	// Random values
	random_overflow_parity_test<std::int8_t>(BrainMath::IntMath::sub_overflow<std::int8_t, true>,
	                                         BrainMath::IntMath::sub_overflow<std::int8_t, false>);
	random_overflow_parity_test<std::uint8_t>(BrainMath::IntMath::sub_overflow<std::uint8_t, true>,
	                                          BrainMath::IntMath::sub_overflow<std::uint8_t, false>);
	random_overflow_parity_test<std::int16_t>(BrainMath::IntMath::sub_overflow<std::int16_t, true>,
	                                          BrainMath::IntMath::sub_overflow<std::int16_t, false>);
	random_overflow_parity_test<std::uint16_t>(BrainMath::IntMath::sub_overflow<std::uint16_t, true>,
	                                           BrainMath::IntMath::sub_overflow<std::uint16_t, false>);
	random_overflow_parity_test<std::int32_t>(BrainMath::IntMath::sub_overflow<std::int32_t, true>,
	                                          BrainMath::IntMath::sub_overflow<std::int32_t, false>);
	random_overflow_parity_test<std::uint32_t>(BrainMath::IntMath::sub_overflow<std::uint32_t, true>,
	                                           BrainMath::IntMath::sub_overflow<std::uint32_t, false>);
	random_overflow_parity_test<std::int64_t>(BrainMath::IntMath::sub_overflow<std::int64_t, true>,
	                                          BrainMath::IntMath::sub_overflow<std::int64_t, false>);
	random_overflow_parity_test<std::uint64_t>(BrainMath::IntMath::sub_overflow<std::uint64_t, true>,
	                                           BrainMath::IntMath::sub_overflow<std::uint64_t, false>);
}

TEST(IntMath, mul_overflow_parity) {
	auto test_parity = []<BrainMath::Concepts::Integer T>(T a, T b) {
		const std::pair<T, bool> builtin = BrainMath::IntMath::mul_overflow<T, true>(a, b);
		const std::pair<T, bool> custom = BrainMath::IntMath::mul_overflow<T, false>(a, b);

		EXPECT_EQ(builtin, custom);

		return custom;
	};

	// Critical values
	mul_overflow_test<std::int8_t>(test_parity);
	mul_overflow_test<std::uint8_t>(test_parity);
	mul_overflow_test<std::int16_t>(test_parity);
	mul_overflow_test<std::uint16_t>(test_parity);
	mul_overflow_test<std::int32_t>(test_parity);
	mul_overflow_test<std::uint32_t>(test_parity);
	mul_overflow_test<std::int64_t>(test_parity);
	mul_overflow_test<std::uint64_t>(test_parity);

	// Random values
	random_overflow_parity_test<std::int8_t>(BrainMath::IntMath::mul_overflow<std::int8_t, true>,
	                                         BrainMath::IntMath::mul_overflow<std::int8_t, false>);
	random_overflow_parity_test<std::uint8_t>(BrainMath::IntMath::mul_overflow<std::uint8_t, true>,
	                                          BrainMath::IntMath::mul_overflow<std::uint8_t, false>);
	random_overflow_parity_test<std::int16_t>(BrainMath::IntMath::mul_overflow<std::int16_t, true>,
	                                          BrainMath::IntMath::mul_overflow<std::int16_t, false>);
	random_overflow_parity_test<std::uint16_t>(BrainMath::IntMath::mul_overflow<std::uint16_t, true>,
	                                           BrainMath::IntMath::mul_overflow<std::uint16_t, false>);
	random_overflow_parity_test<std::int32_t>(BrainMath::IntMath::mul_overflow<std::int32_t, true>,
	                                          BrainMath::IntMath::mul_overflow<std::int32_t, false>);
	random_overflow_parity_test<std::uint32_t>(BrainMath::IntMath::mul_overflow<std::uint32_t, true>,
	                                           BrainMath::IntMath::mul_overflow<std::uint32_t, false>);
	random_overflow_parity_test<std::int64_t>(BrainMath::IntMath::mul_overflow<std::int64_t, true>,
	                                          BrainMath::IntMath::mul_overflow<std::int64_t, false>);
	random_overflow_parity_test<std::uint64_t>(BrainMath::IntMath::mul_overflow<std::uint64_t, true>,
	                                           BrainMath::IntMath::mul_overflow<std::uint64_t, false>);
}
#endif

TEST(IntMath, sqrt) {
	sqrt_test<std::int8_t>();
	sqrt_test<std::uint8_t>();
	sqrt_test<std::int16_t>();
	sqrt_test<std::uint16_t>();
	sqrt_test<std::int32_t>();
	sqrt_test<std::uint32_t>();
	sqrt_test<std::int64_t>();
	sqrt_test<std::uint64_t>();
}
