Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`

cmake --build build --target clean

cmake --build build

cd /mnt/e/Web/minnow/build && /usr/bin/ctest --force-new-ctest-process --output-on-failure --stop-on-failure --timeout 30 -E 'speed_test|optimization|webget'

cd /mnt/e/Web/minnow/build && /usr/bin/ctest --output-on-failure --stop-on-failure --timeout 30 -R '^net_interface'

cd /mnt/e/Web/minnow/build && /usr/bin/ctest --output-on-failure --stop-on-failure --timeout 30 -R '^net_interface|^router'