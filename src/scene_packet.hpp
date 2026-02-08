static_assert( __cplusplus > 2020'00 );
#pragma once

#include <boost/json.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace robot::src::detail::scene_packet::inline exports
{
struct ScenePacket
{
    // This is a simple struct that will be serialized to JSON and sent to
    // the client.  It contains the necessary information for rendering the
    // scene, such as the positions and shapes of entities. For simplicity,
    // we'll just include polygons with their positions.

    // N.b. The geometry here is meant to be rendered by an HTML canvas, so
    // we must do the necessary transformations to convert from the robot's
    // coordinate system to the canvas coordinate system on the client side.
    // This includes flipping the y-axis and applying any necessary scaling.
    // Then, we store just the raw polygon drawing instructions. This way,
    // the client can be really dumb and just render the polygons as they
    // are without needing to understand the robot's coordinate system.

    // We need to store the stroke width and color, as well as the fill
    // color for each polygon, followed by the vertex list. The client will
    // use the stroke color and width to draw the outline of the polygon,
    // and the fill color to fill it in. This allows for more flexible
    // rendering on the client side, as different entities can have
    // different visual styles.

    std::vector< std::tuple< float, std::string, std::string, std::vector< std::pair< float, float > > > > geometries;

    /// @brief Add a polygon to the scene packet.
    /// @param stroke_width Width of the polygon's outline.
    /// @param stroke_color Color of the polygon's outline (e.g., "#FF0000" for red).
    /// @param fill_color Color to fill the polygon (e.g., "#00FF00" for green).
    /// @param vertices List of (x, y) vertex coordinates for the polygon.
    void add_polygon(
        float stroke_width,
        std::string stroke_color,
        std::string fill_color,
        std::vector< std::pair< float, float > > vertices )
    {
        geometries.emplace_back( stroke_width, stroke_color, fill_color, std::move( vertices ) );
    }

    /// @brief Serialize the scene packet to a JSON string.
    /// @return JSON string representation of the scene packet.
    std::string to_json() const
    {
        // We'll use Boost.JSON to serialize the scene packet to JSON. The structure will be:
        // {
        //   "geometries": [
        //     {
        //       "stroke_width": 2.0,
        //       "stroke_color": "#FF0000",
        //       "fill_color": "#00FF00",
        //       "vertices": [[0, 0], [1, 0], [1, 1], [0, 1]]
        //     },
        //     ...
        //   ]
        // }
        boost::json::object scene;
        boost::json::array geometries_json;

        for( const auto & [ stroke_width, stroke_color, fill_color, vertices ] : geometries )
        {
            boost::json::object geo;
            geo[ "stroke_width" ] = stroke_width;
            geo[ "stroke_color" ] = stroke_color;
            geo[ "fill_color" ] = fill_color;

            boost::json::array vertices_json;
            for( const auto & [ x, y ] : vertices )
            {
                vertices_json.push_back( boost::json::array{ x, y } );
            }
            geo[ "vertices" ] = vertices_json;

            geometries_json.push_back( geo );
        }
        scene[ "geometries" ] = geometries_json;
        return boost::json::serialize( scene );
    }

    /// @brief Define the ostream operator for easy printing of the scene packet.
    /// @param os Output stream to write to.
    /// @param packet ScenePacket to print.
    /// @return Reference to the output stream.
    friend std::ostream & operator<<( std::ostream & os, const ScenePacket & packet )
    {
        os << packet.to_json();
        return os;
    }
};
} // namespace robot::src::detail::scene_packet::inline exports

namespace robot::src::inline exports::inline scene_packet
{
using namespace detail::scene_packet::exports;
}