#include <algorithm>
#include <iostream>
#include <meta>
#include <print>
#include <utility>
#include <tuple>
#include "reflect_router/route.hpp"
#include <ranges>

// // 1. Wir definieren eine Liste der erlaubten Typen
// using supported_types = std::tuple<uint64_t, int32_t, std::string>;

// // 2. Wir suchen den Typen zur Compilezeit basierend auf dem String
// template <FixedString S>
// auto string_to_type_dispatcher() {
//     return []<size_t... Is>(std::index_sequence<Is...>) {
//         auto found_type = ...; // Magie zum Matchen des Strings S gegen display_name_of(^T)
//         // Hier kommt ein "Splicer" zum Einsatz [: ... :]
//         return ...;
//     }(std::make_index_sequence<std::tuple_size_v<supported_types>>{});
// }

template <typename T>
consteval auto get_member_count()
{
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^T, ctx).size();
}

template <typename T, size_t N = get_member_count<T>()>
consteval std::array<std::string_view, N> get_member_names()
{
    std::array<std::string_view, N> val;
    constexpr auto ctx = std::meta::access_context::current();
    size_t i{};
    template for (constexpr auto mem : define_static_array(nonstatic_data_members_of(^^T, ctx)))
    {
        val[i] = identifier_of(mem);
        ++i;
    }
    return val;
}


namespace reflect_router
{
    template <auto route>
    struct S;

    // Die Hilfsfunktion, die die Schleife kapselt
    template <auto route>
    consteval auto build_member_specs_safe()
    {
        constexpr std::size_t param_count = route.path_params().size();

        // Wir nutzen std::array statt std::vector
        std::array<std::meta::info, param_count> specs{};

        std::size_t i = 0;
        for (const auto &val : route.path_params())
        {
            // Typ auflösen
            auto type_info = meta::dispatch_type<uint64_t, std::string>(val.type);
            if (type_info == std::meta::info{}) {
                // Da wir in consteval sind, bricht das den Build mit dieser Meldung ab:
                throw "Unbekannter Typ in der Route gefunden! Check deinen Parser.";
            }
            std::meta::data_member_options opts;
            opts.name = val.name;

            // Ins Array einfügen
            specs[i++] = std::meta::data_member_spec(type_info, opts);
        }

        return specs;
    }

// --- 4. Der Tuple-Generator ---
    // Wir brauchen einen Weg, um aus einer Liste von meta::info ein Tuple zu machen
    template <std::meta::info... Infos>
    struct type_list {
        using type = std::tuple<typename [: Infos :]...>;
    };

    // Hilfskonstrukt um die Params der Route in ein Tuple zu wandeln
    template <auto route, std::size_t... Is>
    consteval auto make_tuple_type(std::index_sequence<Is...>) {
        return ^^typename type_list<reflect_router::meta::dispatch_type(route.path_params()[Is].type)...>::type;
    }

    // --- 5. Das Named-Tuple Gehäuse ---

    template<typename T>
    auto extract_value(std::string_view param)
    {
        if constexpr(std::is_integral<T>())
        {
            return std::atoi(std::string(param).c_str());
        }
        else if constexpr(std::same_as<T, std::string>)
        {
            return std::string(param);
        }
        else
        {

            return void{};
        }
    }

    template <auto route>
    struct Params {
        // Generiere den Tuple-Typ via Reflection
        using Storage = typename [: make_tuple_type<route>(
            std::make_index_sequence<route.path_params().size()>{}
        ) :];
        
        Storage data;

        // Named Accessor: Sucht den Index zur Compilezeit
        template <detail::FixedString Name>
        constexpr auto& get() {
            constexpr std::size_t idx = []() {
                for (std::size_t i = 0; i < route.path_params().size(); ++i) {
                    if (route.path_params()[i].name == static_cast<std::string_view>(Name)) return i;
                }
                throw "Param nicht gefunden!";
            }();
            return std::get<idx>(data);
        }
        // template for is currently broken here -> internal compiler error
        #ifdef USE_TEMPLATE_FOR
        explicit Params(std::string_view actual_route)
        {
            std::vector<std::string_view> parts;
            for (auto part : std::views::split(actual_route, '/'))
                parts.push_back(std::string_view{part});
            
            template for(constexpr auto param : route.path_params())
            {
                constexpr auto name = detail::FixedString<32>{param.name};
                using TargetType = std::remove_cvref_t<decltype(get<name>())>;
                get<name>() = extract_value<TargetType>(parts[param.index]);
            }
        }
        #else
        explicit Params(std::string_view actual_route)
{
    // 1. URI zerlegen wie gehabt
    std::vector<std::string_view> parts;
    for (auto part : std::views::split(actual_route.substr(1), '/'))
        parts.push_back(std::string_view{part});

    // 2. Die stabilere Variante der Compile-Time-Iteration
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        // Fold-Expression: Keine versteckten Lambdas, kein template for Stress
        ((
            // Wir greifen explizit über den Index Is zu
            get<detail::FixedString<32>(route.path_params()[Is].name)>() = 
                extract_value<std::remove_cvref_t<decltype(get<detail::FixedString<32>{route.path_params()[Is].name}>())>>(parts[route.path_params()[Is].index])
        ), ...);
    }(std::make_index_sequence<route.path_params().size()>{});
}
#endif
    };

    template<auto route, typename Func> requires(std::declval<Func>()(std::declval<Params<route>>()))
    void dispatch_route(std::string_view call, Func&& func)
    {
        // func(Params<route>{});
    }
};

int main()
{
    using namespace reflect_router::literals;
    constexpr auto route = "/api/:id:=uint64_t/:name:=string/:value:=uint64_t"_route;
    for (const auto val : route.path_parts())
        std::cout << val << '\n';
    std::cout << route.path_parts().size() << "\n";
    std::cout << route.path_params().size() << '\n';
    constexpr auto path_params = route.path_params();
    for (auto val : route.path_params())
    {
        std::cout << val.name << " -> " << val.type << '\n';
    }
    std::cout << "\n\n\n";
    ;

    // Die Route als NTTP
    constexpr auto my_route = "/api/:id:=uint64_t/:name:=string"_route;

    for (auto val : my_route.path_params())
    {
        std::cout << val.index << ": " << val.name << " -> " << val.type << '\n';
    }

    reflect_router::Params<my_route> req_params{"/api/12/Hallo123"};

    std::cout << "ID: " << req_params.get<"id">() << "\n";
    std::cout << "Name: " << req_params.get<"name">() << "\n";

}