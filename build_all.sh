cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..
emcmake cmake -S . -B build -G "Ninja Multi-Config"
cmake --build build --config Release --parallel 8
mkdir angular/src/assets
cp build/Release/web_demangler.* angular/src/assets/
cd angular
npm install -g npm@latest
npm install -g @angular/cli
npm install
ng build

