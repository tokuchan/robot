static_assert( __cplusplus > 2020'00 );

#include <catch2/catch_test_macros.hpp>

#include "components.hpp"

using namespace robot::src::exports::components;

struct Position
{
    float x, y;
};

struct Velocity
{
    float dx, dy;
};

struct Health
{
    int hp;
};

SCENARIO( "Components storage manages multiple component types" )
{
    GIVEN( "A Components collection for Position and Velocity" )
    {
        Components< Position, Velocity > components;

        WHEN( "accessing Position storage" )
        {
            auto & pos_storage = components.get< Position >();
            THEN( "Position storage is accessible" )
            {
                REQUIRE( pos_storage.size() == 0 );
            }
        }

        WHEN( "accessing Velocity storage" )
        {
            auto & vel_storage = components.get< Velocity >();
            THEN( "Velocity storage is accessible" )
            {
                REQUIRE( vel_storage.size() == 0 );
            }
        }
    }

    GIVEN( "A Components collection with three component types" )
    {
        Components< Position, Velocity, Health > components;

        WHEN( "accessing each component type via get()" )
        {
            auto & pos = components.get< Position >();
            auto & vel = components.get< Velocity >();
            auto & health = components.get< Health >();

            THEN( "all storages are independent and accessible" )
            {
                REQUIRE( pos.size() == 0 );
                REQUIRE( vel.size() == 0 );
                REQUIRE( health.size() == 0 );
            }
        }
    }

    GIVEN( "A const Components collection" )
    {
        const Components< Position, Velocity > components;

        WHEN( "accessing storages via const get()" )
        {
            const auto & pos_storage = components.get< Position >();
            const auto & vel_storage = components.get< Velocity >();

            THEN( "const accessors return const references" )
            {
                REQUIRE( pos_storage.size() == 0 );
                REQUIRE( vel_storage.size() == 0 );
            }
        }
    }
}