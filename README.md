# misaki-render
misaki render is a physically-based photorealistic global illumination renderer written in modular, plugin based architecture

(still working in progress)
## Prerequisite
``vcpkg``
## Build
```
vcpkg install fmt rttr pugixml tbb embree3 OpenImageIO
mkdir build
cd build
cmake ..
make -j8
```
## Features
#### Light transport algorithm
- [x] Path tracing with mis
#### Material models
- [x] Basic bsdfs: diffuse, dieletric
- [ ] Microfacet bsdfs: glass, disney...
#### Lights
- [x] Area light
- [ ] Envrioment map
## Roadmap
- Advance shading system
## Gallery