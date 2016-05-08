#!/usr/bin/env bash

echo "Environment: 'uname -a'"
echo "Compiler: '$CXX --version'"
export CXX="g++-4.9"
make || exit 1
make test || exit 1
