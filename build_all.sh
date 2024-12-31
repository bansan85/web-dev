#npm install -g npm@latest
#npm install -g @angular/cli

git submodule update --init --recursive --depth=1

# Build generator with native llvm to prepare wrapper for clang-format
cmake -S cpp/native -B build_native_release -G Ninja -DCMAKE_BUILD_TYPE=Release
cd build_native_release
npm install typescript
export PATH=$PATH:$(pwd)/node_modules/.bin
cmake --build . --parallel 8
cd ..

cmake -S cpp/native -B build_native_debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cd build_native_debug
npm install typescript
export PATH=$PATH:$(pwd)/node_modules/.bin
cmake --build . --parallel 8
cd ..

# Build wasm-assembly wrapper
cd cpp/third_party/emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ../../..

emcmake cmake -S cpp/webassembly -B build_webassembly_release -G "Ninja" -DCMAKE_BUILD_TYPE="Release"
cmake --build build_webassembly_release --parallel $(nproc --all)
mkdir -p angular/src/assets
rm -f angular/src/assets/*
cp build_webassembly_release/web* angular/src/assets/

emcmake cmake -S cpp/webassembly -B build_webassembly_debug -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DWITH_SANITIZE_ADDRESS=ON -DWITH_SANITIZE_UNDEFINED=ON
cmake --build build_webassembly_debug --parallel $(nproc --all)
mkdir -p angular/src/assets
rm -f angular/src/assets/*
cp build_webassembly_debug/web* angular/src/assets/

# Build Angular project
cd angular
npm install

exit 1

ng build
ng test --browsers=ChromeHeadless --watch=false
ng serve --open --host 0.0.0.0
ng lint
cd ..

cmake -S cpp/native -B build_utils_iwyu -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
/usr/lib/llvm/18/bin/iwyu_tool.py -p build_utils_iwyu utils/*.cpp > iwyu_tool.log
run-clang-tidy -p build_utils_iwyu utils/*.cpp > clang-tidy-utils.log

emcmake cmake -S cpp/webassembly -B build_iwyu -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build_iwyu --parallel 8
/usr/lib/llvm/18/bin/iwyu_tool.py -p build_iwyu src/*.cpp > iwyu_tool.log
run-clang-tidy -p build_iwyu src/*.cpp > clang-tidy-utils.log

mkdir -p cpp/tests/output-lighten-number
cmake -S cpp/tests/ -B build_tests_debug -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DWITH_SANITIZE_ADDRESS=OFF -DWITH_SANITIZE_UNDEFINED=OFF -G "Ninja" -DCMAKE_BUILD_TYPE="Debug"
cmake --build build_tests_debug --parallel $(nproc --all)
afl-fuzz -i cpp/tests/seeds-lighten-number -o cpp/tests/output-lighten-number -- ./build_tests_debug/test_lighten_number
