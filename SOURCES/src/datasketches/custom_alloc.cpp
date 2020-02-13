#include "../../include/datasketches/custom_alloc.hpp"

std::atomic<int> custom_alloc_state::size_used{0};
const int64_t custom_alloc_state::size_max{1LL * 1024 * 1024 * 1024 * 10}; // 10GB
