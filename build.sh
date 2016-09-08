#!/usr/bin/env bash

echo "Environment: 'uname -a'"
echo "Compiler: '$CXX --version'"
export CXX="g++-6"
make || exit 1
make test || exit 1
