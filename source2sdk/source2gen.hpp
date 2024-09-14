#pragma once
#include <cstdint>
#include <string_view>

#include <convar.h>
#include <mathlib/vector4d.h>
#include <variant.h>

#include "../deadlock/defs.hpp"

// intentionally left undefined. if you want to access static fields, add your own sdk.
namespace interfaces {
struct SchemaStaticFieldData_t {
    void *m_pInstance {};
};

struct CSchemaClassInfo {
    auto GetStaticFields() -> SchemaStaticFieldData_t **;
};

struct CSchemaSystemTypeScope {
    auto FindDeclaredClass(std::string_view) -> CSchemaClassInfo *;
};

struct schema_t {
    auto FindTypeScopeForModule(std::string_view) -> CSchemaSystemTypeScope *;
};

extern schema_t *g_schema;
} // namespace interfaces
