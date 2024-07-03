#include <cstdint>
#include <limits>
#include <tuple>

#include "BrainMath/IntMath.hpp"
#include "gtest/gtest.h"

template <BrainMath::Concepts::Integer T>
void mean_test() {
	for (const auto& [a, b, res] : std::initializer_list<std::tuple<T, T, T>>{
	         {0, 0, 0},
	         {1, 0, 0},
	         {0, 1, 0},
	         {1, 1, 1},
	         {0, std::numeric_limits<T>::max(), std::numeric_limits<T>::max() / 2},
	         {std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max()},
	         {0, std::numeric_limits<T>::min(), std::numeric_limits<T>::min() / 2},
	         {std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), std::numeric_limits<T>::min()}}) {
		EXPECT_EQ(BrainMath::IntMath::mean(a, b), res);
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
