#include "sigscan.hpp"

bool sigscan::check_pattern(uintptr_t start, const char *pattern, const char *mask) {
    for (; *mask; start++, pattern++, mask++) {
        if (*mask == 'x' && *reinterpret_cast<const char *>(start) != *pattern) {
            return false;
        }
    }
    return true;
}

uintptr_t sigscan::scan(const char *pattern, const char *mask, uintptr_t start, uintptr_t end) {
    for (; start < end - strlen(mask); start++) {
        if (check_pattern(start, pattern, mask)) {
            return start;
        }
    }
    return 0;
}

uintptr_t sigscan::scan(HMODULE module, const char *pattern, const char *mask) {
    MODULEINFO info;
    if (GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info))) {
        return scan(pattern, mask, reinterpret_cast<uintptr_t>(info.lpBaseOfDll), reinterpret_cast<uintptr_t>(info.lpBaseOfDll) + info.SizeOfImage);
    }
}
