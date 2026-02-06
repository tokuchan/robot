#include <cassert>
#include <vector>
#include <stdexcept>

namespace robot::src::sparse_set::detail
{
    inline namespace exports
    {
        template <int EntityCount = 1000>
        class SparseSet
        {
        public:
            using ContainerType = std::vector<std::size_t>;
            using IndexContainerType = std::vector<std::size_t>;

        private:
            IndexContainerType dataIndex;
            ContainerType data;

        public:
            SparseSet() : dataIndex(EntityCount, static_cast<std::size_t>(-1)) {}
            SparseSet(const SparseSet &) = default;
            SparseSet(SparseSet &&) = default;
            SparseSet &operator=(const SparseSet &) = default;
            SparseSet &operator=(SparseSet &&) = default;

            void swap(SparseSet &other) noexcept
            {
                assert(dataIndex.size() == other.dataIndex.size());
                assert(data.size() == other.data.size());
                assert(this != &other);

                using std::swap;
                swap(dataIndex, other.dataIndex);
                data.swap(other.data);
            }

            void clear() noexcept
            {
                dataIndex.assign(EntityCount, static_cast<std::size_t>(-1));
                data.clear();
            }

            void reserve(std::size_t new_cap)
            {
                assert(new_cap >= data.size());
                assert(new_cap <= dataIndex.size());

                data.reserve(new_cap);
            }

            std::size_t size() const noexcept
            {
                return data.size();
            }

            bool empty() const noexcept
            {
                return data.empty();
            }

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

            auto begin() noexcept
            {
                return data.begin();
            }

            auto end() noexcept
            {
                return data.end();
            }

            auto cbegin() const noexcept
            {
                return data.cbegin();
            }

            auto cend() const noexcept
            {
                return data.cend();
            }
        };
    }
}

namespace robot::inline sparse_set ::inline exports
{
    using namespace src::sparse_set::detail::exports;
}