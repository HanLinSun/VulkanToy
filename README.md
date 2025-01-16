# VulkanToy

Vulkan-based 3D Renderer(WIP)


- Now it is an visual studio project only(I use github templates)
- build on vs2022, you will need to install Vulkan SDK to make it operable.
- CMAKE WIP, Once all feature finished will post images.
In the future may add DirectX Support

## Build
This project use submodules so need to use 'git clone --recursive <project address>'
or if you have already used git clone to clone this project, need to use 'git submodule update'

## References/Credits

- Ray Tracing in One Weekend (https://github.com/petershirley/raytracinginoneweekend) scratch of the ray trace pipeline
- Vulkan Example made by Sascha Willems (https://github.com/SaschaWillems/Vulkan) which helps me to learn the Vulkan structure better
- PBRT-v4 (https://pbrt.org/users-guide-v4) for raytrace knowledge and relevant concept
- PBRT-v3 (https://github.com/mmp/pbrt-v3) code reference for implement sample method and MIS
- Vulkan Tutorial (https://vulkan-tutorial.com/) which helps me to start this project
- Game Engine tutorial made by the Cherno(https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT) for game event polling and render architecture
- Disney BRDF implements (https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf) as a start up for Disney BRDF 