#include "serializers.hpp"
#include <string>

namespace {
    struct UDT {};
}
static_assert(np::JsonComposer::is_literal<int>::value);
static_assert(np::JsonComposer::is_literal<bool>::value);
static_assert(!np::JsonComposer::is_literal<std::string>::value);
static_assert(!np::JsonComposer::is_literal<char>::value);
static_assert(!np::JsonComposer::is_literal<UDT>::value);

static_assert(!np::TextComposer::is_literal<int>::value);
static_assert(!np::TextComposer::is_literal<bool>::value);
static_assert(!np::TextComposer::is_literal<std::string>::value);
static_assert(!np::TextComposer::is_literal<char>::value);
static_assert(!np::TextComposer::is_literal<UDT>::value);

int main() {


}
