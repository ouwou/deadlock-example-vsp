#pragma once

#include <Windows.h>
#include <Psapi.h>

#include <cstdint>

namespace sigscan {
bool check_pattern(uintptr_t start, const char *pattern, const char *mask);
uintptr_t scan(const char *pattern, const char *mask, uintptr_t start, uintptr_t end);
uintptr_t scan(HMODULE module, const char *pattern, const char *mask);
} // namespace sigscan
