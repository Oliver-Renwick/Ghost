#include "Vulkan_Init.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace graphics
{
	void GPUDevice::Init(DeviceInfo* deviceInfo)
	{
		bool supported = checkValidationLayerSupport();  
		if (enableValidationLayers && !supported)
		{
			throw std::runtime_error("requested but not found the validation layer");
		}
		 
		VkApplicationInfo appInfo{};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pEngineName = "Real_Time_Renderer_Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = "Real_Time_Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

		//Instance Info

		std::vector<const char*> instance_extension = { VK_KHR_SURFACE_EXTENSION_NAME };

		if (_WIN32)
			instance_extension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		uint32_t extCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		std::vector<VkExtensionProperties> extProperties(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProperties.data());

		if (extProperties.size() > 0)
		{
			for (auto& extension : extProperties)
			{
				supportedInstanceExtension.push_back(extension.extensionName);
			}
		}

		VkInstanceCreateInfo InstanceCI{};
		InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceCI.pApplicationInfo = &appInfo;
		InstanceCI.pNext = nullptr;

		if (std::find(supportedInstanceExtension.begin(), supportedInstanceExtension.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtension.end())
		{
			instance_extension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instance_extension.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		if (instance_extension.size() > 0)
		{
			InstanceCI.enabledExtensionCount = static_cast<uint32_t>(instance_extension.size());
			InstanceCI.ppEnabledExtensionNames = instance_extension.data();
		}

		//Layer
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			InstanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			InstanceCI.ppEnabledLayerNames = validationLayers.data();

			debug::setupDebugingMessengerCreateInfo(debugCreateInfo);
			InstanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			InstanceCI.enabledLayerCount = 0;

			InstanceCI.pNext = nullptr;
		}

		VK_CHECK_RESULT(vkCreateInstance(&InstanceCI, nullptr, &deviceInfo->Instance));

		//Debug Creation

		debug::setupDebugging(deviceInfo->Instance);


		//Window Surface Creation
		VkWin32SurfaceCreateInfoKHR surfaceCI{};
		surfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCI.hinstance = deviceInfo->hInst;
		surfaceCI.hwnd = deviceInfo->window;

		VkResult res = vkCreateWin32SurfaceKHR(deviceInfo->Instance, &surfaceCI, nullptr, &deviceInfo->_surface);

		if (res != VK_SUCCESS)
			throw std::runtime_error("Cannot able to create Window Surface");


		//Physical Device
		uint32_t device_Count = 0;

		vkEnumeratePhysicalDevices(deviceInfo->Instance, &device_Count, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(device_Count);
		vkEnumeratePhysicalDevices(deviceInfo->Instance, &device_Count, physicalDevices.data());

		if (device_Count == 0) { throw std::runtime_error("Cannot able to Find Suitable Gpu to run the application"); }
		for (const auto& device : physicalDevices)
		{
			bool check = IsDeviceSuitable(device, deviceInfo->_surface);
			if (check)
			{
				deviceInfo->PhysicalDevice = device;
				deviceInfo->msaaSamples = getMaxUsableSampleCount(deviceInfo->PhysicalDevice);
				break;
			}
		}

		std::cout << physicalDeviceProperties.deviceName << std::endl;



		//Queue Family Creation
		QueueFamily indices = QueueFamily::findQueueFamily(deviceInfo->PhysicalDevice);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsQueueIndex.value(), indices.computeQueueIndex.value() , indices.transferQueueIndex.value() };

		deviceInfo->graphicsQueueIndex = indices.graphicsQueueIndex.value();
		deviceInfo->computeQueueIndex = indices.computeQueueIndex.value();
		deviceInfo->transferQueueIndex = indices.transferQueueIndex.value();

		std::cout << "Graphics Index => " << indices.graphicsQueueIndex.value() << std::endl;
		std::cout << "Compute Index => " << indices.computeQueueIndex.value() << std::endl;
		std::cout << "Transfer Index => " << indices.transferQueueIndex.value() << std::endl;

		float queuePriorities[2] = { 1.0f, 1.0f };

		for (uint32_t queueIDX : uniqueQueueFamilies)
		{
			if (queueIDX == 0)
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueIDX;
				queueInfo.queueCount = 2;
				queueInfo.pQueuePriorities = queuePriorities;
				queueCreateInfos.push_back(queueInfo);
			}
			else
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueIDX;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &queuePriorities[0];
				queueCreateInfos.push_back(queueInfo);
			}
		}

		//Logical Device Creation

		deviceInfo->indexingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		deviceFeatures_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures_2.pNext = &deviceInfo->indexingFeature;

		vkGetPhysicalDeviceFeatures2(deviceInfo->PhysicalDevice, &deviceFeatures_2);

		bool bindlessSupported = 
			deviceInfo->indexingFeature.descriptorBindingPartiallyBound && 
			deviceInfo->indexingFeature.runtimeDescriptorArray && 
			deviceInfo->indexingFeature.descriptorBindingSampledImageUpdateAfterBind && 
			deviceInfo->indexingFeature.descriptorBindingVariableDescriptorCount && 
			deviceInfo->indexingFeature.descriptorBindingUpdateUnusedWhilePending &&
			deviceInfo->indexingFeature.shaderSampledImageArrayNonUniformIndexing &&
			deviceInfo->indexingFeature.descriptorBindingStorageBufferUpdateAfterBind;


		VkDeviceCreateInfo DeviceCI{};
		DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCI.pNext = nullptr;

		if (enableValidationLayers)
		{
			DeviceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			DeviceCI.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			DeviceCI.enabledLayerCount = 0;
		}

		//Solves for Shading Aliasing
		deviceFeatures_2.features.sampleRateShading = VK_TRUE;
		deviceFeatures_2.features.fillModeNonSolid = VK_TRUE;

		deviceFeatures_2.features.samplerAnisotropy = VK_TRUE;
		deviceFeatures_2.features.geometryShader = VK_TRUE;

		DeviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		DeviceCI.ppEnabledExtensionNames = deviceExtensions.data();

		DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		DeviceCI.pQueueCreateInfos = queueCreateInfos.data();

		DeviceCI.pNext = &deviceFeatures_2;

		if (bindlessSupported)
		{
			deviceFeatures_2.pNext = &deviceInfo->indexingFeature;
		}

		//Indexing Properties
		deviceInfo->indexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
		VkPhysicalDeviceProperties2 properties2{};
		properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		properties2.pNext = &deviceInfo->indexingProperties;
		vkGetPhysicalDeviceProperties2(deviceInfo->PhysicalDevice, &properties2);

		//Device Creation and Queue Creation
		VK_CHECK_RESULT(vkCreateDevice(deviceInfo->PhysicalDevice, &DeviceCI, nullptr, &deviceInfo->Device));
		vkGetDeviceQueue(deviceInfo->Device, indices.graphicsQueueIndex.value(), 0, &deviceInfo->GraphicsQueue);
		vkGetDeviceQueue(deviceInfo->Device, indices.graphicsQueueIndex.value(), 1, &deviceInfo->transferGraphicsQueue);
		vkGetDeviceQueue(deviceInfo->Device, indices.computeQueueIndex.value(), 0, &deviceInfo->ComputeQueue);
		vkGetDeviceQueue(deviceInfo->Device, indices.transferQueueIndex.value(), 0, &deviceInfo->TransferQueue);
		

		//Swapchain

		InitSwapchain(deviceInfo);
	}


	void GPUDevice::InitSwapchain(DeviceInfo* deviceInfo)
	{
		SwapChainSupportDetails swapchainDetails = queryswapchain(deviceInfo->PhysicalDevice, deviceInfo->_surface);

		VkExtent2D extent = chooseswapextent(swapchainDetails.capabilities, deviceInfo->width, deviceInfo->height);
		VkSurfaceFormatKHR Surface_Format = chooseswapsurfaceFormat(swapchainDetails.formats);
		VkPresentModeKHR present_Mode = choosepresentMode(swapchainDetails.presentModes);

		uint32_t image_count = swapchainDetails.capabilities.minImageCount + 1;

		if (swapchainDetails.capabilities.maxImageCount > 0 && image_count > swapchainDetails.capabilities.maxImageCount)
		{
			image_count = swapchainDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainCI{};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.surface = deviceInfo->_surface;

		swapchainCI.minImageCount = image_count;
		swapchainCI.imageFormat = Surface_Format.format;
		swapchainCI.imageColorSpace = Surface_Format.colorSpace;
		swapchainCI.imageExtent = extent;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//Swapchain Transformation
		swapchainCI.preTransform = swapchainDetails.capabilities.currentTransform;

		//Blending of other window systems
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		swapchainCI.presentMode = present_Mode;
		swapchainCI.clipped = VK_TRUE;

		//To-Do : implementaion in swapchain recreation
		swapchainCI.oldSwapchain = VK_NULL_HANDLE;

		VkResult res = vkCreateSwapchainKHR(deviceInfo->Device, &swapchainCI, nullptr, &deviceInfo->_swapChain);

		if (res != VK_SUCCESS)
			throw std::runtime_error("Cannot able to create swapchain");

		deviceInfo->swapchainImageFormat = Surface_Format.format;
		deviceInfo->swapchainColorSpace = Surface_Format.colorSpace;
		deviceInfo->swapchainextent = extent;

		vkGetSwapchainImagesKHR(deviceInfo->Device, deviceInfo->_swapChain, &image_count, nullptr);
		deviceInfo->swapchain_Images.resize(image_count);
		vkGetSwapchainImagesKHR(deviceInfo->Device, deviceInfo->_swapChain, &image_count, deviceInfo->swapchain_Images.data());


		// Create Swap Chain Image View

		deviceInfo->swapchainImageView.resize(image_count);

		for (int i = 0; i < image_count; i++)
		{
			VkImageViewCreateInfo imageViewInfo{};
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.pNext = nullptr;
			imageViewInfo.image = deviceInfo->swapchain_Images[i];

			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.format = deviceInfo->swapchainImageFormat;

			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.levelCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(deviceInfo->Device, &imageViewInfo, nullptr, &deviceInfo->swapchainImageView[i]));

		}
	}


	VkExtent2D GPUDevice::chooseswapextent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height)
	{
		VkExtent2D extent{};

		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		else
		{
			extent.width = static_cast<uint32_t>(width);
			extent.height = static_cast<uint32_t>(height);

			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		return extent;
	}

	VkPresentModeKHR GPUDevice::choosepresentMode(const std::vector<VkPresentModeKHR>& availablePresent)
	{
		for (const auto& present : availablePresent)
		{
			if (present == VK_PRESENT_MODE_MAILBOX_KHR)
				return present;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormatKHR GPUDevice::chooseswapsurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormat)
	{
		//UNORM FORMAT
		//VkSurfaceFormatKHR selectedFormat = _availableFormat[0];
		//std::vector<VkFormat> preferredImageFormats = {
		//	VK_FORMAT_B8G8R8A8_UNORM,
		//	VK_FORMAT_R8G8B8A8_UNORM,
		//	VK_FORMAT_A8B8G8R8_UNORM_PACK32
		//};
		//
		//for (auto& availableFormat : _availableFormat) {
		//	if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
		//		selectedFormat = availableFormat;
		//		break;
		//	}
		//}
		//
		//return selectedFormat;

		for (const auto& format : _availableFormat)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}
		
		return _availableFormat[0];
	}


	SwapChainSupportDetails GPUDevice::queryswapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails supportDetails{};

		//Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &supportDetails.capabilities);

		//Format
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			supportDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, supportDetails.formats.data());
		}

		//Present Mode
		uint32_t presentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			supportDetails.presentModes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, supportDetails.presentModes.data());
		}

		return supportDetails;
	}

	QueueFamily QueueFamily::findQueueFamily(const VkPhysicalDevice physicalDevice)
	{
		uint32_t queueCount = 0;

		QueueFamily indices;

		indices.graphicsQueueIndex = getQueueFamilyIndex(physicalDevice , VK_QUEUE_GRAPHICS_BIT);
		indices.computeQueueIndex  = getQueueFamilyIndex(physicalDevice, VK_QUEUE_COMPUTE_BIT);
		indices.transferQueueIndex = getQueueFamilyIndex(physicalDevice, VK_QUEUE_TRANSFER_BIT);

		if (indices.isComplete())
		{
			return indices;
		}

		else
		{
			throw std::runtime_error("Cannot create all the neccessary queue families");
		}
	}

	VkSampleCountFlagBits GPUDevice::getMaxUsableSampleCount(const VkPhysicalDevice& PhysicalDevice)
	{
		VkPhysicalDeviceProperties _physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(PhysicalDevice, &_physicalDeviceProperties);

		VkSampleCountFlags count = _physicalDeviceProperties.limits.framebufferColorSampleCounts & _physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (count & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (count & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (count & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (count & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (count & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (count & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}

	bool GPUDevice::IsDeviceSuitable(const VkPhysicalDevice& Physicaldevice, VkSurfaceKHR surface)
	{
		vkGetPhysicalDeviceFeatures(Physicaldevice, &physicalDeviceFeatures);
		vkGetPhysicalDeviceProperties(Physicaldevice, &physicalDeviceProperties);

		bool extensionSupport = checkDeviceExtension(Physicaldevice);

		QueueFamily indices = QueueFamily::findQueueFamily(Physicaldevice);

		SwapChainSupportDetails swapchainDetails = queryswapchain(Physicaldevice, surface);

		bool swapchainAdequate = false;
		if (extensionSupport)
		{
			swapchainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentModes.empty();
		}


		return physicalDeviceFeatures.samplerAnisotropy && extensionSupport && indices.isComplete() && swapchainAdequate;
	}

	bool GPUDevice::checkDeviceExtension(const VkPhysicalDevice& PhysicalDevice)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> ExtensionProperties(extensionCount);
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, ExtensionProperties.data());

		std::set<std::string> extensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extProp : ExtensionProperties)
		{
			extensions.erase(extProp.extensionName);
		}

		return extensions.empty();
	}

	bool GPUDevice::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			  
			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void GPUDevice::shutDown(DeviceInfo* deviceInfo)
	{  
		for (auto& swapView : deviceInfo->swapchainImageView)
		{
			vkDestroyImageView(deviceInfo->Device, swapView, nullptr);
		}

		vkDestroySwapchainKHR(deviceInfo->Device, deviceInfo->_swapChain, nullptr);
		vkDestroyDevice(deviceInfo->Device, nullptr);
		vkDestroySurfaceKHR(deviceInfo->Instance, deviceInfo->_surface, nullptr);
		debug::freeDebugCallback(deviceInfo->Instance);
		vkDestroyInstance(deviceInfo->Instance, nullptr);
	}

}