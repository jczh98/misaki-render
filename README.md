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
- [x] Basic bsdfs: diffuse, dieletric, conductor...
- [x] Microfacet bsdfs: conductor, dieletric...
#### Lights
- [x] Area light and point light
- [x] Envrioment map with importance sample
## Roadmap
- Advance shading system
## Gallery
Bedroom (pt 1024spp) (scene ref [here](https://benedikt-bitterli.me/resources/)):
![pic](./assets/gallery/bedroom1024spp.png)