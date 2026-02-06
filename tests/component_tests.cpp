#include <catch2/catch_test_macros.hpp>
#include "component.hpp"
#include <string>

struct Position
{
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x_, float y_) : x(x_), y(y_) {}

    bool operator==(const Position &) const = default;
};

struct Health
{
    int current = 100;
    int maximum = 100;

    Health() = default;
    Health(int cur, int max) : current(cur), maximum(max) {}
};

namespace c = robot::src::exports::component;

SCENARIO("Component storage and entity management", "[component]")
{
    GIVEN("A newly created Component storage for Position")
    {
        c::Component<Position> components;

        THEN("it should be empty")
        {
            REQUIRE(components.empty());
            REQUIRE(components.size() == 0);
        }

        WHEN("an entity with a component is inserted")
        {
            components.insert(42, Position{10.0f, 20.0f});

            THEN("the component should be stored and accessible")
            {
                REQUIRE(components.contains(42));
                REQUIRE(components.size() == 1);
                REQUIRE(!components.empty());

                auto &pos = components.get(42);
                REQUIRE(pos.x == 10.0f);
                REQUIRE(pos.y == 20.0f);
            }
        }

        WHEN("multiple entities are inserted")
        {
            components.insert(1, Position{1.0f, 2.0f});
            components.insert(2, Position{3.0f, 4.0f});
            components.insert(3, Position{5.0f, 6.0f});

            THEN("all components should be accessible")
            {
                REQUIRE(components.size() == 3);
                REQUIRE(components.contains(1));
                REQUIRE(components.contains(2));
                REQUIRE(components.contains(3));

                REQUIRE(components[1] == Position{1.0f, 2.0f});
                REQUIRE(components[2] == Position{3.0f, 4.0f});
                REQUIRE(components[3] == Position{5.0f, 6.0f});
            }
        }
    }

    GIVEN("A Component storage with existing entities")
    {
        c::Component<Position> components;
        components.insert(10, Position{100.0f, 200.0f});
        components.insert(20, Position{300.0f, 400.0f});
        components.insert(30, Position{500.0f, 600.0f});

        WHEN("an entity is removed")
        {
            components.erase(20);

            THEN("the entity should no longer be present")
            {
                REQUIRE(!components.contains(20));
                REQUIRE(components.size() == 2);
                REQUIRE(components.contains(10));
                REQUIRE(components.contains(30));
            }
        }

        WHEN("a component is accessed and modified")
        {
            components[10].x = 999.0f;

            THEN("the modification should persist")
            {
                REQUIRE(components.get(10).x == 999.0f);
                REQUIRE(components.get(10).y == 200.0f);
            }
        }

        WHEN("all components are cleared")
        {
            components.clear();

            THEN("the storage should be empty")
            {
                REQUIRE(components.empty());
                REQUIRE(components.size() == 0);
                REQUIRE(!components.contains(10));
                REQUIRE(!components.contains(20));
                REQUIRE(!components.contains(30));
            }
        }
    }

    GIVEN("A Component storage using emplace construction")
    {
        c::Component<Health> components;

        WHEN("components are emplaced with constructor arguments")
        {
            components.emplace(100, 80, 100);
            components.emplace(101, 50, 75);

            THEN("components should be constructed in-place")
            {
                REQUIRE(components.contains(100));
                REQUIRE(components.contains(101));

                REQUIRE(components[100].current == 80);
                REQUIRE(components[100].maximum == 100);

                REQUIRE(components[101].current == 50);
                REQUIRE(components[101].maximum == 75);
            }
        }
    }

    GIVEN("A Component storage with move semantics")
    {
        c::Component<std::string> components;

        WHEN("components are inserted via move")
        {
            std::string name = "EntityOne";
            components.insert(1, std::move(name));

            THEN("the component should be moved into storage")
            {
                REQUIRE(components.contains(1));
                REQUIRE(components[1] == "EntityOne");
            }
        }
    }
}