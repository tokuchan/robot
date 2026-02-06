static_assert(__cplusplus > 2020'00);
#pragma once

#include <ranges>
#include "component.hpp"

namespace robot::src::detail::components ::inline exports
{
    /// @brief A collection of component storages for multiple component types.
    /// @tparam ...ComponentTypes
    template <typename... ComponentTypes>
    struct Components
    {
        /// @brief Tuple of component storages, one for each component type.
        std::tuple<Component<ComponentTypes>...> storages;

        /// @brief  Get the component storage for a specific component type.
        /// @tparam T
        /// @return Reference to the component storage for type T.
        template <typename T>
        auto &get()
        {
            return std::get<Component<T>>(storages);
        }

        /// @brief  Get the component storage for a specific component type.
        /// @tparam T
        /// @return Reference to the component storage for type T.
        template <typename T>
        const auto &get() const
        {
            return std::get<Component<T>>(storages);
        }
    };
}

namespace robot::src::inline exports::inline components
{
    using namespace detail::components::exports;
}