static_assert( __cplusplus > 2020'00 );
#pragma once

namespace robot::src::detail::systems::inline exports
{
inline void handlePlayerInput( EntityStore & store )
{
    auto & inputs = store.get< PlayerInput >();
    for( auto [ entity, input ] : inputs )
    {
        // Process player input for the entity
        // For example, you could update the entity's velocity based on input
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
        }
    }
}
} // namespace robot::src::detail::systems::inline exports

namespace robot::src::inline exports::inline systems
{
using namespace detail::systems::exports;
}