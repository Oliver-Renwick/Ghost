#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <optional>

#include "Tools.h"
#include "Vulkan_debug.h"


//This Unit has Initalizer Resource for Vulkan object

namespace graphics
{

	struct DeviceInfo
	{
		VkInstance                   Instance			 = VK_NULL_HANDLE;
		VkSurfaceKHR                 _surface			 = VK_NULL_HANDLE;
		VkDevice                     Device				 = VK_NULL_HANDLE;
		VkPhysicalDevice             PhysicalDevice		 = VK_NULL_HANDLE;
		VkQueue                      GraphicsQueue		 = VK_NULL_HANDLE;
		VkQueue                      PresentQueue		 = VK_NULL_HANDLE;
		VkSwapchainKHR               _swapChain			 = VK_NULL_HANDLE;
		VkCommandPool                commandPool		 = VK_NULL_HANDLE;
		VkRenderPass                 renderPass			 = VK_NULL_HANDLE;
		VkPipelineLayout             pipelineLayout		 = VK_NULL_HANDLE;
		VkDescriptorPool			 DescriptorPool		 = VK_NULL_HANDLE; // Default Descriptor Pool For Now
		VkPipeline                   pipeline			 = VK_NULL_HANDLE; // Default Pipeline For Now;
		VkDescriptorSetLayout        descriptorSetLayout = VK_NULL_HANDLE; // Set Layout for Uniform Buffers
		VkDescriptorSet              descriptorSet		 = VK_NULL_HANDLE; // Descriptor Sets for Uniform Buffer

		std::vector<VkFramebuffer>   frameBuffers;
		std::vector<VkCommandBuffer> _CommandBuffers;// This is the default command Buffer for now!!
		std::vector<VkSemaphore>     imageAvailableSemaphore;
		std::vector<VkSemaphore>     renderFinishedSemaphore;
		std::vector<VkFence>         inFlightFence;

		int       width  = 0;
		int       height = 0;
		HINSTANCE hInst  = nullptr; 
		HWND      window = nullptr;

		std::vector<VkImage> swapchain_Images;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainextent;
		std::vector<VkImageView> swapchainImageView;
		int MAX_FRAME_IN_FLIGHT = 2;

		bool displayShadowMap = false;


	};

	struct OffscreenData
	{
		VkFramebuffer		frameBuffer = VK_NULL_HANDLE;
		VkRenderPass		renderPass = VK_NULL_HANDLE;
		VkDescriptorSet		descriptorSet		 = VK_NULL_HANDLE; // DescriptorSet for Shadows
		VkDescriptorSet     debugDescriptor = VK_NULL_HANDLE;
		VkPipeline			pipeline  = VK_NULL_HANDLE; // Pipeline for Shadow Rendering
		VkPipeline				debugPipeline	= VK_NULL_HANDLE; //Debug pipeline
		
		VkDescriptorImageInfo descriptorimageInfo;
		VkSampler depthsampler;

		VkImage depthImage;
		VkDeviceMemory depthMemory;
		VkImageView depthView;

		uint32_t width = 4000, height = 4000;
		float lightFOV = 55.0f;
		float zNear = 1.0f;
		float zFar = 500.0f;
		VkFormat offScreenDepthFormat{ VK_FORMAT_D16_UNORM };
		// Depth bias (and slope) are used to avoid shadowing artifacts
		// Constant depth bias factor (always applied)
		float depthBiasConstant = 1.25f;
		// Slope depth bias factor, applied depending on polygon's slope
		float depthBiasSlope = 1.75f;
	};

	struct QueueFamily
	{
		std::optional<uint32_t> graphicsQueueIndex;
		std::optional<uint32_t> presentQueueIndex;

		bool isComplete()
		{
			return graphicsQueueIndex.has_value();
		}

		static QueueFamily findQueueFamily(const VkPhysicalDevice physicalDevice);
	};
	
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct GPUDevice
	{
		std::vector<std::string> supportedInstanceExtension;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		void		 Init(DeviceInfo* deviceInfo);
 static void         InitSwapchain(DeviceInfo* deviceInfo);
		void         shutDown(DeviceInfo* deviceInfo);

		//Helper
		bool checkValidationLayerSupport();
		bool IsDeviceSuitable(const VkPhysicalDevice& Physicaldevice, VkSurfaceKHR surface);
		bool checkDeviceExtension(const VkPhysicalDevice& PhysicalDevice);
		static SwapChainSupportDetails queryswapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		static VkExtent2D chooseswapextent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);
		static VkPresentModeKHR choosepresentMode(const std::vector<VkPresentModeKHR>& availablePresent);
		static VkSurfaceFormatKHR chooseswapsurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormat);
	};
}