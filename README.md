# Ghost Engine

**Ghost** is a custom game engine built from scratch using **C++** and **Vulkan**. It features:

- Multithreaded draw submission  
- Asynchronous asset streaming  
- Frame graph system for efficient render pass management  
- Custom UDP-based multiplayer networking  
- Impulse-based physics simulation
- Bindless Descriptor
- Dynamic Rendering
- GLTF file loading with full PBR material support
-  skeletal animation (in progress)  
- Async compute (in progress)

---
&nbsp;

## 🎥 Youtube Videos

[![Preview 1](https://github-production-user-asset-6210df.s3.amazonaws.com/111285385/446884791-8d41dd2a-a300-42da-beaf-d41fda5eccab.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20250523%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20250523T063620Z&X-Amz-Expires=300&X-Amz-Signature=679e2d0420f2cee05f0ad04a793d6f7536d315e7120ec0d1d4cc959558eb32f1&X-Amz-SignedHeaders=host)](https://www.youtube.com/watch?v=YTnyMrknYvQ)  
👉 **[Watch on YouTube](https://www.youtube.com/watch?v=YTnyMrknYvQ)

&nbsp;

[![Preview 2](https://github-production-user-asset-6210df.s3.amazonaws.com/111285385/446886449-c80a4594-3007-4cc5-a573-7e0108b0582e.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20250523%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20250523T063709Z&X-Amz-Expires=300&X-Amz-Signature=874aa3c8ccfc35c114ee900b56f6035ef035a6afe84038c07e25a69f9a4aac04&X-Amz-SignedHeaders=host)](https://www.youtube.com/watch?v=0siSgmZTg-s)  
👉 **[Watch on YouTube](https://www.youtube.com/watch?v=0siSgmZTg-s)

---

&nbsp;

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
