#pragma once

#include <string>
#include <variant>

using RuntimeValue = std::variant<int, double, bool, char, std::string>;

std::string runtimeValueToString(const RuntimeValue& value);
