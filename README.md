# Ghost Engine

A lightweight, high-performance real-time game engine written in C++ using Vulkan.  
Ghost features custom rendering, physics simulation, and multiplayer networking — built from scratch.

---

## 🚀 Features

- 🔥 **Vulkan Renderer**  
  Real-time physically-based rendering (PBR), descriptor indexing, skybox support, mipmapping, and dynamic rendering.

- 🧠 **Multithreaded Task System**  
  Powered by [EnkiTS](https://github.com/dougbinks/enkiTS) for parallel asset loading and rendering.

- ⚙️ **Physics Engine**  
  Custom impulse-based collision and physics engine — supports AABB and dynamic rigid bodies.

- 🌐 **Networking Engine**  
  UDP-based client-server architecture for real-time multiplayer, with packet sequencing and interpolation.

- 🧵 **Asynchronous Asset Streaming**  
  Loads textures, GLTF models, and shaders in the background without stalling the frame loop.

- 🖼️ **GLTF Loader**  
  Supports PBR materials, node hierarchies, and skeletal structure loading.

- 🧰 **Debug UI**  
  Integration with Dear ImGui for real-time debugging and profiling.

---

## 📸 Screenshots

https://www.youtube.com/watch?v=YTnyMrknYvQ

---

## 🧪 Built With

- [Vulkan SDK](https://vulkan.lunarg.com/)
- [GLM](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [EnkiTS](https://github.com/dougbinks/enkiTS)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [KTX-Software](https://github.com/KhronosGroup/KTX-Software)

---
