cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..
emcmake cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE="Release"
cmake --build build --parallel 8
mkdir -p angular/src/assets
rm -Rf angular/src/assets/*
cp build/web_.* angular/src/assets/
cd angular
npm install -g npm@latest
npm install -g @angular/cli
npm install
ng build
ng serve --open --host 0.0.0.0
