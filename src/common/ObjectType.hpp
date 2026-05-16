#pragma once

#include <string>

enum class ObjectType {
    RESERVE,
    CONSTANT,
    VARIABLE,
    TYPE,
    PROCEDURE,
    FUNCTION,
};

inline std::string objectTypeToString(ObjectType type) {
    switch (type) {
        case ObjectType::RESERVE:    return "RESERVE";
        case ObjectType::CONSTANT:   return "CONSTANT";
        case ObjectType::VARIABLE:   return "VARIABLE";
        case ObjectType::TYPE:       return "TYPE";
        case ObjectType::PROCEDURE:  return "PROCEDURE";
        case ObjectType::FUNCTION:   return "FUNCTION";
        default:                     return "UNKNOWN";
    }
}