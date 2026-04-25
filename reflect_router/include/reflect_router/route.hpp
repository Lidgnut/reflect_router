#pragma once

#include "reflect_router/meta.hpp"

namespace reflect_router
{

    namespace detail
    {
        consteval bool is_route_allowed(char val)
        {
            return std::ranges::contains("/:=_abcdefghijklmnopqrstuvwxyyz", val);
        }

        consteval bool is_route_allowed(std::string_view route)
        {
            if (route[0] != '/')
                return false;
            for (auto val : route)
                if (!is_route_allowed(val))
                    return false;
            return true;
        }

        template <size_t N>
        struct FixedString
        {
            char data[N]{};
            size_t size{N - 1};

            constexpr FixedString(const char (&str)[N])
            {
                std::copy_n(str, N, data);
            }
        // Der Konstruktor, der string_view frisst
            consteval FixedString(std::string_view str) 
            : size(str.size()){
                // 1. Validierung (funktioniert zur Compile-Zeit als Error!)
                if (str.size() >= N) {
                    throw "String too big for FixedString buffer!";
                }

                // 2. Daten kopieren
                for (std::size_t i = 0; i < str.size(); ++i) {
                    data[i] = str[i];
                }
            }

            constexpr operator std::string_view() const
            {
                return {data, size};
            }
        };

        struct RouteParam
        {
            std::string_view name;
            std::string_view type;
            uint64_t index;
        };

    } // namespace detail

    template <reflect_router::detail::FixedString fs>
    struct Route
    {
        static constexpr std::string_view path = fs;
        
        consteval auto path_parts() const
        {
            constexpr auto num_params = [&]()
            {
                size_t num{0};
                for (auto val : path)
                    num += val == '/';
                return num;
            }();
            std::array<std::string_view, num_params> out;
            uint64_t i{0};
            for (uint64_t found = path.find('/'); found != std::string_view::npos; ++i)
            {
                uint64_t next_found = path.find('/', found + 1);
                out[i] = path.substr(found, next_found - found).substr(1);
                found = next_found;
            }
            return out;
        }
        

        consteval auto path_params() const
        {
            constexpr auto num_path_params = [this]()
            {
                uint64_t num{0};
                for (const auto path_part : path_parts())
                {
                    if (path_part[0] == ':')
                        ++num;
                }
                return num;
            }();
            std::array<detail::RouteParam, num_path_params> out;
            size_t i{0};
            constexpr auto curr_path_parts = path_parts();
            for (size_t index{0}; index != curr_path_parts.size(); ++index)
            {
                if (curr_path_parts[index][0] == ':')
                {
                    auto column_pos = curr_path_parts[index].find(':', 1);
                    out[i].name = curr_path_parts[index].substr(1, column_pos - 1);
                    out[i].type = curr_path_parts[index].substr(column_pos + 2);
                    out[i].index = index;
                    ++i;
                }
            }
            return out;
        }
        // consteval auto path_params() const
        // {
        //     std::vector<detail::RouteParam> out;
        //     constexpr auto curr_path_parts = path_parts();
        //     for (size_t index{0}; index != curr_path_parts.size(); ++index)
        //     {
        //         if (curr_path_parts[index][0] == ':')
        //         {
        //             auto column_pos = curr_path_parts[index].find(':', 1);
        //             out.emplace_back(curr_path_parts[index].substr(1, column_pos - 1), curr_path_parts[index].substr(column_pos + 2), index);
        //             // out[i].name = curr_path_parts[index].substr(1, column_pos - 1);
        //             // out[i].type = curr_path_parts[index].substr(column_pos + 2);
        //             // out[i].index = index;
        //         }
        //     }
        //     return out;
        // }
    };

    namespace literals
    {
        template <reflect_router::detail::FixedString fs>
        constexpr auto operator""_route()
        {
            return Route<fs>{};
        }
    } // namespace literals
} // namespace reflect_router