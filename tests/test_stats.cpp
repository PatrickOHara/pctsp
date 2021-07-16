
#include "pctsp/stats.hh"
#include <gtest/gtest.h>

TEST(TestStats, testCSVReader) {
    CSVReader reader("very_big_file.csv");

    for (auto& row : reader) {
        if (row["timestamp"].is_int()) {
            // Can use get<>() with any integer type, but negative
            // numbers cannot be converted to unsigned types
            row["timestamp"].get<int>();

            // ..
        }
    }
}