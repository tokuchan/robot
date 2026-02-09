static_assert( __cplusplus > 2020'00 );
#pragma once

#include <cmath>

namespace robot::src::detail::systems::inline exports
{
// World bounds for wrapping geometry
constexpr float WORLD_MIN_X = -120.0f;
constexpr float WORLD_MAX_X = 120.0f;
constexpr float WORLD_MIN_Y = -120.0f;
constexpr float WORLD_MAX_Y = 120.0f;
constexpr float WORLD_WIDTH = WORLD_MAX_X - WORLD_MIN_X;
constexpr float WORLD_HEIGHT = WORLD_MAX_Y - WORLD_MIN_Y;

inline float wrapCoordinate( float value, float min_val, float max_val )
{
    float range = max_val - min_val;
    if( range <= 0.0f )
        return value;
    while( value < min_val )
        value += range;
    while( value >= max_val )
        value -= range;
    return value;
}

inline void handlePlayerInput( EntityStore & store )
{
    auto & inputs = store.get< PlayerInput >();
    auto & velocities = store.get< Velocity >();

    for( auto [ entity, input ] : inputs )
    {
        // Apply player input to entity's velocity
        if( velocities.contains( entity ) )
        {
            velocities[ entity ] = Velocity{ input.x, input.y };
            //std::cout << "Applied input to entity " << entity << ": velocity = (" << input.x << ", " << input.y << ")"
            //          << std::endl;
        }
        else
        {
            std::cerr << "Entity " << entity << " has PlayerInput but no Velocity component!" << std::endl;
        }
    }
}

inline void handleCollisions( EntityStore & store )
{
    auto & polygons = store.get< Polygon >();
    auto & velocities = store.get< Velocity >();
    auto & hit_counters = store.get< HitCounter >();

    // Use AABB checks to find potential collisions as the broad phase
    for( auto [ entity_a, poly_a ] : polygons )
    {
        for( auto [ entity_b, poly_b ] : polygons )
        {
            if( entity_a >= entity_b )
                continue; // Avoid duplicate checks and self-collision

            if( poly_a.may_intersect( { poly_b } ) )
            {
                // Perform narrow phase collision check using SAT
                if( poly_a.intersects( poly_b ) )
                {
                    // We only need to handle the robot's collision, which is the entity with the HitCounter component.
                    if( hit_counters.contains( entity_a ) )
                    {
                        hit_counters[ entity_a ].hits += 1;
                        // zero out the velocity to stop movement after a hit
                        if( velocities.contains( entity_a ) )
                        {
                            velocities[ entity_a ] = { 0.0f, 0.0f };
                        }
                    }
                }
            }
        }
    }
}

inline void updatePositions( EntityStore & store )
{
    auto & velocities = store.get< Velocity >();
    auto & positions = store.get< Position >();

    for( auto [ entity, velocity ] : velocities )
    {
        if( positions.contains( entity ) )
        {
            auto & position = positions[ entity ];
            position.x += velocity.x;
            position.y += velocity.y;
            // Wrap coordinates to keep them within world bounds
            position.x = wrapCoordinate( position.x, WORLD_MIN_X, WORLD_MAX_X );
            position.y = wrapCoordinate( position.y, WORLD_MIN_Y, WORLD_MAX_Y );
        }
    }
}
} // namespace robot::src::detail::systems::inline exports

namespace robot::src::inline exports::inline systems
{
using namespace detail::systems::exports;
}