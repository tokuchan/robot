static_assert(__cplusplus > 2020'00);
#pragma once

#include <boost/iterator/zip_iterator.hpp>
#include "sparse_set.hpp"
#include <vector>
#include <cstdint>

/// @file component.hpp
/// @brief Component storage container for entity-component system (ECS).
///
/// This file provides a template-based component storage that efficiently
/// manages associations between entity IDs and component data using a sparse set
/// and parallel dense vector for optimal cache locality and fast iteration.

namespace robot::src::detail::component ::inline exports
{

    /// @class Component
    /// @brief Efficient component storage that associates typed data with entity IDs.
    ///
    /// The Component class implements a data-oriented design pattern where component data
    /// is stored densely alongside entity IDs using a SparseSet for O(1) lookup and a
    /// parallel std::vector for contiguous data storage. This design provides:
    ///
    /// - **O(1) insertion**: Add a component to an entity in constant time
    /// - **O(1) removal**: Remove a component from an entity in constant time
    /// - **O(1) lookup**: Query if an entity has a component or access its data in constant time
    /// - **Cache-friendly iteration**: All component data is stored contiguously, enabling
    ///   efficient cache-line utilization for bulk operations
    /// - **Type safety**: Template parameter T ensures compile-time type checking
    ///
    /// The component maintains two parallel data structures:
    /// - A SparseSet for entity-to-index mapping
    /// - A std::vector<T> for actual component data storage
    ///
    /// @tparam T The component data type. Must be copy-constructible or move-constructible.
    /// @tparam MaxEntities The maximum number of entities that can have this component
    ///                     (default: 1000). This determines the size of the sparse array.
    ///
    /// @par Example usage:
    /// @code
    /// struct Position { float x, y, z; };
    ///
    /// Component<Position, 5000> positions;
    ///
    /// // Insert component for entities
    /// positions.insert(entity1, Position{1.0f, 2.0f, 3.0f});
    /// positions.insert(entity2, Position{4.0f, 5.0f, 6.0f});
    ///
    /// // Check if entity has component
    /// if (positions.contains(entity1)) {
    ///     auto& pos = positions[entity1];
    ///     std::cout << pos.x << std::endl;
    /// }
    ///
    /// // Iterate over all (entity, component) pairs
    /// for (auto [entityId, pos] : positions) {
    ///     // Process position for entityId
    /// }
    /// @endcode
    template <typename T, unsigned int MaxEntities = 1000>
    class Component
    {
    private:
        robot::src::SparseSet<MaxEntities> entities; ///< Sparse set mapping entity IDs to data indices
        std::vector<T> data;                         ///< Dense array of component data

    public:
        using value_type = T;              ///< The component data type
        using size_type = std::size_t;     ///< Unsigned integer type for sizes and indices
        using reference = T &;             ///< Reference to component data
        using const_reference = const T &; ///< Const reference to component data

        /// @brief Default constructs an empty component storage.
        ///
        /// Initializes the component storage with no entities or data.
        /// The sparse set and data vector are ready for use.
        Component() = default;

        /// @brief Inserts an entity with associated component data (copy version).
        ///
        /// Adds a new entity to the component storage with a copy of the provided data.
        /// If the entity is already present, it will be added again (no deduplication).
        /// New component data is appended to the dense vector.
        ///
        /// @param entity The entity ID to insert.
        /// @param value The component data to associate with the entity (copied).
        ///
        /// @note Time complexity: O(1) amortized
        /// @see insert(std::size_t, T&&) for the move version
        /// @see emplace(std::size_t, Args&&...) for in-place construction
        void insert(std::size_t entity, const T &value)
        {
            entities.insert(entity);
            data.push_back(value);
        }

        /// @brief Inserts an entity with associated component data (move version).
        ///
        /// Adds a new entity to the component storage by moving the provided data.
        /// This is more efficient than the copy version when the data is a temporary
        /// or when you no longer need the original value. If the entity is already present,
        /// it will be added again (no deduplication).
        ///
        /// @param entity The entity ID to insert.
        /// @param value The component data to associate with the entity (moved).
        ///
        /// @note Time complexity: O(1) amortized
        /// @see insert(std::size_t, const T&) for the copy version
        void insert(std::size_t entity, T &&value)
        {
            entities.insert(entity);
            data.push_back(std::move(value));
        }

        /// @brief In-place constructs component data for an entity.
        ///
        /// Constructs component data directly in the dense vector without requiring a
        /// temporary object. This is the most efficient method when component construction
        /// is non-trivial. Arguments are perfectly forwarded to the component constructor.
        ///
        /// @tparam Args Parameter types for the component constructor.
        /// @param entity The entity ID to insert.
        /// @param args Arguments to forward to the component's constructor.
        ///
        /// @note Time complexity: O(1) amortized
        /// @see insert(std::size_t, const T&) for copy insertion
        /// @see insert(std::size_t, T&&) for move insertion
        template <typename... Args>
        void emplace(std::size_t entity, Args &&...args)
        {
            entities.insert(entity);
            data.emplace_back(std::forward<Args>(args)...);
        }

        /// @brief Removes an entity and its associated component data.
        ///
        /// Erases the component for the given entity. Uses the "swap and pop" technique
        /// to maintain dense array contiguity in O(1) time. If the entity is not present,
        /// the operation has no effect (idempotent).
        ///
        /// @param entity The entity ID to remove.
        ///
        /// @note Time complexity: O(1)
        /// @warning The order of remaining components may change due to the swap operation.
        /// @see contains() to check if an entity has a component before erasing
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

        /// @brief Checks whether an entity has this component.
        ///
        /// Performs a fast O(1) membership test to determine if the given entity
        /// has a component in this storage.
        ///
        /// @param entity The entity ID to check.
        /// @return true if the entity has a component, false otherwise.
        ///
        /// @note Time complexity: O(1)
        /// @see operator[] for direct access when presence is known
        [[nodiscard]] bool contains(std::size_t entity) const
        {
            return entities.contains(entity);
        }

        /// @brief Retrieves the component data for an entity.
        ///
        /// Provides mutable access to the component data associated with the given entity.
        /// Use contains() to verify the entity has a component before calling this method.
        ///
        /// @param entity The entity ID to retrieve the component for.
        /// @return A mutable reference to the component data.
        ///
        /// @throw std::out_of_range if entity is not present in this component storage.
        /// @note Time complexity: O(1)
        /// @see operator[] for direct access using subscript notation
        /// @see contains() to check if an entity has a component
        [[nodiscard]] reference get(std::size_t entity)
        {
            return data[entities.indexFor(entity)];
        }

        /// @brief Retrieves the component data for an entity (const version).
        ///
        /// Provides const access to the component data associated with the given entity.
        /// Use contains() to verify the entity has a component before calling this method.
        ///
        /// @param entity The entity ID to retrieve the component for.
        /// @return A const reference to the component data.
        ///
        /// @throw std::out_of_range if entity is not present in this component storage.
        /// @note Time complexity: O(1)
        /// @see operator[] for direct access using subscript notation
        /// @see contains() to check if an entity has a component
        [[nodiscard]] const_reference get(std::size_t entity) const
        {
            return data[entities.indexFor(entity)];
        }

        /// @brief Accesses component data by entity ID using subscript notation.
        ///
        /// Provides convenient mutable access to component data using the subscript operator.
        /// This is equivalent to calling get(entity). Use contains() to verify the entity
        /// has a component before calling this method.
        ///
        /// @param entity The entity ID to access the component for.
        /// @return A mutable reference to the component data.
        ///
        /// @throw std::out_of_range if entity is not present in this component storage.
        /// @note Time complexity: O(1)
        /// @see get() for the semantically equivalent method
        /// @see contains() to check if an entity has a component
        [[nodiscard]] reference operator[](std::size_t entity)
        {
            return get(entity);
        }

        /// @brief Accesses component data by entity ID using subscript notation (const version).
        ///
        /// Provides convenient const access to component data using the subscript operator.
        /// This is equivalent to calling get(entity). Use contains() to verify the entity
        /// has a component before calling this method.
        ///
        /// @param entity The entity ID to access the component for.
        /// @return A const reference to the component data.
        ///
        /// @throw std::out_of_range if entity is not present in this component storage.
        /// @note Time complexity: O(1)
        /// @see get() for the semantically equivalent method
        /// @see contains() to check if an entity has a component
        [[nodiscard]] const_reference operator[](std::size_t entity) const
        {
            return get(entity);
        }

        /// @brief Returns the number of entities that have this component.
        ///
        /// @return The count of entities with active component data.
        /// @note Time complexity: O(1)
        /// @see empty() to check if storage contains no components
        [[nodiscard]] size_type size() const noexcept
        {
            return entities.size();
        }

        /// @brief Checks whether the component storage contains any data.
        ///
        /// @return true if no entities have this component, false otherwise.
        /// @note Time complexity: O(1)
        /// @see size() for the count of components stored
        [[nodiscard]] bool empty() const noexcept
        {
            return entities.empty();
        }

        /// @brief Removes all entities and their component data.
        ///
        /// Clears both the sparse set and the data vector, leaving the component storage empty.
        /// All entity-component associations are destroyed.
        ///
        /// @post empty() == true && size() == 0
        /// @note Time complexity: O(n) where n is the number of entities
        /// @see erase() to remove a single entity's component
        void clear()
        {
            entities.clear();
            data.clear();
        }

        /// @brief Returns a range of entity IDs in this component storage.
        ///
        /// Provides a std::ranges-compatible view over all entity IDs that have
        /// this component. Useful for filtering, transforming, or composing with
        /// other range algorithms.
        ///
        /// @return A range view of entity IDs.
        /// @note Time complexity: O(1) (lazy evaluation)
        /// @see begin() for iteration over (entity, data) pairs
        ///
        /// @par Example:
        /// @code
        /// auto active = positions.entities()
        ///     | std::views::filter([](auto id) { return id > 100; });
        /// @endcode
        [[nodiscard]] auto entities_view() const
        {
            return entities.entities();
        }

        /// @brief Returns an iterator to the beginning of (entity, data) pairs.
        ///
        /// Provides mutable iteration over all entity-component pairs stored in this component.
        /// The iterator dereferences to a tuple of (entity_id, component_reference).
        /// Used in range-based for loops to iterate over all entities with this component.
        ///
        /// @return A zip iterator over (entity, data) pairs.
        /// @note Time complexity: O(1)
        /// @note The order of iteration is determined by insertion/deletion history.
        /// @see end() for the terminating iterator
        /// @see cbegin() for const iteration
        ///
        /// @par Example:
        /// @code
        /// for (auto [entityId, position] : components) {
        ///     // Process entity entityId with its position component
        /// }
        /// @endcode
        [[nodiscard]] auto begin()
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.begin(), data.begin()));
        }
        /// @brief Returns an iterator to the beginning of (entity, data) pairs (const version).
        ///
        /// Provides const iteration over all entity-component pairs.
        /// The iterator dereferences to a tuple of (entity_id, const_component_reference).
        ///
        /// @return A const zip iterator over (entity, data) pairs.
        /// @note Time complexity: O(1)
        /// @see cbegin() for const_iterator semantics
        [[nodiscard]] auto begin() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.begin(), data.begin()));
        }
        /// @brief Returns a const iterator to the beginning of (entity, data) pairs.
        ///
        /// Provides const iteration over all entity-component pairs with explicit
        /// const semantics. The iterator dereferences to a tuple of
        /// (entity_id, const_component_reference).
        ///
        /// @return A const zip iterator over (entity, data) pairs.
        /// @note Time complexity: O(1)
        /// @see begin() for mutable iteration
        /// @see cend() for the corresponding end iterator
        [[nodiscard]] auto cbegin() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.cbegin(), data.cbegin()));
        }

        /// @brief Returns an iterator to the end of (entity, data) pairs.
        ///
        /// The returned iterator is a sentinel value and should not be dereferenced.
        /// Used as the terminating condition in loops over entity-component pairs.
        ///
        /// @return A zip iterator pointing one past the last (entity, data) pair.
        /// @note Time complexity: O(1)
        /// @see begin() for the starting iterator
        /// @see cend() for const iteration
        [[nodiscard]] auto end()
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.end(), data.end()));
        }
        /// @brief Returns an iterator to the end of (entity, data) pairs (const version).
        ///
        /// The returned iterator is a sentinel value and should not be dereferenced.
        ///
        /// @return A const zip iterator pointing one past the last (entity, data) pair.
        /// @note Time complexity: O(1)
        /// @see begin() for starting iteration
        [[nodiscard]] auto end() const
        {
            return boost::make_zip_iterator(boost::make_tuple(entities.end(), data.end()));
        }
        /// @brief Returns a const iterator to the end of (entity, data) pairs.
        ///
        /// The returned iterator is a sentinel value and should not be dereferenced.
        /// Used as the terminating condition in const loops over entity-component pairs.
        ///
        /// @return A const zip iterator pointing one past the last (entity, data) pair.
        /// @note Time complexity: O(1)
        /// @see cbegin() for the starting iterator
        /// @see end() for mutable iteration
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