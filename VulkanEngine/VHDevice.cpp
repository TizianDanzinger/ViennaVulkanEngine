/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/

#include "VHHelper.h"

namespace vh {

	//-------------------------------------------------------------------------------------------------------
	
	/**
	*
	* \brief Check validation layers of the Vulkan instance
	* \param[in] validationLayers
	* \returns true if all requested layers are supported by the device, else false
	*
	*/
	bool checkValidationLayerSupport(std::vector<const char*> &validationLayers) {
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

	//-------------------------------------------------------------------------------------------------------
	/**
	*
	* \brief Create a Vulkan instance
	* \param[in] extensions Requested layers 
	* \param[in] validationLayers Requested validation layers
	* \param[out] instance The new instance
	* \returns VkResult of the operation
	*
	*/
	VkResult vhDevCreateInstance(std::vector<const char*> &extensions, std::vector<const char*> &validationLayers, VkInstance *instance) {

		if (validationLayers.size() > 0 && !checkValidationLayerSupport(validationLayers) ) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		return vkCreateInstance(&createInfo, nullptr, instance);
	}


	//-------------------------------------------------------------------------------------------------------
	/**
	*
	* \brief Find suitable queue families of a given physical device
	* \param[in] device A physical device
	* \param[in] surface The surface of a window
	* \returns a structure containing queue family indices of suitable families
	*
	*/
	QueueFamilyIndices vhDevFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface ) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}


	/**
	*
	* \brief Check whether a given physical device offers a list of required extensions
	* \param[in] device A physical device
	* \param[in] requiredDeviceExtensions A list with required device extensions
	* \returns whether the device supports all extensions or not
	*
	*/
	bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredDeviceExtensions ) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	/**
	*
	* \brief Query which swap chains a physical device supports
	* \param[in] device A physical device
	* \param[in] surface A window surface
	* \returns a structure holding details about capabilities, formats and present modes offered by the device
	*
	*/
	SwapChainSupportDetails vhDevQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface ) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	/**
	*
	* \brief Query whether a physical offers all required extensions
	* \param[in] device A physical device
	* \param[in] surface A window surface
	* \param[in] requiredDeviceExtensions A list of required device extensions
	* \returns whether the device supports all required extensions or not
	*
	*/
	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> requiredDeviceExtensions) {
		QueueFamilyIndices indices = vhDevFindQueueFamilies(device, surface);

		bool extensionsSupported = checkDeviceExtensionSupport(device, requiredDeviceExtensions);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = vhDevQuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate  && supportedFeatures.samplerAnisotropy;
	}


	/**
	*
	* \brief Query whether a physical offers all required extensions
	* \param[in] instance The Vulkan instance
	* \param[in] surface A window surface
	* \param[in] requiredDeviceExtensions A list of required device extensions
	* \param[out] physicalDevice A physical device that supports all required extensions
	*
	*/
	void vhDevPickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, 
								std::vector<const char*> requiredDeviceExtensions,
								VkPhysicalDevice *physicalDevice) {

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto device : devices) {
			if (isDeviceSuitable(device, surface, requiredDeviceExtensions)) {
				*physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

	}


	/**
	*
	* \brief Query a suitable image format that the device supports
	* \param[in] physicalDevice The physical device
	* \param[in] candidates A list with candidate formats
	* \param[in] tiling Linear or optimal
	* \param[in] features Usage of the format
	* \returns a suitable format that is supported by the physical device
	*
	*/
	VkFormat vhDevFindSupportedFormat(	VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, 
										VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}


	/**
	*
	* \brief Find a suitable format for the depth/stencil buffer
	* \param[in] physicalDevice The physical device
	* \returns a suitable format that is supported by the physical device
	*
	*/
	VkFormat vhDevFindDepthFormat(VkPhysicalDevice physicalDevice) {
		return vhDevFindSupportedFormat(physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}



	//-------------------------------------------------------------------------------------------------------
	/**
	*
	* \brief Create a logical device and according queues
	* \param[in] physicalDevice The physical device
	* \param[in] surface Window surface
	* \param[in] requiredDeviceExtensions List of required device extensions
	* \param[in] requiredValidationLayers List of required validation layers
	* \param[out] device The new logical device
	* \param[out] graphicsQueue A graphics queue into the device
	* \param[out] presentQueue A present queue into the device
	*
	*/
	void vhDevCreateLogicalDevice(	VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, 
									std::vector<const char*> requiredDeviceExtensions, 
									std::vector<const char*> requiredValidationLayers, 
									VkDevice *device, VkQueue *graphicsQueue, VkQueue *presentQueue) {
		QueueFamilyIndices indices = vhDevFindQueueFamilies(physicalDevice, surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.textureCompressionBC = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		createInfo.ppEnabledLayerNames = requiredValidationLayers.data();

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(*device, indices.graphicsFamily, 0, graphicsQueue);
		vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);

	}
}
