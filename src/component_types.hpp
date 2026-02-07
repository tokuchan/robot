static_assert(__cplusplus > 2020'00);
#pragma once

#include <boost/iterator/zip_iterator.hpp>
#include <boost/json.hpp>
#include <limits>
#include <tuple>
#include <vector>
#include "components.hpp"

/// @file component_types.hpp
/// @brief Type definitions for commonly used components in the ECS.
/// This file defines specific component types such as `Position`, `Velocity`, and `HitCounter`
/// that can be used in the entity-component system. Each component type is a simple struct that
/// holds relevant data for that aspect of an entity. These components can be
/// stored in the component storage defined in component.hpp.

namespace robot::src::detail::ecs::inline exports
{
    /// @brief Position component representing 2D coordinates.
    struct Position
    {
        float x, y; ///< Position of an entity in 2D space.

        /// @brief Move the position by a given velocity and time delta.
        /// @param velocity Velocity to apply to the position.
        /// @param delta_time Time delta in seconds.
        void move(const Velocity &velocity, float delta_time)
        {
            x += velocity.dx * delta_time;
            y += velocity.dy * delta_time;
        }

        /// @brief Stream output operator that formats the position as JSON.
        /// @param os Output stream to write to.
        /// @param pos Position to format.
        /// @return Reference to the output stream.
        friend std::ostream &operator<<(std::ostream &os, const Position &pos)
        {
            os << "{\"x\": " << pos.x << ", \"y\": " << pos.y << "}";
            return os;
        }

        /// @brief Stream input operator that parses a position from JSON format using boost::json.
        /// @param is Input stream to read from.
        /// @param pos Position to populate with parsed values.
        /// @return Reference to the input stream.
        friend std::istream &operator>>(std::istream &is, Position &pos)
        {
            boost::json::value json_value;
            is >> json_value;
            if (json_value.is_object())
            {
                auto const &obj = json_value.as_object();
                pos.x = obj.at("x").as_double();
                pos.y = obj.at("y").as_double();
            }
            else
            {
                is.setstate(std::ios::failbit);
            }
            return is;
        }
    };

    /// @brief Velocity component representing 2D motion.
    struct Velocity
    {
        float dx, dy; ///< Velocity of an entity in 2D space.
    };

    /// @brief Counter component storing the number of hits taken.
    struct HitCounter
    {
        std::uint32_t hits; ///< Number of hits an entity has taken.
    };

    /// @brief Simple 2D point type.
    struct Point2D
    {
        float x, y; ///< 2D point coordinates.
    };

    /// @brief Polygon represented by separate vectors of x and y vertex coordinates.
    struct Polygon
    {
        std::vector<float> vertices_x; ///< X-coordinates of the polygon's vertices.
        std::vector<float> vertices_y; ///< Y-coordinates of the polygon's vertices.

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
        std::tuple<float, float, float, float> get_aabb() const
        {
            float min_x = *std::min_element(vertices_x.begin(), vertices_x.end());
            float max_x = *std::max_element(vertices_x.begin(), vertices_x.end());
            float min_y = *std::min_element(vertices_y.begin(), vertices_y.end());
            float max_y = *std::max_element(vertices_y.begin(), vertices_y.end());
            return {min_x, min_y, max_x, max_y};
        }

        /// @brief Get the unnormalized normal vector of the edge at index i.
        /// @param i Edge index (uses i and (i + 1) % vertex_count).
        /// @return Pair containing the (x, y) components of the normal.
        std::pair<float, float> get_edge_normal(std::size_t i) const
        {
            std::size_t vertex_count = vertices_x.size();
            float edge_x = vertices_x[(i + 1) % vertex_count] - vertices_x[i];
            float edge_y = vertices_y[(i + 1) % vertex_count] - vertices_y[i];
            return {-edge_y, edge_x}; // Perpendicular to the edge
        }

        /// @brief Project a polygon onto the axis defined by (normal_x, normal_y).
        /// @param poly Polygon to project.
        /// @param normal_x X component of the axis normal.
        /// @param normal_y Y component of the axis normal.
        /// @return Tuple containing the minimum and maximum projection values.
        auto project_onto_axis(Polygon const &poly, float normal_x, float normal_y) const
        {
            float min_a = std::numeric_limits<float>::infinity();
            float max_a = -std::numeric_limits<float>::infinity();
            for (auto const &[vertex_x, vertex_y] : poly)
            {
                float projection = normal_x * vertex_x + normal_y * vertex_y;
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
                for (std::size_t i = 0; i < poly.vertices_x.size(); ++i)
                {
                    auto [normal_x, normal_y] = get_edge_normal(i);
                    auto [min_a, max_a] = project_onto_axis(*this, normal_x, normal_y);
                    auto [min_b, max_b] = project_onto_axis(other, normal_x, normal_y);

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
}

namespace robot::src::inline exports::inline ecs
{
    using namespace detail::ecs::exports;
}