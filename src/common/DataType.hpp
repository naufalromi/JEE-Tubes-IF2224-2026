#pragma once
#include <string>

enum class DataType{
    UNKNOWN,
    INTEGER,
    REAL,
    CHAR,
    BOOLEAN,
    STRING,
    SUBRANGE,
    ENUMERATED,
    ARRAY,
    RECORD,
    VOID, // Ada di spek di contoh jir
    ERROR
};


inline std::string dataTypeToString(DataType type){
    switch (type)
    {
        case DataType::INTEGER:     return "INTEGER";
        case DataType::REAL:        return "REAL";
        case DataType::CHAR:        return "CHAR";
        case DataType::BOOLEAN:     return "BOOLEAN";
        case DataType::STRING:      return "STRING";
        case DataType::SUBRANGE:    return "SUBRANGE";
        case DataType::ENUMERATED:  return "ENUMERATED";
        case DataType::ARRAY:       return "ARRAY";
        case DataType::RECORD:      return "RECORD";
        case DataType::VOID:        return "VOID";
        case DataType::ERROR:       return "ERROR";
        default: return "UNKOWN";

    }
}