#include "stringinterner.hpp"

namespace np::log {
StringInterner<1024*1024> global_interner;
}