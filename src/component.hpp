static_assert(__cplusplus > 2020'00);
#pragma once

#include <boost/iterator/zip_iterator.hpp>
#include "sparse_set.hpp"
#include <vector>
#include <cstdint>

namespace robot::src::detail::component ::inline exports
{

    /// @brief A component storage container that associates data of type T with entity IDs.
    /// Uses a SparseSet for efficient entity-to-index mapping and a parallel dense vector for data.
    template <typename T, unsigned int MaxEntities = 1000>
    class Component
    {
    private:
        robot::src::SparseSet<MaxEntities> entities;
        std::vector<T> data;

    public:
        using value_type = T;
        using size_type = std::size_t;
        using reference = T &;
        using const_reference = const T &;

        Component() = default;

        /// @brief Insert an entity with associated component data.
        /// @param entity The entity ID to insert.
        /// @param value The component data to associate with the entity.
        void insert(std::size_t entity, const T &value)
        {
            entities.insert(entity);
            data.push_back(value);
        }

        /// @brief Insert an entity with associated component data (move version).
        void insert(std::size_t entity, T &&value)
        {
            entities.insert(entity);
            data.push_back(std::move(value));
        }

        /// @brief Emplace construct component data for an entity.
        template <typename... Args>
        void emplace(std::size_t entity, Args &&...args)
        {
            entities.insert(entity);
            data.emplace_back(std::forward<Args>(args)...);
        }

        /// @brief Remove an entity and its associated component data.
        void erase(std::size_t entity)
        {
            if (!entities.contains(entity))
            {
                return;
            }
            size_type index = entities.indexFor(entity);
            // Swap the data at index with the last element, then pop
            if (index < data.size() - 1)
            {
                std::swap(data[index], data.back());
            }
            data.pop_back();
            entities.erase(entity);
        }

        /// @brief Check if an entity has this component.
        [[nodiscard]] bool contains(std::size_t entity) const
        {
            return entities.contains(entity);
        }

        /// @brief Get the component data for an entity.
        /// @throws std::out_of_range if entity is not present.
        [[nodiscard]] reference get(std::size_t entity)
        {
            return data[entities.indexFor(entity)];
        }

        /// @brief Get the component data for an entity (const version).
        [[nodiscard]] const_reference get(std::size_t entity) const
        {
            return data[entities.indexFor(entity)];
        }

        /// @brief Access component data by entity ID.
        [[nodiscard]] reference operator[](std::size_t entity)
        {
            return get(entity);
        }

        /// @brief Access component data by entity ID (const version).
        [[nodiscard]] const_reference operator[](std::size_t entity) const
        {
            return get(entity);
        }

        /// @brief Get the number of components stored.
        [[nodiscard]] size_type size() const noexcept
        {
            return entities.size();
        }

        /// @brief Check if the component storage is empty.
        [[nodiscard]] bool empty() const noexcept
        {
            return entities.empty();
        }

        /// @brief Remove all entities and component data.
        void clear()
        {
            entities.clear();
            data.clear();
        }

        /// @brief Get iterator to beginning of (entity, data) pairs.
        [[nodiscard]] auto begin()
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.begin(), data.begin()));
        }
        [[nodiscard]] auto begin() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.begin(), data.begin()));
        }
        [[nodiscard]] auto cbegin() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.cbegin(), data.cbegin()));
        }

        /// @brief Get iterator to end of (entity, data) pairs.
        [[nodiscard]] auto end()
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.end(), data.end()));
        }
        [[nodiscard]] auto end() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.end(), data.end()));
        }
        [[nodiscard]] auto cend() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.cend(), data.cend()));
        }
    };

}

namespace robot::src::inline exports::inline component
{
    using namespace detail::component::exports;
}