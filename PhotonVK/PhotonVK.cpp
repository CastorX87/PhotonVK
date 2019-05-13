#include "VKUtil.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> REQ_VAL_LAYERS = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> REQ_EXTENSIONS = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report", "VK_EXT_debug_utils" };

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && computeFamily.has_value();
	}
};

class PhotonVK_Application
{
private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		void* msgType = pUserData;
		std::string msg(pCallbackData->pMessage);
		msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());

		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			PRINT_VK_VERBOSE(msg);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			PRINT_VK_INFO(msg);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			PRINT_VK_WARNING(msg);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			PRINT_VK_ERROR(msg);
			break;
		default:
			PRINT_APP_COMMENT(msg);
			break;
		}
		return VK_FALSE;
	}

public:
	void run() {
		PRINT("[ -------------------------------------------- ]");
		PRINT("[                   PhotonVK                   ]");
		PRINT("[ -------------------------------------------- ]");
		PRINT("");
		PRINT("[ -------------- Initialization -------------- ]");
		initWindow();
		initVulkan();
		PRINT("");
		PRINT("[ ----------------- Main Loop ---------------- ]");
		mainLoop();
		PRINT("");
		PRINT("[ ------------------ Cleanup ----------------- ]");
		cleanup();
	}

private:
	// ------------------------------------------------ //

	GLFWwindow* window;

	// ------------------------------------------------ //

	vk::Instance						vkInstance;
	vk::PhysicalDevice				vkPhysicalDevice;
	vk::Device							vkDevice;
	vk::Queue							vkGraphicsQueue;
	vk::DispatchLoaderDynamic		vkDispatcher;
	vk::DebugUtilsMessengerEXT		vkDebugMessenger;

	// ------------------------------------------------ //

	void createInstance()
	{
		// Validation layer check
		if (enableValidationLayers && CheckValidationLayerSupport() == false)
			throw new std::runtime_error("Required ValidationLayer(s) are not supported!");

		// Validation layer check
		if (CheckExtensionSupport() == false)
			throw new std::runtime_error("Required Extension(s) are not supported!");

		DEBUG_PRINT_VECTOR_DATA("Used Vulkan Extensions", REQ_EXTENSIONS);
		DEBUG_PRINT_VECTOR_DATA("Used Vulkan Validation Layers", REQ_VAL_LAYERS);

		// Fill required structs and create instance
		vk::ApplicationInfo ai{ "PhotonVK", 0, nullptr, 0, VK_API_VERSION_1_1 };
		vk::InstanceCreateInfo ci{ vk::InstanceCreateFlags(), &ai, (uint32_t)REQ_VAL_LAYERS.size(), REQ_VAL_LAYERS.data(), (uint32_t)REQ_EXTENSIONS.size(), REQ_EXTENSIONS.data() };
		vkInstance = vk::createInstance(ci);
	}

	void setupDispatcher()
	{
		vkDispatcher.init(vkInstance);
	}

	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		vkDebugMessenger = vkInstance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT
			{
				{},
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				debugCallback
			}, nullptr, vkDispatcher);

	}

	bool CheckValidationLayerSupport()
	{
		std::set<std::string> notSupportedLayers(REQ_VAL_LAYERS.begin(), REQ_VAL_LAYERS.end());

		auto supportedLayers = vk::enumerateInstanceLayerProperties();
		for (const auto& layer : supportedLayers)
		{
			auto found = notSupportedLayers.find(layer.layerName);
			if (found != notSupportedLayers.end())
				notSupportedLayers.erase(found);
		}
		return notSupportedLayers.size() == 0;
	}

	bool CheckExtensionSupport()
	{
		std::set<std::string> notSupportedExtensions(REQ_EXTENSIONS.begin(), REQ_EXTENSIONS.end());

		auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
		for (const auto& layer : supportedExtensions)
		{
			auto found = notSupportedExtensions.find(layer.extensionName);
			if (found != notSupportedExtensions.end())
				notSupportedExtensions.erase(found);
		}
		return notSupportedExtensions.size() == 0;
	}

	void pickPhysicalDevice() {
		auto physDevices = vkInstance.enumeratePhysicalDevices();
		
		if (physDevices.size() == 0)
			throw std::runtime_error("No Vulkan compatible devices found!");

		for (auto device : physDevices)
		{
			if (isDeviceSuitable(device))
			{
				vkPhysicalDevice = device;
				break;
			}
		}

		if ((VkPhysicalDevice)vkPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("No suitable physical device found!");
		}
	}

	bool isDeviceSuitable(const vk::PhysicalDevice& device) {
		auto devProperties = device.getProperties();
		auto devFeatures = device.getFeatures();

		bool isDiscreteGpu = devProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
		bool supportsQueues = findQueueFamilies(device).isComplete();

		return isDiscreteGpu && supportsQueues;
	}

	QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device)
	{
		QueueFamilyIndices indices;
		auto queueFamilyProps = device.getQueueFamilyProperties();
		for(int i = 0; i < queueFamilyProps.size(); i++)
		{
			if(queueFamilyProps[i].queueCount > 0 && (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics))
			{
				indices.graphicsFamily = i;
			}
			if (queueFamilyProps[i].queueCount > 0 && (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eCompute))
			{
				indices.computeFamily = i;
			}
			if (indices.isComplete())
				break;
		}

		return indices;
	}



	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);
		float prio = 1;
		vk::DeviceQueueCreateInfo dqci = vk::DeviceQueueCreateInfo().setQueueFamilyIndex(indices.graphicsFamily.value()).setQueueCount(1).setPQueuePriorities(&prio);
		
		vk::PhysicalDeviceFeatures pdf;
		
		vk::DeviceCreateInfo dci = vk::DeviceCreateInfo().setQueueCreateInfoCount(1).setPQueueCreateInfos(&dqci).setPEnabledFeatures(&pdf);
		dci.enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(REQ_VAL_LAYERS.size()) : 0;
		dci.ppEnabledLayerNames = enableValidationLayers ? REQ_VAL_LAYERS.data() : nullptr;

		vkDevice = vkPhysicalDevice.createDevice(dci);
		
		REGISTER_OBJ_NAME(vkDevice, VkDevice, vk::ObjectType::eDevice);

		vkGraphicsQueue = vkDevice.getQueue(indices.graphicsFamily.value(), 0);
	}

	// ------------------------------------------------ //

	void initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDispatcher();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		//vkDevice.destroy();
		vkInstance.destroyDebugUtilsMessengerEXT(vkDebugMessenger, nullptr, vkDispatcher);
		vkInstance.destroy();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
	
	// ------------------------------------------------ //
};

int main()
{
	PhotonVK_Application app;

	try
	{
		app.run();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}