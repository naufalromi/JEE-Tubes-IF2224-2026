#include "RuntimeValue.hpp"

#include <sstream>
#include <type_traits>

std::string runtimeValueToString(const RuntimeValue& value)
{
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        } else if constexpr (std::is_same_v<T, char>) {
            return std::string(1, v);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return v;
        } else {
            std::ostringstream oss;
            oss << v;
            return oss.str();
        }
    }, value);
}