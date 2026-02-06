static_assert(__cplusplus > 2020'00);
#pragma once

#include <cassert>
#include <vector>
#include <stdexcept>

/// @file sparse_set.hpp
/// @brief Sparse set container implementation for efficient entity storage and lookup.
///
/// This file provides a sparse set data structure optimized for fast O(1) insertion,
/// deletion, and lookup operations with contiguous storage of active elements.

namespace robot::src::detail::sparse_set ::inline exports
{
    /// @class SparseSet
    /// @brief A sparse set container for managing a dense collection of entity identifiers.
    ///
    /// A sparse set is a data structure that provides O(1) insert, erase, and lookup
    /// operations while maintaining all active elements in contiguous storage. This makes
    /// it ideal for entity management systems where you need fast membership testing and
    /// efficient iteration over active entities.
    ///
    /// The sparse set uses two arrays:
    /// - A sparse array (dataIndex) that maps entity IDs to their positions in the dense array
    /// - A dense array (data) that stores the actual entity IDs in contiguous memory
    ///
    /// @tparam EntityCount The maximum number of entities that can be stored (default: 1000).
    ///                     This determines the size of the sparse array.
    ///
    /// @par Example usage:
    /// @code
    /// SparseSet<5000> entities;
    /// entities.insert(42);
    /// entities.insert(100);
    ///
    /// if (entities.contains(42)) {
    ///     // Process entity 42
    /// }
    ///
    /// for (auto entityId : entities) {
    ///     // Iterate over all active entities
    /// }
    /// @endcode
    template <unsigned int EntityCount = 1000>
    class SparseSet
    {
    public:
        using ContainerType = std::vector<std::size_t>;      ///< Type alias for the dense array
        using IndexContainerType = std::vector<std::size_t>; ///< Type alias for the sparse array

    private:
        IndexContainerType dataIndex; ///< Sparse array mapping entity IDs to dense array indices
        ContainerType data;           ///< Dense array storing active entity IDs

    public:
        /// @brief Constructs an empty sparse set.
        ///
        /// Initializes the sparse array with size EntityCount, with all entries marked as invalid.
        /// The dense array starts empty and grows as entities are inserted.
        SparseSet() : dataIndex(EntityCount, static_cast<std::size_t>(-1)) {}

        /// @brief Copy constructor (defaulted).
        SparseSet(const SparseSet &) = default;

        /// @brief Move constructor (defaulted).
        SparseSet(SparseSet &&) = default;

        /// @brief Copy assignment operator (defaulted).
        SparseSet &operator=(const SparseSet &) = default;

        /// @brief Move assignment operator (defaulted).
        SparseSet &operator=(SparseSet &&) = default;

        /// @brief Swaps the contents of this sparse set with another.
        ///
        /// Exchanges the internal sparse and dense arrays with another sparse set.
        /// This is a no-throw operation.
        ///
        /// @param other The sparse set to swap with.
        ///
        /// @pre Both sparse sets must have the same EntityCount template parameter.
        /// @post This sparse set's contents are swapped with other's contents.
        ///
        /// @note Time complexity: O(1)
        void swap(SparseSet &other) noexcept
        {
            assert(dataIndex.size() == other.dataIndex.size());
            assert(data.size() == other.data.size());
            assert(this != &other);

            using std::swap;
            swap(dataIndex, other.dataIndex);
            data.swap(other.data);
        }

        /// @brief Removes all entities from the sparse set.
        ///
        /// Clears both the dense array and resets the sparse array to its initial state.
        /// After this call, the sparse set will be empty.
        ///
        /// @post size() == 0 && empty() == true
        ///
        /// @note Time complexity: O(EntityCount)
        void clear() noexcept
        {
            dataIndex.assign(EntityCount, static_cast<std::size_t>(-1));
            data.clear();
        }

        /// @brief Pre-allocates space in the dense array for entities.
        ///
        /// Reserves capacity in the dense array to avoid reallocations during subsequent
        /// insert operations. The sparse array size remains fixed at EntityCount.
        ///
        /// @param new_cap The new capacity to reserve. Must be >= current size and <= EntityCount.
        ///
        /// @pre new_cap >= size() && new_cap <= EntityCount
        ///
        /// @note Time complexity: O(1) amortized
        void reserve(std::size_t new_cap)
        {
            assert(new_cap >= data.size());
            assert(new_cap <= dataIndex.size());

            data.reserve(new_cap);
        }

        /// @brief Returns the number of entities currently in the sparse set.
        ///
        /// @return The count of active entities.
        ///
        /// @note Time complexity: O(1)
        std::size_t size() const noexcept
        {
            return data.size();
        }

        /// @brief Checks whether the sparse set is empty.
        ///
        /// @return true if no entities are stored, false otherwise.
        ///
        /// @note Time complexity: O(1)
        bool empty() const noexcept
        {
            return data.empty();
        }

        /// @brief Inserts an entity into the sparse set.
        ///
        /// Adds an entity identifiedby the given value. If the entity is already present,
        /// the operation has no effect (idempotent). New entities are appended to the dense array.
        ///
        /// @param value The entity ID to insert. Must be in the range [0, EntityCount).
        ///
        /// @throw std::out_of_range if value >= EntityCount.
        ///
        /// @post contains(value) == true
        ///
        /// @note Time complexity: O(1) amortized
        /// @note If the entity already exists, this is a no-op with O(1) lookup time.
        void insert(std::size_t value)
        {
            assert(value < EntityCount);
            assert(dataIndex.size() == EntityCount);
            assert(data.size() <= EntityCount);
            assert(this != nullptr);

            if (dataIndex.size() <= value)
            {
                throw std::out_of_range("SparseSet::insert: value out of range");
            }

            if (dataIndex[value] != static_cast<std::size_t>(-1))
            {
                return; // Already present
            }

            data.push_back(value);
            dataIndex[value] = data.size() - 1;
        }

        /// @brief Removes an entity from the sparse set.
        ///
        /// Erases the entity identified by the given value. If the entity is not present,
        /// the operation has no effect (idempotent). Uses the "swap and pop" technique
        /// to maintain dense array integrity in O(1) time.
        ///
        /// @param value The entity ID to erase. Must be in the range [0, EntityCount).
        ///
        /// @throw std::out_of_range if value >= EntityCount.
        ///
        /// @post contains(value) == false
        ///
        /// @note Time complexity: O(1) amortized
        /// @note If the entity does not exist, this is a no-op with O(1) lookup time.
        /// @warning The iteration order may change after this operation due to the swap.
        void erase(std::size_t value)
        {
            assert(value < EntityCount);
            assert(dataIndex.size() == EntityCount);
            assert(this != nullptr);

            if (dataIndex.size() <= value)
            {
                throw std::out_of_range("SparseSet::erase: value out of range");
            }

            std::size_t index = dataIndex[value];
            if (index == static_cast<std::size_t>(-1))
            {
                return; // Not present
            }

            std::size_t lastValue = data.back();
            data[index] = lastValue;
            dataIndex[lastValue] = index;

            data.pop_back();
            dataIndex[value] = static_cast<std::size_t>(-1);
        }

        /// @brief Checks whether an entity is in the sparse set.
        ///
        /// Performs a fast membership test on the given entity ID.
        ///
        /// @param value The entity ID to check. If >= EntityCount, always returns false.
        ///
        /// @return true if the entity is in the set, false otherwise.
        ///
        /// @note Time complexity: O(1)
        bool contains(std::size_t value) const
        {
            assert(dataIndex.size() == EntityCount);
            assert(this != nullptr);

            if (dataIndex.size() <= value)
            {
                return false;
            }
            return dataIndex[value] != static_cast<std::size_t>(-1);
        }

        /// @brief Gets the index of an entity in the dense array.
        ///
        /// Maps an entity ID to its position in the dense array. Useful for random access.
        ///
        /// @param entityId The entity ID to look up.
        ///
        /// @return The zero-based index in the dense array where this entity is stored.
        ///
        /// @throw std::out_of_range if entityId >= EntityCount.
        ///
        /// @pre contains(entityId) == true (if precondition is violated, result is undefined)
        ///
        /// @note Time complexity: O(1)
        std::size_t indexFor(std::size_t entityId) const
        {
            assert(dataIndex.size() == EntityCount);
            assert(this != nullptr);

            if (entityId >= dataIndex.size())
            {
                throw std::out_of_range("SparseSet::indexFor: index out of range");
            }
            return dataIndex[entityId];
        }

        /// @brief Gets the entity ID at a given index in the dense array.
        ///
        /// The inverse of indexFor(). Maps from a position in the dense array to the entity ID.
        ///
        /// @param index The zero-based position in the dense array.
        ///
        /// @return The entity ID stored at the given index.
        ///
        /// @throw std::out_of_range if index >= size().
        ///
        /// @pre index < size()
        ///
        /// @note Time complexity: O(1)
        std::size_t idFor(std::size_t index) const
        {
            assert(dataIndex.size() == EntityCount);
            assert(this != nullptr);

            if (index >= data.size())
            {
                throw std::out_of_range("SparseSet::idFor: index out of range");
            }
            return data[index];
        }

        /// @brief Returns a range view over all entity IDs in the sparse set.
        ///
        /// Provides a std::ranges-compatible view of the dense array for use with
        /// range algorithms and range-based for loops with pipe operators.
        ///
        /// @return A range view over all active entity IDs.
        ///
        /// @note Time complexity: O(1)
        ///
        /// @see begin(), end() for iterator access
        auto entities() noexcept
        {
            return data;
        }

        /// @brief Returns a const range view over all entity IDs in the sparse set.
        ///
        /// Provides a const std::ranges-compatible view of the dense array for use with
        /// range algorithms when the sparse set should not be modified.
        ///
        /// @return A const range view over all active entity IDs.
        ///
        /// @note Time complexity: O(1)
        auto entities() const noexcept
        {
            return data;
        }
        /// @brief Returns an iterator to the beginning of the dense array.
        ///
        /// Allows range-based iteration over all active entities in the sparse set.
        /// The order of iteration is determined by insertion/deletion history.
        ///
        /// @return An iterator pointing to the first entity, or end() if empty.
        ///
        /// @note Time complexity: O(1)
        ///
        /// @see cbegin() for a const version
        auto begin() noexcept
        {
            return data.begin();
        }

        /// @brief Returns an iterator to the end of the dense array.
        ///
        /// The returned iterator is a sentinel value and should not be dereferenced.
        /// Use with begin() for range-based iteration.
        ///
        /// @return An iterator pointing one past the last entity.
        ///
        /// @note Time complexity: O(1)
        ///
        /// @see cend() for a const version
        auto end() noexcept
        {
            return data.end();
        }

        /// @brief Returns a const iterator to the beginning of the dense array.
        ///
        /// Allows const range-based iteration over all active entities. Useful when
        /// the sparse set should not be modified during iteration.
        ///
        /// @return A const iterator pointing to the first entity, or cend() if empty.
        ///
        /// @note Time complexity: O(1)
        auto cbegin() const noexcept
        {
            return data.cbegin();
        }

        /// @brief Returns a const iterator to the end of the dense array.
        ///
        /// The returned iterator is a sentinel value and should not be dereferenced.
        /// Use with cbegin() for const range-based iteration.
        ///
        /// @return A const iterator pointing one past the last entity.
        ///
        /// @note Time complexity: O(1)
        auto cend() const noexcept
        {
            return data.cend();
        }
    };

}

namespace robot::src::inline exports::inline sparse_set
{
    using namespace detail::sparse_set::exports;
}