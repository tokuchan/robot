static_assert(__cplusplus > 2020'00);
#pragma once

#include <boost/iterator/zip_iterator.hpp>
#include <limits>
#include <tuple>
#include <vector>
#include "components.hpp"
#include "math.hpp"

/// @file component_types.hpp
/// @brief Type definitions for commonly used components in the ECS.
/// This file defines specific component types such as `Position`, `Velocity`, and `HitCounter`
/// that can be used in the entity-component system. Each component type is a simple struct that
/// holds relevant data for that aspect of an entity. These components can be
/// stored in the component storage defined in component.hpp.

namespace robot::src::detail::component_types::inline exports
{
    /// @brief Velocity component representing 2D motion.
    using Velocity = Vec2;

    /// @brief Position component representing 2D coordinates.
    using Position = Vec2;

    /// @brief Player input component representing directional input.
    using PlayerInput = Vec2;

    /// @brief Counter component storing the number of hits taken.
    struct HitCounter
    {
        std::uint32_t hits = 0; ///< Number of hits an entity has taken.
    };

    /// @brief Polygon represented by separate vectors of x and y vertex coordinates.
    struct Polygon
    {
        std::vector<Float> vertices_x; ///< X-coordinates of the polygon's vertices.
        std::vector<Float> vertices_y; ///< Y-coordinates of the polygon's vertices.

        /// @brief Construct a polygon from a list of vertices.
        /// @param vertices List of (x, y) vertex coordinates.
        Polygon(std::initializer_list<std::pair<Float, Float>> vertices)
        {
            for (const auto &[x, y] : vertices)
            {
                vertices_x.push_back(x);
                vertices_y.push_back(y);
            }
        }

        /// @brief Construct a polygon from a list of Vec2 vertices.
        /// @param vertices List of Vec2 vertex coordinates.
        Polygon(std::initializer_list<Vec2> vertices)
        {
            for (const auto &v : vertices)
            {
                vertices_x.push_back(v.x);
                vertices_y.push_back(v.y);
            }
        }

        /// @brief Default constructor for an empty polygon.
        Polygon() = default;

        /// @brief Return the number of vertices in the polygon.
        /// @return The vertex count.
        std::size_t size() const
        {
            assert(vertices_x.size() == vertices_y.size());
            return vertices_x.size();
        }

        /// @brief Return true if this polygon has no vertices.
        /// @return True if the polygon is empty; otherwise false.
        bool empty() const
        {
            return vertices_x.empty() && vertices_y.empty();
        }

        /// @brief Begin an iterator over the polygon's vertices.
        /// @return Iterator that yields tuples of (x, y) coordinates.
        auto begin() const
        {
            return boost::make_zip_iterator(boost::make_tuple(vertices_x.begin(), vertices_y.begin()));
        }

        /// @brief End an iterator over the polygon's vertices.
        /// @return Iterator marking the end of the vertex sequence.
        auto end() const
        {
            return boost::make_zip_iterator(boost::make_tuple(vertices_x.end(), vertices_y.end()));
        }

        /// @brief Compute the AABB (axis-aligned bounding box) of the polygon.
        /// @return Tuple containing (min_x, min_y, max_x, max_y).
        AxisAlignedBoundingBox get_aabb() const
        {
            Float min_x = *std::min_element(vertices_x.begin(), vertices_x.end());
            Float max_x = *std::max_element(vertices_x.begin(), vertices_x.end());
            Float min_y = *std::min_element(vertices_y.begin(), vertices_y.end());
            Float max_y = *std::max_element(vertices_y.begin(), vertices_y.end());
            return {{min_x, min_y}, {max_x, max_y}};
        }

        /// @brief Test to see if this Polygon may intersect any other Polygons in a list using AABB checks.
        /// @param others List of other Polygons to test against.
        /// @return True if this Polygon's AABB overlaps with any other Polygon's AABB; otherwise false.
        bool may_intersect(const std::vector<Polygon> &others) const
        {
            auto aabb = get_aabb();
            for (const auto &other : others)
            {
                auto other_aabb = other.get_aabb();
                if (aabb.intersects(other_aabb))
                {
                    return true; // AABBs overlap, so intersection is possible
                }
            }
            return false; // No overlaps found
        }

        /// @brief Get the unnormalized normal vector of the edge at index i.
        /// @param i Edge index (uses i and (i + 1) % vertex_count).
        /// @return Pair containing the (x, y) components of the normal.
        Vec2 get_edge_normal(std::size_t i) const
        {
            std::size_t vertex_count = this->size();
            Float edge_x = vertices_x[(i + 1) % vertex_count] - vertices_x[i];
            Float edge_y = vertices_y[(i + 1) % vertex_count] - vertices_y[i];
            return {-edge_y, edge_x}; // Perpendicular to the edge
        }

        /// @brief Project a polygon onto the axis defined by (normal_x, normal_y).
        /// @param poly Polygon to project.
        /// @param normal Normal vector of the axis.
        /// @return Tuple containing the minimum and maximum projection values.
        auto project_onto_axis(Polygon const &poly, Vec2 normal) const
        {
            Float min_a = std::numeric_limits<Float>::infinity();
            Float max_a = -std::numeric_limits<Float>::infinity();
            for (auto const &[vertex_x, vertex_y] : poly)
            {
                Float projection = normal.x * vertex_x + normal.y * vertex_y;
                min_a = std::min(min_a, projection);
                max_a = std::max(max_a, projection);
            }
            return std::make_tuple(min_a, max_a);
        }

        /// @brief Determine whether this polygon intersects another using the SAT.
        /// @param other Polygon to test against.
        /// @return True when the polygons intersect; otherwise false.
        bool intersects(const Polygon &other) const
        {
            auto checkSeparationWith = [*this, &other](const Polygon &poly)
            {
                // Check for separation on this polygon's edges
                for (std::size_t i = 0; i < poly.size(); ++i)
                {
                    auto normal = get_edge_normal(i);
                    auto [min_a, max_a] = project_onto_axis(*this, normal);
                    auto [min_b, max_b] = project_onto_axis(other, normal);

                    if (max_a < min_b or max_b < min_a)
                    {
                        return false; // Separation found
                    }
                }
                return true;
            };

            return checkSeparationWith(*this) && checkSeparationWith(other);
        }
    };

    using EntityStore = Components<Position, Velocity, PlayerInput, HitCounter, Polygon>;
}

namespace robot::src::inline exports::inline component_types
{
    using namespace detail::component_types::exports;
}