-- Project Dependencies 

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}

IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["GLM"] = "%{wks.location}/Dependencies/glm"
IncludeDir["KTX"] = "%{wks.location}/Dependencies/ktx/include"
IncludeDir["TINY_GLTF"] = "%{wks.location}/Dependencies/tinygltf"
IncludeDir["ImGUI"] = "%{wks.location}/Dependencies/imgui"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["KTX"] = "%{wks.location}/Dependencies/ktx/lib"

Library = {}

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["ktx"] = "%{LibraryDir.KTX}/ktx.lib"