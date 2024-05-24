#include "def.h"

#include <memory>

#include <utility>

pCode make_code(String code, String file_name) {
    return std::make_shared<Code>(Code{std::make_shared<String>(std::move(code)), std::move(file_name)});
}
