include "Dependencies.lua"

workspace "Ghost"
   configurations 
   { 
      "Debug", 
      "Release" 
   }

   platforms 
   { 
      "x64" 
   }     

project "Ghost"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"

   targetdir "bin/%{cfg.buildcfg}"

   files 
   { 
       "Ghost/**.h",
       "Ghost/**.cpp" 
   }

   includedirs
   {
      "Ghost",
      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.GLM}",
      "%{IncludeDir.KTX}",
      "%{IncludeDir.TINY_GLTF}",
      "%{IncludeDir.ImGUI}",
   } 

   

   links
   {
      "%{Library.Vulkan}",  
      "%{Library.ktx}"     
   }

   filter "system:windows"
      defines { 
         "VK_USE_PLATFORM_WIN32_KHR",
         "PROJECT_PATH=\"" .. os.getcwd() .. "\"" 
    } 

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter "platforms:x64"  
      architecture "x86_64"







