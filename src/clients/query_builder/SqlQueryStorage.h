#pragma once

#include <variant>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

namespace marine_navi::clients::query_builder {

using BaseArgVar = std::variant<int64_t, double, std::string>;
using SingleArgVar = std::variant<BaseArgVar, std::optional<BaseArgVar> >;
using ComposedArgVar = std::variant<
    SingleArgVar,
    std::vector<SingleArgVar>,
    std::vector<std::vector<SingleArgVar>> >;

class SqlTemplate {
public:
    SqlTemplate(const std::string& query): name_(ParseName(query)), tokens_(ParseTokens(query)) {}
    const std::string& GetName() const;
    std::string MakeQuery(const std::vector<ComposedArgVar>& args) const;

private:
    using TokenType = std::variant<int, std::string>;

    static std::string ParseName(const std::string& query);
    static std::vector<TokenType> ParseTokens(const std::string& query);

private:
    std::string name_;
    std::vector<TokenType> tokens_;
};

class SqlQueryStorage {
public:
    SqlQueryStorage(const std::string& path_to_dir): storage_(LoadTemplates(path_to_dir)) {}
    const SqlTemplate& GetTemplate(const std::string& name) const;

private:
    static std::unordered_map<std::string, SqlTemplate> LoadTemplates(const std::string& path_to_dir);

private:
    const std::unordered_map<std::string, SqlTemplate> storage_;
};

namespace details {

inline ComposedArgVar processArgument(const BaseArgVar& arg) {
    return SingleArgVar{arg};
}

inline ComposedArgVar processArgument(const SingleArgVar& arg) {
    return arg;
}

inline ComposedArgVar processArgument(const std::vector<SingleArgVar>& arg) {
    return arg;
}

inline ComposedArgVar processArgument(const std::vector<std::vector<SingleArgVar>>& arg) {
    return arg;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
inline ComposedArgVar processArgument(const T& arg) {
    return SingleArgVar{BaseArgVar{static_cast<int64_t>(arg)}};
}

} // namespace details


template <typename... Args>
inline std::vector<ComposedArgVar> ComposeArguments(Args&&... args) {
    return { details::processArgument(std::forward<Args>(args))... };
}

} // namespace marine_navi::clients::query_builder
