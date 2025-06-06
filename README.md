# Ghost Engine

**Ghost** is a custom game engine built from scratch using **C++** and **Vulkan**. It features:

- Multithreaded draw submission  
- Asynchronous asset streaming  
- Frame graph data structure for efficient render pass management  
- Custom UDP-based multiplayer networking  
- Impulse-based physics simulation
- Bindless Descriptor
- Dynamic Rendering
- GLTF file loading with full PBR material support
-  skeletal animation (in progress)  
- Async compute (in progress)
&nbsp;

---

## 🎥 Youtube Videos

[![Preview 1](https://github-production-user-asset-6210df.s3.amazonaws.com/111285385/446884791-8d41dd2a-a300-42da-beaf-d41fda5eccab.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20250523%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20250523T063620Z&X-Amz-Expires=300&X-Amz-Signature=679e2d0420f2cee05f0ad04a793d6f7536d315e7120ec0d1d4cc959558eb32f1&X-Amz-SignedHeaders=host)](https://www.youtube.com/watch?v=YTnyMrknYvQ)  
👉 **[Watch on YouTube](https://www.youtube.com/watch?v=YTnyMrknYvQ)

&nbsp;

[![Preview 2](https://github-production-user-asset-6210df.s3.amazonaws.com/111285385/446886449-c80a4594-3007-4cc5-a573-7e0108b0582e.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20250523%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20250523T063709Z&X-Amz-Expires=300&X-Amz-Signature=874aa3c8ccfc35c114ee900b56f6035ef035a6afe84038c07e25a69f9a4aac04&X-Amz-SignedHeaders=host)](https://www.youtube.com/watch?v=0siSgmZTg-s)  
👉 **[Watch on YouTube](https://www.youtube.com/watch?v=0siSgmZTg-s)
&nbsp;

---


## 🔧 Built With

- **C++** — Core language used for performance and control  
- **Vulkan** — Low-level graphics API for rendering  
- **GLM** — Mathematics library for graphics (vector/matrix operations)  
- **EnkiTS** — Multithreaded task scheduler for parallelism  
- **KTX** — Texture loading support (Basis Universal, mipmaps, etc.)  
- **TinyGLTF** — 3D asset format for PBR materials, skeletal animations  
- **Dear ImGui** — Debug tools and in-engine UI overlay  
- **Winsock** — Windows Sockets API for custom UDP multiplayer  
- **nlohmann::json** — Lightweight JSON parser used to define Frame Graph / Render Graph structures
&nbsp;

---


## 🚧 On Progress Features

- **Animation Support** — Implementing full skeletal and morph target animations for characters and objects to enable smooth, realistic motion.

- **Asynchronous Compute** — Integrating asynchronous compute shaders to offload non-graphics tasks such as physics calculations and post-processing effects. This allows better GPU utilization by running compute workloads concurrently with graphics rendering.

&nbsp;
---


## 🎯 Future Goals

- **Mesh Shaders** — Planning to adopt mesh shaders for more flexible and efficient geometry processing, enabling finer control over the graphics pipeline and potentially improving performance for complex scenes.

- **Robust Physics Collision** — Enhancing the physics engine to support more accurate and stable collision detection and response.

- **Co-op Networking** — Expanding the multiplayer system to support cooperative multiplayer gameplay with synchronized states.

&nbsp;
---
> **Note**  
> This engine is built exclusively using the Win32 API and is supported only on Windows.

