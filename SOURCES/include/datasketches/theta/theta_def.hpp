#ifndef VERTICA_UDFS_THETA_DEF_HPP
#define VERTICA_UDFS_THETA_DEF_HPP

#include <theta_sketch.hpp>
#include <theta_union.hpp>
#include <theta_intersection.hpp>
#include "../custom_alloc.hpp"

typedef datasketches::update_theta_sketch_alloc <custom_alloc<int>> update_theta_sketch_custom;
typedef datasketches::theta_intersection_alloc <custom_alloc<int>> theta_intersection_custom;
typedef datasketches::theta_union_alloc <custom_alloc<int>> theta_union_custom;
typedef datasketches::compact_theta_sketch_alloc <custom_alloc<int>> compact_theta_sketch_custom;
typedef datasketches::theta_a_not_b_alloc<custom_alloc<int>> theta_a_not_b_custom;

#endif //VERTICA_UDFS_THETA_DEF_HPP
