#include <catch2/catch_test_macros.hpp>
#include <sparse_set.hpp>
#include <algorithm>

SCENARIO("SparseSet manages entity-component associations", "[sparse_set]")
{
    GIVEN("an empty SparseSet")
    {
        robot::SparseSet set;

        WHEN("checking if it contains an entity")
        {
            THEN("it returns false")
            {
                REQUIRE_FALSE(set.contains(0));
                REQUIRE_FALSE(set.contains(42));
            }
        }

        WHEN("querying its size")
        {
            THEN("the size is zero")
            {
                REQUIRE(set.size() == 0);
                REQUIRE(set.empty());
            }
        }

        WHEN("an entity is inserted")
        {
            set.insert(5);

            THEN("the set contains that entity")
            {
                REQUIRE(set.contains(5));
                REQUIRE(set.size() == 1);
                REQUIRE_FALSE(set.empty());
            }

            AND_WHEN("the same entity is inserted again")
            {
                set.insert(5);

                THEN("the set still contains it only once")
                {
                    REQUIRE(set.contains(5));
                    REQUIRE(set.size() == 1);
                }
            }
        }

        WHEN("multiple entities are inserted")
        {
            set.insert(1);
            set.insert(10);
            set.insert(100);

            THEN("all entities are contained")
            {
                REQUIRE(set.contains(1));
                REQUIRE(set.contains(10));
                REQUIRE(set.contains(100));
                REQUIRE(set.size() == 3);
            }

            AND_WHEN("one entity is removed")
            {
                set.erase(10);

                THEN("only the removed entity is absent")
                {
                    REQUIRE(set.contains(1));
                    REQUIRE_FALSE(set.contains(10));
                    REQUIRE(set.contains(100));
                    REQUIRE(set.size() == 2);
                }
            }
        }
    }

    GIVEN("a SparseSet with several entities")
    {
        robot::SparseSet set;
        set.insert(7);
        set.insert(3);
        set.insert(15);

        WHEN("removing an entity that exists")
        {
            set.erase(3);

            THEN("the entity is no longer present")
            {
                REQUIRE_FALSE(set.contains(3));
                REQUIRE(set.contains(7));
                REQUIRE(set.contains(15));
                REQUIRE(set.size() == 2);
            }
        }

        WHEN("removing an entity that does not exist")
        {
            set.erase(99);

            THEN("the set remains unchanged")
            {
                REQUIRE(set.contains(7));
                REQUIRE(set.contains(3));
                REQUIRE(set.contains(15));
                REQUIRE(set.size() == 3);
            }
        }

        WHEN("clearing the set")
        {
            set.clear();

            THEN("the set becomes empty")
            {
                REQUIRE(set.empty());
                REQUIRE(set.size() == 0);
                REQUIRE_FALSE(set.contains(7));
                REQUIRE_FALSE(set.contains(3));
                REQUIRE_FALSE(set.contains(15));
            }
        }
    }

    GIVEN("a SparseSet supporting iteration")
    {
        robot::SparseSet set;
        set.insert(2);
        set.insert(4);
        set.insert(6);

        WHEN("iterating over entities")
        {
            std::vector<int> entities;
            for (auto entity : set)
            {
                entities.push_back(entity);
            }

            THEN("all entities are visited")
            {
                REQUIRE(entities.size() == 3);
                REQUIRE(std::find(entities.begin(), entities.end(), 2) != entities.end());
                REQUIRE(std::find(entities.begin(), entities.end(), 4) != entities.end());
                REQUIRE(std::find(entities.begin(), entities.end(), 6) != entities.end());
            }
        }
    }
}