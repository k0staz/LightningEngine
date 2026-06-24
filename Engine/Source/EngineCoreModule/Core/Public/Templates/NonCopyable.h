#pragma once

namespace LE
{
class NonCopyable
{
public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;

protected:
	NonCopyable() {}
	~NonCopyable() {}
};
}