#define VK_USE_PLATFORM_WIN32_KHR
#include "VKUtil.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> REQ_VAL_LAYERS = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_monitor", "VK_LAYER_GOOGLE_unique_objects" };
const std::vector<const char*> REQ_INST_EXTENSIONS = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report", "VK_EXT_debug_utils" };
const std::vector<const char*> REQ_DEV_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

const size_t NUM_REQ_QUEUE_FAMILIES = 3;
enum class QueueFamilyType
{
	Graphics,
	Presentation,
	Compute
};

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

std::map<QueueFamilyType, uint32_t> families;

class PhotonVK_Application
{
private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		void* msgType = pUserData;
		std::string msg(pCallbackData->pMessage);
		msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());

		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) PRINT_VK_ERROR(msg);
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) PRINT_VK_WARNING(msg);
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) PRINT_VK_INFO(msg);
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) PRINT_VK_VERBOSE(msg);

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
	vk::Queue							vkPresentationQueue;
	vk::Queue							vkComputeQueue;
	vk::DispatchLoaderDynamic		vkDispatcher;
	vk::DebugUtilsMessengerEXT		vkDebugMessenger;
	vk::SurfaceKHR						vkSurface;
	vk::SwapchainKHR					vkSwapChain;
	std::vector<vk::Image>			vkSwapChainImages;
	vk::Format							vkSwapChainImageFormat;
	vk::Extent2D						vkSwapChainExtent;
	std::vector<vk::ImageView>		vkSwapChainImageViews;
	vk::ShaderModule					vkVertShaderModule;
	vk::ShaderModule					vkFragShaderModule;
	vk::RenderPass						vkRenderPass;
	vk::PipelineLayout				vkPipelineLayout;
	vk::Pipeline						vkGraphicsPipeline;
	// ------------------------------------------------ //
	  
	void createSwapChainImageViews()
	{
		vkSwapChainImageViews.resize(vkSwapChainImages.size());

		vk::ImageViewCreateInfo ivci;
		ivci.viewType = vk::ImageViewType::e2D;
		ivci.format = vkSwapChainImageFormat;
		ivci.components = vk::ComponentMapping();
		ivci.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		for (size_t i = 0; i < vkSwapChainImages.size(); i++)
		{
			ivci.image = vkSwapChainImages[i];
			vkSwapChainImageViews[i] = vkDevice.createImageView(ivci);
		}
	}

	void createInstance()
	{
		// Validation layer check
		if (enableValidationLayers && CheckValidationLayerSupport() == false)
			throw new std::runtime_error("Required ValidationLayer(s) are not supported!");

		// Validation layer check
		if (CheckExtensionSupport() == false)
			throw new std::runtime_error("Required Extension(s) are not supported!");

		DEBUG_PRINT_VECTOR_DATA("Used Vulkan Extensions", REQ_INST_EXTENSIONS);
		DEBUG_PRINT_VECTOR_DATA("Used Vulkan Validation Layers", REQ_VAL_LAYERS);
		DEBUG_PRINT_VECTOR_DATA("Used Vulkan Device Extensions", REQ_DEV_EXTENSIONS);

		// Fill required structs and create instance
		vk::ApplicationInfo ai{ "PhotonVK", 0, nullptr, 0, VK_API_VERSION_1_1 };
		vk::InstanceCreateInfo ci{ vk::InstanceCreateFlags(), &ai, (uint32_t)REQ_VAL_LAYERS.size(), REQ_VAL_LAYERS.data(), (uint32_t)REQ_INST_EXTENSIONS.size(), REQ_INST_EXTENSIONS.data() };
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
		std::set<std::string> notSupportedExtensions(REQ_INST_EXTENSIONS.begin(), REQ_INST_EXTENSIONS.end());

		auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
		for (const auto& layer : supportedExtensions)
		{
			auto found = notSupportedExtensions.find(layer.extensionName);
			if (found != notSupportedExtensions.end())
				notSupportedExtensions.erase(found);
		}
		return notSupportedExtensions.size() == 0;
	}

	void pickPhysicalDevice()
	{
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

		if ((VkPhysicalDevice)vkPhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("No suitable physical device found!");
		}
	}

	bool isDeviceSuitable(const vk::PhysicalDevice & device)
	{
		auto devProperties = device.getProperties();
		auto devFeatures = device.getFeatures();

		bool isDiscreteGpu = devProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
		bool supportsQueues = findQueueFamilyIndices(device).size() == NUM_REQ_QUEUE_FAMILIES;
		bool supportsReqExt = checkDeviceExtensionSupport(device);
		bool supportsSwapChain = false;

		if (supportsReqExt)
		{
			auto swapChainSupport = querySwapChainSupport(device);
			supportsSwapChain = swapChainSupport.formats.empty() == false && swapChainSupport.presentModes.empty() == false;
		}

		return isDiscreteGpu && supportsQueues && supportsReqExt && supportsSwapChain;
	}

	bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
	{
		auto extensions = device.enumerateDeviceExtensionProperties(nullptr, vkDispatcher);
		std::set<std::string> remainingReqExtensions(REQ_DEV_EXTENSIONS.begin(), REQ_DEV_EXTENSIONS.end());
		
		for (auto ext : extensions) {
			if (remainingReqExtensions.find(std::string(ext.extensionName)) != remainingReqExtensions.end())
				remainingReqExtensions.erase(std::string(ext.extensionName));
		}
		return remainingReqExtensions.empty();
	}

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availModes)
	{
		if (STL_CONTAINS(availModes, vk::PresentModeKHR::eMailbox)) return vk::PresentModeKHR::eMailbox;
		//if (STL_CONTAINS(availModes, vk::PresentModeKHR::eImmediate)) return vk::PresentModeKHR::eImmediate;
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D actualExtent = { WIDTH, HEIGHT };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device)
	{
		SwapChainSupportDetails details;
		details.capabilities = device.getSurfaceCapabilitiesKHR(vkSurface, vkDispatcher);
		details.formats = device.getSurfaceFormatsKHR(vkSurface, vkDispatcher);
		details.presentModes = device.getSurfacePresentModesKHR(vkSurface, vkDispatcher);
		return details;
	}

	std::map<QueueFamilyType, uint32_t> findQueueFamilyIndices(const vk::PhysicalDevice & device)
	{
		std::map<QueueFamilyType, uint32_t> indices;
		auto queueFamilyProps = device.getQueueFamilyProperties();
		for (int i = 0; i < queueFamilyProps.size(); i++)
		{
			if (queueFamilyProps[i].queueCount > 0 && (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)) indices[QueueFamilyType::Graphics] = i;
			if (queueFamilyProps[i].queueCount > 0 && (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eCompute)) indices[QueueFamilyType::Compute] = i;
			if (queueFamilyProps[i].queueCount > 0 && device.getWin32PresentationSupportKHR(i, vkDispatcher)) indices[QueueFamilyType::Presentation] = i;
			if (indices.size() == NUM_REQ_QUEUE_FAMILIES) break;
		}
		return indices;
	}

	void createSwapChain() {
		
		// This function must be called otherwise an error will be reported by the validation layer!
		if (vkPhysicalDevice.getSurfaceSupportKHR(findQueueFamilyIndices(vkPhysicalDevice)[QueueFamilyType::Presentation], vkSurface, vkDispatcher) == false)
		{
			throw std::runtime_error("Surface not supported!");
		}

		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkPhysicalDevice);

		vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		
		vk::SwapchainCreateInfoKHR scci;
		scci.minImageCount = imageCount;
		scci.imageFormat = surfaceFormat.format;
		scci.imageColorSpace = surfaceFormat.colorSpace;
		scci.imageExtent = extent;
		scci.imageArrayLayers = 1;
		scci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		scci.surface = vkSurface;

		auto indices = findQueueFamilyIndices(vkPhysicalDevice);
		uint32_t indicesArr[] = { indices[QueueFamilyType::Graphics], indices[QueueFamilyType::Presentation] };
		if (indices[QueueFamilyType::Graphics] != indices[QueueFamilyType::Presentation])
		{
			scci.imageSharingMode = vk::SharingMode::eConcurrent;
			scci.queueFamilyIndexCount = 2;
			scci.pQueueFamilyIndices = indicesArr;
		}
		else
		{
			scci.imageSharingMode = vk::SharingMode::eExclusive;
			scci.queueFamilyIndexCount = 1;
			scci.pQueueFamilyIndices = nullptr;
		}
		scci.preTransform = swapChainSupport.capabilities.currentTransform;
		scci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		scci.presentMode = presentMode;
		scci.clipped = true;
		scci.oldSwapchain = nullptr;

		vkSwapChain = vkDevice.createSwapchainKHR(scci, nullptr, vkDispatcher);

		vkSwapChainImages = vkDevice.getSwapchainImagesKHR(vkSwapChain, vkDispatcher);
		vkSwapChainImageFormat = surfaceFormat.format;
		vkSwapChainExtent = extent;
	}

	void createLogicalDevice()
	{
		auto famIndices = findQueueFamilyIndices(vkPhysicalDevice);

		std::set<uint32_t> uniFamIndices;
		std::transform(famIndices.begin(), famIndices.end(), std::inserter(uniFamIndices, uniFamIndices.begin()), [](auto x) { return x.second; });

		float prio = 1;

		std::vector<vk::DeviceQueueCreateInfo> dqci_arr;
		for (auto& queueFamilyIndex : uniFamIndices)
		{
			dqci_arr.push_back(vk::DeviceQueueCreateInfo().setQueueFamilyIndex(queueFamilyIndex).setQueueCount(1).setPQueuePriorities(&prio));
		}
		vk::PhysicalDeviceFeatures pdf;
		vk::DeviceCreateInfo dci = vk::DeviceCreateInfo().setQueueCreateInfoCount((uint32_t)dqci_arr.size()).setPQueueCreateInfos(dqci_arr.data()).setPEnabledFeatures(&pdf);
		dci.enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(REQ_VAL_LAYERS.size()) : 0;
		dci.ppEnabledLayerNames = enableValidationLayers ? REQ_VAL_LAYERS.data() : nullptr;
		dci.enabledExtensionCount = (uint32_t)REQ_DEV_EXTENSIONS.size();
		dci.ppEnabledExtensionNames = REQ_DEV_EXTENSIONS.data();

		vkDevice = vkPhysicalDevice.createDevice(dci);
		REGISTER_OBJ_NAME(vkDevice, VkDevice, vk::ObjectType::eDevice);

		vkGraphicsQueue     = vkDevice.getQueue(famIndices[QueueFamilyType::Graphics], 0);
		vkPresentationQueue = vkDevice.getQueue(famIndices[QueueFamilyType::Presentation], 0);
		vkComputeQueue      = vkDevice.getQueue(famIndices[QueueFamilyType::Compute], 0);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface((VkInstance)vkInstance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&vkSurface)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// ------------------------------------------------ //

	void initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "PhotonVK", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDispatcher();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createSwapChainImageViews();
		createRenderPass();
		createGraphicsPipeline();
	}

	void createRenderPass()
	{
		vk::AttachmentDescription color_attach_descr;
		color_attach_descr.format = vkSwapChainImageFormat;
		color_attach_descr.samples = vk::SampleCountFlagBits::e1;
		color_attach_descr.loadOp = vk::AttachmentLoadOp::eClear;
		color_attach_descr.storeOp = vk::AttachmentStoreOp::eStore;
		color_attach_descr.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		color_attach_descr.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		color_attach_descr.initialLayout = vk::ImageLayout::eUndefined;
		color_attach_descr.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference color_attach_ref;
		color_attach_ref.attachment = 0;
		color_attach_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription color_subpass_descr;
		color_subpass_descr.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		color_subpass_descr.colorAttachmentCount = 1;
		color_subpass_descr.pColorAttachments = &color_attach_ref;

		vk::RenderPassCreateInfo renderpass_ci;
		renderpass_ci.attachmentCount = 1;
		renderpass_ci.pAttachments = &color_attach_descr;
		renderpass_ci.subpassCount = 1;
		renderpass_ci.pSubpasses = &color_subpass_descr;

		vkRenderPass = vkDevice.createRenderPass(renderpass_ci);
	}

	void createGraphicsPipeline()
	{
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		vkVertShaderModule = createShaderModule(vertShaderCode);
		vkFragShaderModule= createShaderModule(fragShaderCode);

		vk::PipelineShaderStageCreateInfo pssciVS = {};
		pssciVS.setModule(vkVertShaderModule);
		pssciVS.setStage(vk::ShaderStageFlagBits::eVertex);
		pssciVS.setPName("main");

		vk::PipelineShaderStageCreateInfo pssciFS = {};
		pssciFS.setModule(vkFragShaderModule);
		pssciFS.setStage(vk::ShaderStageFlagBits::eFragment);
		pssciFS.setPName("main");

		vk::PipelineShaderStageCreateInfo pssciArr[] = { pssciVS, pssciFS };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssembly.primitiveRestartEnable = false;

		vk::Viewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)vkSwapChainExtent.width;
		viewport.height = (float)vkSwapChainExtent.height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		vk::Rect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent = vkSwapChainExtent;

		vk::PipelineViewportStateCreateInfo pvstci;
		pvstci.viewportCount = 1;
		pvstci.pViewports = &viewport;
		pvstci.scissorCount = 1;
		pvstci.pScissors = &scissor;

		vk::PipelineRasterizationStateCreateInfo prsci;
		prsci.depthClampEnable = false;
		prsci.rasterizerDiscardEnable = false;
		prsci.polygonMode = vk::PolygonMode::eFill;
		prsci.lineWidth = 1;
		prsci.cullMode = vk::CullModeFlagBits::eBack;
		prsci.frontFace = vk::FrontFace::eClockwise;
		prsci.depthBiasEnable = false;

		vk::PipelineMultisampleStateCreateInfo pmsci;
		pmsci.sampleShadingEnable = false;
		pmsci.rasterizationSamples = vk::SampleCountFlagBits::e1;

		vk::PipelineColorBlendAttachmentState cba;
		cba.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		cba.blendEnable = false;

		vk::PipelineColorBlendStateCreateInfo colorBlending;
		colorBlending.logicOpEnable = false;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &cba;
		colorBlending.blendConstants[0] = 0;
		colorBlending.blendConstants[1] = 0;
		colorBlending.blendConstants[2] = 0;
		colorBlending.blendConstants[3] = 0;

		vk::PipelineLayoutCreateInfo plci;
		plci.setLayoutCount = 0;
		plci.pushConstantRangeCount = 0;

		vkPipelineLayout = vkDevice.createPipelineLayout(plci);

		vk::GraphicsPipelineCreateInfo gpci;
		gpci.stageCount = 2;
		gpci.pStages = pssciArr;
		gpci.pVertexInputState = &vertexInputInfo;
		gpci.pInputAssemblyState = &inputAssembly;
		gpci.pViewportState = &pvstci;
		gpci.pRasterizationState = &prsci;
		gpci.pMultisampleState = &pmsci;
		gpci.pColorBlendState = &colorBlending;
		gpci.layout = vkPipelineLayout;
		gpci.renderPass = vkRenderPass;
		gpci.subpass = 0;
		gpci.basePipelineHandle = vk::Pipeline();

		vkGraphicsPipeline = vkDevice.createGraphicsPipeline(vk::PipelineCache(), gpci);

		vkDevice.destroy(vkVertShaderModule);
		vkDevice.destroy(vkFragShaderModule);
	}

	vk::ShaderModule createShaderModule(const std::vector<char>& code)
	{
		vk::ShaderModuleCreateInfo smci;
		smci.codeSize = code.size();
		smci.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return vkDevice.createShaderModule(smci);
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
		vkDevice.destroyPipeline(vkGraphicsPipeline);
		vkDevice.destroyPipelineLayout(vkPipelineLayout);
		vkDevice.destroyRenderPass(vkRenderPass);

		for(auto iv : vkSwapChainImageViews) { vkDevice.destroyImageView(iv); }

		vkDevice.destroySwapchainKHR(vkSwapChain);
		vkDevice.destroy();
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
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}