# misaki
misaki is a physically-based photorealistic global illumination renderer written in modular, plugin based architecture

(still working in progress)
## Prerequisite
``vcpkg``
## Build
```
vcpkg install fmt pugixml tbb embree3 OpenImageIO
mkdir build
cd build
cmake ..
make -j8
```