static_assert( __cplusplus > 2020'00 );
#pragma once

#include <cmath>
#include <functional>
#include <random>
#include <string>

#include "component_types.hpp"

namespace robot::src::detail::assets::inline exports
{
// This file contains code for procedurally generating assets at runtime. In a
// real application, you would probably want to load assets from disk or a
// remote server, but for this example we'll just generate some simple assets in
// code from a given key.

void buildProceduralAssets( EntityStore & store, const std::string & key, std::size_t numAssets = 10 )
{
    // For simplicity, we'll just generate some random polygons based on the key.
    // In a real application, you could use the key to seed a more complex
    // procedural generation algorithm.

    std::hash< std::string > hasher;
    std::size_t seed = hasher( key );
    std::mt19937 rng( seed );
    std::uniform_real_distribution< float > dist( -100.0f, 100.0f );

    // First clear out any existing assets in the store
    store.get< Polygon >().clear();
    store.get< Position >().clear();

    // Next, position the robot at the center of the world
    store.get< Position >().insert( 0, Position{ 0.0f, 0.0f } );
    // Add velocity component so robot can move
    store.get< Velocity >().insert( 0, Velocity{ 0.0f, 0.0f } );
    // Add hit counter to track collisions
    store.get< HitCounter >().insert( 0, HitCounter{ 0 } );
    // Now, generate the robot's geometry as a rectangle centered on the robot's position
    store.get< Polygon >().insert(
        0,
        Polygon( { Vec2{ -10.0f, -10.0f }, Vec2{ 10.0f, -10.0f }, Vec2{ 10.0f, 10.0f }, Vec2{ -10.0f, 10.0f } } ) );

    // To give it character, we'll add two squares on top to represent eyes
    store.get< Polygon >()
        .insert( 1, Polygon( { Vec2{ -5.0f, 5.0f }, Vec2{ -3.0f, 5.0f }, Vec2{ -3.0f, 7.0f }, Vec2{ -5.0f, 7.0f } } ) );
    store.get< Polygon >()
        .insert( 2, Polygon( { Vec2{ 3.0f, 5.0f }, Vec2{ 5.0f, 5.0f }, Vec2{ 5.0f, 7.0f }, Vec2{ 3.0f, 7.0f } } ) );

    constexpr std::size_t base_entity_id = 3;

    // Now, generate some random static obstacles in the world.
    for( std::size_t i = 0; i < numAssets; ++i )
    {
        Polygon polygon;
        int numVertices = 3 + ( rng() % 5 ); // 3 to 7 vertices
        // In order for the polygons to collide properly, we need to make sure
        // they are convex and not self-intersecting. A simple way to do this is
        // to generate the vertices in a circular pattern around a center point.
        float radius = 5.0f + dist( rng ) * 0.125f; // 5 to 20 units
        for( int j = 0; j < numVertices; ++j )
        {
            float angle = ( 2.0f * 3.14159265f * j ) / numVertices;
            polygon.vertices_x.push_back( radius * std::cos( angle ) );
            polygon.vertices_y.push_back( radius * std::sin( angle ) );
        }
        std::size_t entity_id = base_entity_id + i;
        store.get< Polygon >().insert( entity_id, polygon );
        store.get< Position >().insert( entity_id, Position{ dist( rng ), dist( rng ) } );
    }

    // Next, generate some random dynamic entities in the world that will move around. For simplicity, these will just
    // be triangles that move in a random direction.
    for( std::size_t i = 0; i < numAssets; ++i )
    {
        Polygon polygon{ Vec2{ -5.0f, -5.0f }, Vec2{ 5.0f, -5.0f }, Vec2{ 0.0f, 5.0f } };
        std::size_t entity_id = base_entity_id + numAssets + i;
        store.get< Polygon >().insert( entity_id, polygon );
        store.get< Position >().insert( entity_id, Position{ dist( rng ), dist( rng ) } );
        // Don't forget to add a Velocity component so they will move in the main loop!
        store.get< Velocity >().insert( entity_id, Velocity{ dist( rng ) * 0.1f, dist( rng ) * 0.1f } );
    }
}
} // namespace robot::src::detail::assets::inline exports

namespace robot::src::inline exports::inline assets
{
using namespace detail::assets::exports;
}