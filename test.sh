#! /usr/bin/env bash -e

cmake --build --preset=default
GTEST_COLOR=1 ctest --preset=default
