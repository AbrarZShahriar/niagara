// niagara.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <assert.h>
#include <stdio.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)


VkInstance createInstance()
{
	//Short In real vulkan use vkEmuermateInstanceVersion
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;
#ifdef _DEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
	createInfo.ppEnabledLayerNames = debugLayers;
	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);

#endif // _DEBUG

	const char* extentions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME
	};

	createInfo.ppEnabledExtensionNames = extentions;
	createInfo.enabledExtensionCount = sizeof(extentions) / sizeof(extentions[0]);

	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));
	return instance;
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice * physicalDevices, uint32_t physicalDeviceCount)
{
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			printf("Picking Discrete GPU %s\n", props.deviceName);
			return physicalDevices[i];
		}

	}
	if (physicalDeviceCount > 0)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[0], &props);
		printf("Picking fallback GPU %s\n", props.deviceName);
		return physicalDevices[0];
	}
	printf("No physical device/GPU found.");
	return VK_NULL_HANDLE; // vk null handlle is just 0
}

int main()
{
	int rc = glfwInit();
	assert(rc==GLFW_TRUE);

	VkInstance instance = createInstance();
	assert(instance);

	VkPhysicalDevice physicalDevices[16];
	uint32_t physicalDeviceCount = sizeof(physicalDevices)/sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

	
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);
	
	VkDevice device = 0;
	VK_CHECK(vkCreateDevice(physicalDevice, &VkDeviceCreateInfo , 0, &device));

	GLFWwindow* window = glfwCreateWindow(1024, 768, "niagara", 0, 0);
	assert(window);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

	
	}

	glfwDestroyWindow(window);

	vkDestroyInstance(instance, 0);
}