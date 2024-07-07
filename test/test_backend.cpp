#include <gtest/gtest.h>
#include <string>

#include "test_lib.hpp"

void run_sysy(const std::string &filename) {
    auto id = get_id(filename);
    auto path = get_path(filename);

    // STUB
    ASSERT_FALSE(filename.empty());
}