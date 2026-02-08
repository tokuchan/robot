#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "component_types.hpp"

namespace ct = robot::src::exports::component_types;

SCENARIO("Position component operations", "[component_types][position]")
{
    GIVEN("two Position instances")
    {
        ct::Position p1{1.0f, 2.0f};
        ct::Position p2{3.0f, 4.0f};

        WHEN("adding positions")
        {
            auto result = p1 + p2;
            THEN("the result contains the sum of coordinates")
            {
                REQUIRE(result.x == 4.0f);
                REQUIRE(result.y == 6.0f);
            }
        }

        WHEN("subtracting positions")
        {
            auto result = p1 - p2;
            THEN("the result contains the difference of coordinates")
            {
                REQUIRE(result.x == -2.0f);
                REQUIRE(result.y == -2.0f);
            }
        }
    }

    GIVEN("a Position at the origin")
    {
        ct::Position origin{0.0f, 0.0f};

        WHEN("computing the dot product with itself")
        {
            auto result = robot::src::dot(origin, origin);
            THEN("the result should be zero")
            {
                REQUIRE(result == 0.0f);
            }
        }
    }

    GIVEN("a Position with non-zero coordinates")
    {
        ct::Position pos{3.0f, 4.0f};

        WHEN("computing its length")
        {
            auto len = robot::src::length(pos);
            THEN("the result should be the Euclidean distance from origin")
            {
                REQUIRE_THAT(len, Catch::Matchers::WithinAbs(5.0f, 1e-6f));
            }
        }

        WHEN("normalizing the position")
        {
            auto normalized = robot::src::normalized(pos);
            THEN("the normalized position should have length 1")
            {
                auto norm_len = robot::src::length(normalized);
                REQUIRE_THAT(norm_len, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
            }

            THEN("the normalized position should point in the same direction")
            {
                auto dot_product = robot::src::dot(pos, normalized);
                REQUIRE(dot_product > 0.0f);
            }
        }
    }
}

SCENARIO("Velocity component operations", "[component_types][velocity]")
{
    GIVEN("a Velocity component")
    {
        ct::Velocity vel{2.0f, 3.0f};

        WHEN("scaling the velocity")
        {
            auto scaled = vel * 2.0f;
            THEN("the components should be multiplied by the scalar")
            {
                REQUIRE(scaled.x == 4.0f);
                REQUIRE(scaled.y == 6.0f);
            }
        }

        WHEN("adding another velocity")
        {
            ct::Velocity other{1.0f, 1.0f};
            auto combined = vel + other;
            THEN("the result should be the vector sum")
            {
                REQUIRE(combined.x == 3.0f);
                REQUIRE(combined.y == 4.0f);
            }
        }

        WHEN("computing the magnitude")
        {
            auto magnitude = robot::src::length(vel);
            THEN("the magnitude should match the velocity's length")
            {
                REQUIRE_THAT(magnitude, Catch::Matchers::WithinAbs(3.60555f, 1e-5f));
            }
        }
    }

    GIVEN("perpendicular velocities")
    {
        ct::Velocity vel1{1.0f, 0.0f};
        ct::Velocity vel2{0.0f, 1.0f};

        WHEN("computing their dot product")
        {
            auto result = robot::src::dot(vel1, vel2);
            THEN("the result should be zero")
            {
                REQUIRE(result == 0.0f);
            }
        }
    }

    GIVEN("a velocity with magnitude greater than 1")
    {
        ct::Velocity vel{3.0f, 4.0f};

        WHEN("dividing by a scalar")
        {
            auto reduced = vel / 5.0f;
            THEN("the magnitude should be reduced proportionally")
            {
                auto original_len = robot::src::length(vel);
                auto reduced_len = robot::src::length(reduced);
                REQUIRE_THAT(reduced_len / original_len, Catch::Matchers::WithinAbs(0.2f, 1e-6f));
            }
        }
    }
}

SCENARIO("HitCounter component operations", "[component_types][hitcounter]")
{
    GIVEN("a newly created HitCounter")
    {
        ct::HitCounter counter;

        WHEN("checking the initial value (default constructed)")
        {
            THEN("the hits field should be zero-initialized")
            {
                REQUIRE(counter.hits == 0U);
            }
        }
    }

    GIVEN("a HitCounter initialized with 5 hits")
    {
        ct::HitCounter counter{5};

        WHEN("reading the hit count")
        {
            THEN("the value should be accessible")
            {
                REQUIRE(counter.hits == 5U);
            }
        }

        WHEN("incrementing the hit count")
        {
            counter.hits++;
            THEN("the value should increase by one")
            {
                REQUIRE(counter.hits == 6U);
            }
        }

        WHEN("adding multiple hits")
        {
            counter.hits += 10U;
            THEN("the value should reflect the total")
            {
                REQUIRE(counter.hits == 15U);
            }
        }
    }

    GIVEN("two HitCounters")
    {
        ct::HitCounter c1{3};
        ct::HitCounter c2{7};

        WHEN("comparing their values")
        {
            THEN("the comparison should work correctly")
            {
                REQUIRE(c1.hits < c2.hits);
                REQUIRE(c2.hits > c1.hits);
            }
        }

        WHEN("summing their hits")
        {
            auto total = c1.hits + c2.hits;
            THEN("the sum should be correct")
            {
                REQUIRE(total == 10U);
            }
        }
    }

    GIVEN("a HitCounter at maximum uint32_t value")
    {
        ct::HitCounter counter{std::numeric_limits<std::uint32_t>::max()};

        WHEN("checking the maximum value")
        {
            THEN("the value should be the maximum uint32_t")
            {
                REQUIRE(counter.hits == std::numeric_limits<std::uint32_t>::max());
            }
        }
    }
}

SCENARIO("Polygon component construction", "[component_types][polygon]")
{
    GIVEN("a Polygon constructed with pair vertices")
    {
        ct::Polygon poly({std::make_pair(0.0f, 0.0f),
                          std::make_pair(1.0f, 0.0f),
                          std::make_pair(1.0f, 1.0f),
                          std::make_pair(0.0f, 1.0f)});

        WHEN("iterating over vertices")
        {
            std::vector<std::pair<float, float>> vertices;
            for (auto [x, y] : poly)
            {
                vertices.push_back({x, y});
            }

            THEN("all vertices should be present")
            {
                REQUIRE(vertices.size() == 4U);
            }

            THEN("vertices should be in the correct order")
            {
                REQUIRE(vertices[0].first == 0.0f);
                REQUIRE(vertices[0].second == 0.0f);
                REQUIRE(vertices[3].first == 0.0f);
                REQUIRE(vertices[3].second == 1.0f);
            }
        }
    }

    GIVEN("a Polygon constructed with Vec2 vertices")
    {
        ct::Polygon poly({ct::Position{0.0f, 0.0f},
                          ct::Position{2.0f, 0.0f},
                          ct::Position{2.0f, 2.0f},
                          ct::Position{0.0f, 2.0f}});

        THEN("the polygon should have the correct vertex count")
        {
            REQUIRE(poly.size() == 4U);
            REQUIRE(poly.vertices_x.size() == 4U);
            REQUIRE(poly.vertices_y.size() == 4U);
        }
    }

    GIVEN("a default-constructed empty Polygon")
    {
        ct::Polygon empty_poly;

        THEN("the polygon should have no vertices")
        {
            REQUIRE(empty_poly.size() == 0U);
            REQUIRE(empty_poly.empty());
            REQUIRE(empty_poly.vertices_x.empty());
            REQUIRE(empty_poly.vertices_y.empty());
        }
    }
}

SCENARIO("Polygon AABB computation", "[component_types][polygon][aabb]")
{
    GIVEN("a square polygon from (0,0) to (1,1)")
    {
        ct::Polygon square({ct::Position{0.0f, 0.0f},
                            ct::Position{1.0f, 0.0f},
                            ct::Position{1.0f, 1.0f},
                            ct::Position{0.0f, 1.0f}});

        WHEN("computing the AABB")
        {
            auto aabb = square.get_aabb();

            THEN("the minimum bounds should be at (0, 0)")
            {
                REQUIRE(aabb.min.x == 0.0f);
                REQUIRE(aabb.min.y == 0.0f);
            }

            THEN("the maximum bounds should be at (1, 1)")
            {
                REQUIRE(aabb.max.x == 1.0f);
                REQUIRE(aabb.max.y == 1.0f);
            }
        }
    }

    GIVEN("a polygonwith negative coordinates")
    {
        ct::Polygon poly({ct::Position{-2.0f, -1.0f},
                          ct::Position{2.0f, -1.0f},
                          ct::Position{2.0f, 3.0f},
                          ct::Position{-2.0f, 3.0f}});

        WHEN("computing the AABB")
        {
            auto aabb = poly.get_aabb();

            THEN("the AABB should encompass all vertices")
            {
                REQUIRE(aabb.min.x == -2.0f);
                REQUIRE(aabb.min.y == -1.0f);
                REQUIRE(aabb.max.x == 2.0f);
                REQUIRE(aabb.max.y == 3.0f);
            }
        }
    }
}

SCENARIO("Polygon edge normal computation", "[component_types][polygon][normal]")
{
    GIVEN("a square polygon")
    {
        ct::Polygon square({ct::Position{0.0f, 0.0f},
                            ct::Position{1.0f, 0.0f},
                            ct::Position{1.0f, 1.0f},
                            ct::Position{0.0f, 1.0f}});

        WHEN("getting the normal of the first edge (0,0)-(1,0)")
        {
            auto normal = square.get_edge_normal(0);

            THEN("the normal should be perpendicular to the edge")
            {
                // Edge is horizontal (1,0), normal should be (0,1)
                REQUIRE(normal.x == 0.0f);
                REQUIRE(normal.y == 1.0f);
            }
        }

        WHEN("getting the normal of the second edge (1,0)-(1,1)")
        {
            auto normal = square.get_edge_normal(1);

            THEN("the normal should be perpendicular to the edge")
            {
                // Edge is vertical (0,1), normal should be (-1,0)
                REQUIRE(normal.x == -1.0f);
                REQUIRE(normal.y == 0.0f);
            }
        }
    }
}

SCENARIO("Polygon AABB intersection", "[component_types][polygon][collision]")
{
    GIVEN("two overlapping square AABBs")
    {
        ct::Polygon square1({ct::Position{0.0f, 0.0f},
                             ct::Position{2.0f, 0.0f},
                             ct::Position{2.0f, 2.0f},
                             ct::Position{0.0f, 2.0f}});

        ct::Polygon square2({ct::Position{1.0f, 1.0f},
                             ct::Position{3.0f, 1.0f},
                             ct::Position{3.0f, 3.0f},
                             ct::Position{1.0f, 3.0f}});

        WHEN("checking if square1 may intersect a list containing square2")
        {
            bool may_intersect = square1.may_intersect({square2});

            THEN("the result should be true")
            {
                REQUIRE(may_intersect);
            }
        }
    }

    GIVEN("two non-overlapping squares")
    {
        ct::Polygon square1({ct::Position{0.0f, 0.0f},
                             ct::Position{1.0f, 0.0f},
                             ct::Position{1.0f, 1.0f},
                             ct::Position{0.0f, 1.0f}});

        ct::Polygon square2({ct::Position{2.0f, 2.0f},
                             ct::Position{3.0f, 2.0f},
                             ct::Position{3.0f, 3.0f},
                             ct::Position{2.0f, 3.0f}});

        WHEN("checking if square1 may intersect a list containing square2")
        {
            bool may_intersect = square1.may_intersect({square2});

            THEN("the result should be false")
            {
                REQUIRE(!may_intersect);
            }
        }
    }

    GIVEN("multiple polygons, some overlapping and some not")
    {
        ct::Polygon main({ct::Position{0.0f, 0.0f},
                          ct::Position{2.0f, 0.0f},
                          ct::Position{2.0f, 2.0f},
                          ct::Position{0.0f, 2.0f}});

        ct::Polygon overlapping({ct::Position{1.0f, 1.0f},
                                 ct::Position{3.0f, 1.0f},
                                 ct::Position{3.0f, 3.0f},
                                 ct::Position{1.0f, 3.0f}});

        ct::Polygon non_overlapping({ct::Position{5.0f, 5.0f},
                                     ct::Position{6.0f, 5.0f},
                                     ct::Position{6.0f, 6.0f},
                                     ct::Position{5.0f, 6.0f}});

        WHEN("checking intersection against a list with both types")
        {
            bool may_intersect = main.may_intersect({overlapping, non_overlapping});

            THEN("the result should be true due to the overlapping polygon")
            {
                REQUIRE(may_intersect);
            }
        }
    }
}

SCENARIO("Polygon projection and SAT intersection", "[component_types][polygon][sat]")
{
    GIVEN("two overlapping axis-aligned squares")
    {
        ct::Polygon square1({ct::Position{0.0f, 0.0f},
                             ct::Position{2.0f, 0.0f},
                             ct::Position{2.0f, 2.0f},
                             ct::Position{0.0f, 2.0f}});

        ct::Polygon square2({ct::Position{1.0f, 1.0f},
                             ct::Position{3.0f, 1.0f},
                             ct::Position{3.0f, 3.0f},
                             ct::Position{1.0f, 3.0f}});

        WHEN("testing intersection using SAT")
        {
            bool intersects = square1.intersects(square2);

            THEN("the result should indicate intersection")
            {
                REQUIRE(intersects);
            }
        }
    }

    GIVEN("two touching axis-aligned squares")
    {
        ct::Polygon square1({ct::Position{0.0f, 0.0f},
                             ct::Position{1.0f, 0.0f},
                             ct::Position{1.0f, 1.0f},
                             ct::Position{0.0f, 1.0f}});

        ct::Polygon square2({ct::Position{1.0f, 0.0f},
                             ct::Position{2.0f, 0.0f},
                             ct::Position{2.0f, 1.0f},
                             ct::Position{1.0f, 1.0f}});

        WHEN("testing intersection using SAT")
        {
            bool intersects = square1.intersects(square2);

            THEN("the result should indicate intersection or touching")
            {
                REQUIRE(intersects);
            }
        }
    }

    GIVEN("two completely separated squares")
    {
        ct::Polygon square1({ct::Position{0.0f, 0.0f},
                             ct::Position{1.0f, 0.0f},
                             ct::Position{1.0f, 1.0f},
                             ct::Position{0.0f, 1.0f}});

        ct::Polygon square2({ct::Position{5.0f, 5.0f},
                             ct::Position{6.0f, 5.0f},
                             ct::Position{6.0f, 6.0f},
                             ct::Position{5.0f, 6.0f}});

        WHEN("testing intersection using SAT")
        {
            bool intersects = square1.intersects(square2);

            THEN("the result should indicate no intersection")
            {
                REQUIRE(!intersects);
            }
        }
    }
}

SCENARIO("Polygon projection onto axis", "[component_types][polygon][projection]")
{
    GIVEN("a unit square and x-axis normal")
    {
        ct::Polygon square({ct::Position{0.0f, 0.0f},
                            ct::Position{1.0f, 0.0f},
                            ct::Position{1.0f, 1.0f},
                            ct::Position{0.0f, 1.0f}});

        ct::Velocity x_axis{1.0f, 0.0f};

        WHEN("projecting the square onto the x-axis")
        {
            auto [min_proj, max_proj] = square.project_onto_axis(square, x_axis);

            THEN("the projection should span from 0 to 1")
            {
                REQUIRE_THAT(min_proj, Catch::Matchers::WithinAbs(0.0f, 1e-6f));
                REQUIRE_THAT(max_proj, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
            }
        }
    }

    GIVEN("a unit square and y-axis normal")
    {
        ct::Polygon square({ct::Position{0.0f, 0.0f},
                            ct::Position{1.0f, 0.0f},
                            ct::Position{1.0f, 1.0f},
                            ct::Position{0.0f, 1.0f}});

        ct::Velocity y_axis{0.0f, 1.0f};

        WHEN("projecting the square onto the y-axis")
        {
            auto [min_proj, max_proj] = square.project_onto_axis(square, y_axis);

            THEN("the projection should span from 0 to 1")
            {
                REQUIRE_THAT(min_proj, Catch::Matchers::WithinAbs(0.0f, 1e-6f));
                REQUIRE_THAT(max_proj, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
            }
        }
    }
}