// niagara.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define VK_USE_PLATFORM_WIN32_KHR
#define _DEBUG
#define _CONSOLE
#define _GLFW_WIN32

#define GLFW_EXPOSE_NATIVE_WIN32
#define CRT_SECURE_NO_WARNINGS



#include <cassert>
#include <cstdio>


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

    const char* extensions[] =
            {
                    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif // VK_Use

            };
   
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

    VkInstance instance = 0;
    VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));
    return instance;
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount)
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

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t* familyIndex)
{
    *familyIndex = 0; // shortcut

    float queuePriorities[] = { 1.0f };
    //vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, );
    VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };

    queueInfo.queueFamilyIndex = *familyIndex;//shortcut this needs to be computed from queue properties | this brings validation error
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    const char* extentions[] =
            {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };



    VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.ppEnabledExtensionNames = extentions;
    createInfo.enabledExtensionCount = sizeof(extentions) / sizeof(extentions[0]);


    VkDevice device = 0;
    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

    return device;
}



VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = GetModuleHandle(0);
	createInfo.hwnd = glfwGetWin32Window(window);
	VkSurfaceKHR surface = 0;
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface));
	return surface;

#else
#error Unsupported Platfom
#endif
}
VkFormat getSwapchainFormat(VkPhysicalDevice physicalDevice ,VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR formats[16];
    uint32_t formatCount=sizeof(formats)/sizeof(formats[0]);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,&formatCount,formats));

    assert(formatCount>0); // TODO driver bug
    return formats[0].format;

}


VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format,uint32_t width, uint32_t height)
{
    VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.surface = surface;
    createInfo.minImageCount = 2;
//    createInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;// Shortcut: some devices only support BGRA
    createInfo.imageFormat = format;

    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent.width = width;
    createInfo.imageExtent.height = height;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &familyIndex;
    createInfo.preTransform= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //shortcut?


    VkSwapchainKHR swapchain = 0;
    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

    return swapchain;
}

VkSemaphore createSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore semaphore = 0;
    VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));
    return semaphore;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t familyIndex)
{
    VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = familyIndex;
    VkCommandPool commandPool = 0;
    VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &commandPool));

    return commandPool;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat format)
{


    VkAttachmentDescription attachments[1] = {};
//    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].format = format;

    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttactments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };


    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttactments;

    VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    createInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;


    VkRenderPass renderPass = 0;
    VK_CHECK(
            vkCreateRenderPass(device, &createInfo, 0, &renderPass));

    return renderPass;
}

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView imageView
        , uint32_t width, uint32_t height)
{
    VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &imageView;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer = 0;
    VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));
    return framebuffer;
}


VkImageView createImageView(VkDevice device, VkImage image, VkFormat format)
{
    VkImageViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    createInfo.image = image;
    createInfo.viewType=VK_IMAGE_VIEW_TYPE_2D;
//    createInfo.format= VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.format= format;

    createInfo.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.levelCount=1;
    createInfo.subresourceRange.layerCount=1;


    VkImageView view=0;
    VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));

    return view;
}

VkShaderModule loadShader(VkDevice device, const char* path){
    FILE* file=fopen(path, "rb");
    assert(file);
    fseek(file, 0, SEEK_END);
    long length= ftell(file);
    assert(length>=0);
    fseek(file,0,SEEK_SET);

    char* buffer = new char[length];
    assert(buffer);

    size_t  rc= fread(buffer, 1, length, file);
    assert(rc==size_t(length));
    fclose(file);

    //assert(length%4==length);
    VkShaderModuleCreateInfo createInfo={VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize=length;
    createInfo.pCode= reinterpret_cast<const uint32_t*>(buffer);
    VkShaderModule shaderModule=0;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));
    return shaderModule;
}

