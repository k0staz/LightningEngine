#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <optional>
#include <functional>
#include <list>

namespace LE
{
#define FLOAT_MAX 3.402823466e+38F

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
using Map = std::unordered_map<Key, Value, Hasher>;

template <typename FuncType>
using FunctionRef = std::function<FuncType>;

template <typename T>
using Optional = std::optional<T>;

template <typename T>
using LinkedList = std::list<T>;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef std::string String;
}
