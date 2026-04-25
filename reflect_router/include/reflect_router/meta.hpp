namespace reflect_router::meta {

    // Eine kleine Hilfsstruktur, die Typ und Name verknüpft
    template <typename T>
    struct TypeAlias {
        static constexpr std::meta::info type = ^^T; // oder ^^T, je nach dem was dein Clang schluckt
        std::string_view name;
    };

    template<typename... SupportedTypes>
    constexpr auto dispatch_type(std::string_view type_name) {
        // Wir definieren unsere EIGENEN Namen für die API
        constexpr TypeAlias<uint64_t> u64{ "uint64_t" };
        constexpr TypeAlias<std::string> str{ "string" };
        
        // Jetzt vergleichen wir gegen unsere sauberen Strings
        if (type_name == u64.name) return u64.type;
        if (type_name == str.name) return str.type;

        return ^^void; // Fehlerfall
    }
}