VkPipelineLayout createPipelineLayout(VkDevice device){

    VkPipelineLayoutCreateInfo createInfo={VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    VkPipelineLayout layout=0;
    VK_CHECK(vkCreatePipelineLayout(device,&createInfo, 0, &layout));

    return layout;
}

VkPipeline createGraphicsPipeline(VkDevice device,VkPipelineCache pipelineCache, VkRenderPass renderPass,VkShaderModule vs,VkShaderModule fs, VkPipelineLayout layout){

    VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    VkPipelineShaderStageCreateInfo stages[2]={};
    stages[0].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage=VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module=vs;
    stages[0].pName="main";
    stages[1].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage=VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module=fs;
    stages[1].pName="main";

    createInfo.stageCount=sizeof(stages)/sizeof(stages[0]);
    createInfo.pStages=stages;

    VkPipelineVertexInputStateCreateInfo vertexInput= {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    createInfo.pVertexInputState=&vertexInput;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.pInputAssemblyState=&inputAssembly;

    VkPipelineViewportStateCreateInfo viewPortState= {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewPortState.viewportCount=1;
    viewPortState.scissorCount=1;
    createInfo.pViewportState=&viewPortState;

    VkPipelineRasterizationStateCreateInfo rasterizationState= {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationState.lineWidth= 1.f;
//    rasterizationState.polygonMode=VK_POLYGON_MODE_FILL;
    createInfo.pRasterizationState= &rasterizationState;

    VkPipelineMultisampleStateCreateInfo multisampleState ={VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
    createInfo.pMultisampleState=&multisampleState;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    createInfo.pDepthStencilState=&depthStencilState;

    VkPipelineColorBlendAttachmentState colorAttachmentState ={};
    colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendState.attachmentCount=1;
    colorBlendState.pAttachments=&colorAttachmentState;
    createInfo.pColorBlendState=&colorBlendState;

    VkDynamicState dynamicStates[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount=sizeof(dynamicStates)/sizeof(dynamicStates[0]);
    dynamicState.pDynamicStates=dynamicStates;
    createInfo.pDynamicState=&dynamicState;
    createInfo.layout=layout;
    createInfo.renderPass= renderPass;



    VkPipeline pipeline=0;
    VK_CHECK(vkCreateGraphicsPipelines(device,pipelineCache,1, &createInfo, 0, &pipeline));
    return pipeline;
}

int main()
{
    int rc = glfwInit();
    assert(rc == GLFW_TRUE);

    VkInstance instance = createInstance();
    assert(instance);

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));


    VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
    assert(physicalDevice);

    uint32_t familyIndex = 0;
    VkDevice device = createDevice(instance, physicalDevice, &familyIndex);
    assert(device);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "niagara", 0, 0);
    assert(window);


    VkSurfaceKHR surface = createSurface(instance, window);
    assert(surface);

    int windowWidth = 0, windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    VkFormat swapchainFormat = getSwapchainFormat(physicalDevice, surface);

    VkSwapchainKHR swapchain = createSwapchain(device, surface, familyIndex, swapchainFormat, windowWidth, windowHeight);
    assert(swapchain);

    VkSemaphore acquireSemaphore = createSemaphore(device);
    assert(acquireSemaphore);
    VkSemaphore releaseSemaphore = createSemaphore(device);
    assert(releaseSemaphore);

    VkQueue queue = 0;
    vkGetDeviceQueue(device, familyIndex, 0, &queue);

    VkRenderPass renderPass = createRenderPass(device, swapchainFormat);
    assert(renderPass);

    VkShaderModule triangleVS= loadShader(device, "C:/ClionVulkan/shaders/triangle.vert.spv");
    assert(triangleVS);
    VkShaderModule triangleFS= loadShader(device, "C:/ClionVulkan/shaders/triangle.frag.spv");
    assert(triangleFS);
    //TODO: this is critical for perf
    VkPipelineCache pipelineCache=0;

    VkPipelineLayout triangleLayout= createPipelineLayout(device);
    assert(triangleLayout);
    VkPipeline trianglePipeline= createGraphicsPipeline(device,pipelineCache, renderPass, triangleVS, triangleFS, triangleLayout);
    assert(trianglePipeline);




    VkImage swapchainImages[16];
    uint32_t swapchainImageCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages));
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, 0)); //for Intel bug

    VkImageView swapchainImageViews[16];
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        swapchainImageViews[i] = createImageView(device, swapchainImages[i], swapchainFormat);
        assert(swapchainImageViews);
    }

    VkFramebuffer swapchainFramebuffer[16];
    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        swapchainFramebuffer[i] = createFramebuffer(device, renderPass, swapchainImageViews[i], windowWidth,
                                                    windowHeight);
        assert(swapchainFramebuffer[i]);
    }
    VkCommandPool commandPool = createCommandPool(device, familyIndex);
    assert(commandPool);


    VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = 0;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        uint32_t imageIndex = 0;
        VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));


        VK_CHECK(vkResetCommandPool(device, commandPool, 0));

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        VkClearColorValue color = { 48.f/225.f,10.f/225.f,36.f/225.f,1 };
        VkClearValue clearColor={color};
        VkRenderPassBeginInfo passBeginInfo={VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        passBeginInfo.renderPass= renderPass;
        passBeginInfo.framebuffer=swapchainFramebuffer[imageIndex];
        passBeginInfo.renderArea.extent.width=windowWidth;
        passBeginInfo.renderArea.extent.height=windowHeight;
        passBeginInfo.clearValueCount=1;
        passBeginInfo.pClearValues= &clearColor;

        vkCmdBeginRenderPass(commandBuffer,&passBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport={0,float(windowHeight),float(windowWidth), -float(windowHeight), 0,1};
        VkRect2D scissor= {{0,0}, {static_cast<uint32_t>(windowWidth),static_cast<uint32_t>(windowHeight)}};


        vkCmdSetViewport(commandBuffer,0,1,&viewport);
        vkCmdSetScissor(commandBuffer, 0,1, &scissor);
        //// draw calls

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0 );


        vkCmdEndRenderPass(commandBuffer);

//        VkImageSubresourceRange range = {};
//        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        range.levelCount = 1;
//        range.layerCount = 1;
//
//        vkCmdClearColorImage(commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));

        VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &acquireSemaphore;
        submitInfo.pWaitDstStageMask = &submitStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &releaseSemaphore;


        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &releaseSemaphore;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;


        VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

        VK_CHECK(vkDeviceWaitIdle(device));
    }


    glfwDestroyWindow(window);

    vkDestroyInstance(instance, 0);
}