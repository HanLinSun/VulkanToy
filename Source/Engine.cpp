//Copyright 2024 Hanlin Sun

#include <Vulkan/Engine.h>
#include <WindowsPlatform.h>
#include <Loader.h>
#include <Tools.h>
#undef max

double previousX = 0.0;
double previousY = 0.0;



VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

namespace Renderer
{
     int GetSceneModelTotalSize(Scene* scene)
    {
        int sum = 0;
    
        for (auto& modelGroup : scene->GetSceneModelGroupsRaw())
        {
            sum += modelGroup->GetModelSize();
        }
        return sum;
    }

    Engine::Engine(Window* windowptr)
    {
        m_window = reinterpret_cast<GLFWwindow*>(windowptr->GetNativeWindow());
        //glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

        ScreenWidth = windowptr->GetWidth();
        ScreenHeight = windowptr->GetHeight();

        const char* applicationName = "Vulkan Renderer";
        m_instance = std::make_unique<Instance>(applicationName);
        CreateSurface();
        m_instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,VK_KHR_MAINTENANCE3_EXTENSION_NAME,VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, m_surface);

        m_device = m_instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit);
        m_swapChain = m_device->CreateSwapChain(m_surface, 3 , m_window);

        m_skyboxTexture = std::make_unique<TextureCubeMap>();
        imageCount = m_swapChain->GetCount();
        m_msaaSamples = GetMaxUsableSampleCount(m_instance->GetPhysicalDevice());

        m_RayTraceModule = std::make_unique<RayTraceModule>(m_device);

        m_Camera = std::make_shared<Camera>(m_device.get(), m_swapChain->GetVkExtent().width / m_swapChain->GetVkExtent().height);
        m_CameraController = std::make_shared<CameraController>(m_Camera);
        m_Scene =std::make_unique<Scene>(m_Camera);
        m_time = Timestep::GetInstance();

        m_runRaytracePipeline = true;
    }


    void Engine::OnEvent(Event& e)
    {

    }

    void Engine::Run() 
    {
            //m_Camera->Update();
            m_CameraController->Update();
            if (m_runRaytracePipeline)
            {
                m_RayTraceModule->UpdateUniformBuffer(m_Camera.get());
            }
            UpdateIOInput();
            DrawFrame();
    }

    void Engine::UpdateIOInput()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = Timestep::GetInstance()->GetSeconds();
        io.MouseDown[0] = false;
        io.MouseDown[1] = false;
        io.MouseDown[2] = false;
        io.MousePos = ImVec2(Input::GetMouseX(), Input::GetMouseY());

        if (Input::IsMouseButtonPressed(0))
        {
            io.MouseDown[0] = true;
        }
        if (Input::IsMouseButtonPressed(1))
        {
            io.MouseDown[1] = true;
        }
        if (Input::IsMouseButtonPressed(2))
        {
            io.MouseDown[2] = true;
        }
    }

    void Engine::LoadCubeMapTexture()
    {
        if (!m_skyboxTexture->TextureCubeMap::LoadFromFiles(cubeMapPaths, VK_FORMAT_R8G8B8A8_SRGB, m_device))
        {
            std::cout << "Loading Skybox texture failed" << std::endl;
        }
    }

    VkSampleCountFlagBits Engine::GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void Engine::InitGUILayerAttribute()
    {
        //ImGUI Layer(by default put it on top) 
        // need rewrite in future
        m_ImGuiLayer = reinterpret_cast<ImGuiLayer*>(m_layerStack->Top());
        
        VkExtent2D swapChainExtent = m_swapChain->GetVkExtent();
        VkFormat format = m_swapChain->GetVkImageFormat();

        m_ImGuiLayer->InitImGUIAttribute(m_device.get(), swapChainExtent, m_renderPass,m_presentQueue,"./Shaders/",m_msaaSamples);
    }


    void Engine::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Engine::LoadModel(std::string model_path, std::string model_folder_path)
    {
        Loader loader(m_device, m_device->GetGraphicCommandPool());
        std::unique_ptr<ModelGroup> modelgroup=std::make_unique<ModelGroup>();
        loader.LoadModel(model_path, model_folder_path, modelgroup.get());

        m_Scene->AddModelGroup(std::move(modelgroup));
    }

    void Engine::InitVulkan() {

        SetupDebugMessenger();

        //By default we use this
        if (!m_runRaytracePipeline)
        {
            LoadModel(MODEL_PATH, MODEL_FILE_PATH);
        }
   

        CreateRenderPass();
        CreateFrameResources();
        CreatePipelineCache();


        if (!m_runRaytracePipeline)
        {
            CreateDescriptorPool();
            //Graphic Render Pipeline
            CreateCameraDescriptorSetLayout();
            CreateCameraDescriptorSets();
            CreateModelDescriptorSetLayout();
            //Shader binding num is 2 by now and in future may need refractor
            CreateModelDescriptorSets(2);

        }
        else
        {
            //This function creates graphics part of descriptor layout and descriptor sets
            m_RayTraceModule->CreateRayTraceStorageImage(m_swapChain->GetVkExtent().width, m_swapChain->GetVkExtent().height);
            //This is a shared descriptor pool
            m_RayTraceModule->CreateDescriptorPool(m_descriptorPool);
            CreateRayTraceGraphicDescriptorResources();
            m_RayTraceModule->CreateUniformBuffer();
            m_RayTraceModule->CreateRayTracePipeline();
            m_rayTraceResource = m_RayTraceModule->GetRayTraceComputeResource();
        }

        CreateGraphicsPipeline();
        CreateSubmitInfo();
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void Engine::Destroy()
    {
        vkDeviceWaitIdle(m_device->GetVkDevice());
        Cleanup();
    }

    void Engine::DestroyFrameResources() {
        for (size_t i = 0; i < m_imageViews.size(); i++)
        {
            vkDestroyImageView(m_device->GetVkDevice(), m_imageViews[i], nullptr);
        }

        vkDestroyImageView(m_device->GetVkDevice(), m_depthImageView, nullptr);
        vkFreeMemory(m_device->GetVkDevice(), m_depthImageMemory, nullptr);
        vkDestroyImage(m_device->GetVkDevice(), m_depthImage, nullptr);

        vkDestroyImageView(m_device->GetVkDevice(), m_colorImageView, nullptr);
        vkDestroyImage(m_device->GetVkDevice(), m_colorImage, nullptr);
        vkFreeMemory(m_device->GetVkDevice(), m_colorImageMemory, nullptr);

        for (size_t i = 0; i < m_framebuffers.size(); i++) {
            vkDestroyFramebuffer(m_device->GetVkDevice(), m_framebuffers[i], nullptr);
        }
    }

    void Engine::Cleanup() {
        DestroyFrameResources();
    
        vkDestroyPipeline(m_device->GetVkDevice(), m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_graphicPipelineLayout, nullptr);
        vkDestroyRenderPass(m_device->GetVkDevice(), m_renderPass, nullptr);

        vkDestroyPipelineCache(m_device->GetVkDevice(), m_pipelineCache, nullptr);
        vkDestroyDescriptorPool(m_device->GetVkDevice(), m_descriptorPool, nullptr);
        if (!m_runRaytracePipeline)
        {
          
            vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_modelDescriptorSetLayout, nullptr);
            vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_cameraDescriptorSetLayout, nullptr);
        }
        else
        {
            vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_rayTraceGraphicsDescriptorLayout,nullptr);
        }


        vkDestroySemaphore(m_device->GetVkDevice(), m_Semaphores.presentComplete, nullptr);
        vkDestroySemaphore(m_device->GetVkDevice(), m_Semaphores.renderComplete, nullptr);

        if (m_runRaytracePipeline)
        {
            m_RayTraceModule->DestroyVKResources();
        }
        else
        {
            for (int i = 0; i < temp_samplers.size(); i++)
            {
                vkDestroySampler(m_device->GetVkDevice(), temp_samplers[i], nullptr);
            }
        }


        for (size_t i = 0; i < m_swapChain->GetCount(); i++) {

            vkDestroyFence(m_device->GetVkDevice(), m_waitFences[i], nullptr);
        }

        m_ImGuiLayer->Destroy();

        {
            m_Scene->DestroyVKResources();
            m_Scene->GetCamera()->DestroyVKResources();
            m_swapChain->DestroyVKResources();
        }
        vkDestroySurfaceKHR(m_instance->GetVkInstance(), m_surface, nullptr);

        m_device->DestroyVKResources();

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_instance->GetVkInstance(), m_debugMessenger, nullptr);
        }

        m_instance->DestroyVKResources();

        //delete[] m_swapChain;
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Engine::RecreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_device->GetVkDevice());

        DestroyFrameResources();

        m_swapChain->Recreate();

        CreateFrameResources();
    }

    void Engine::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void Engine::SetupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_instance->GetVkInstance(), &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void Engine::CreateSurface() {
        if (glfwCreateWindowSurface(m_instance->GetVkInstance(), m_window, nullptr, &m_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Engine::CreateRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChain->GetVkImageFormat();
        colorAttachment.samples = m_msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkFormat depthFormat = m_device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = m_msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = m_swapChain->GetVkImageFormat();
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = 0;

        dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstSubpass = 0;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].srcAccessMask = 0;
        dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = 0;

        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment,colorAttachmentResolve };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_device->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
 
    void Engine::CreateCameraDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = bindings;

        if (vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &layoutInfo, nullptr, &m_cameraDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void Engine::CreateModelDescriptorSetLayout() {

        VkDescriptorSetLayoutBinding layoutBindings[2] = {};
        // Binding 0: Uniform buffer
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // Binding 1: Image sampler (can be null)
        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { layoutBindings[0], layoutBindings[1]};


        // Define descriptor binding flags (allow partially bound descriptors)
        VkDescriptorBindingFlagsEXT bindingFlags[] = {
            0,  // No special flags for uniform buffer (binding 0)
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT  // Partially bound for image sampler (binding 1)
        };

        // Create a binding flags structure
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo = {};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        bindingFlagsInfo.bindingCount = 2;  // Number of bindings
        bindingFlagsInfo.pBindingFlags = bindingFlags;

        // Create the descriptor set layout
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); //uniform + image
        layoutInfo.pBindings = bindings.data();
        layoutInfo.pNext = &bindingFlagsInfo;

        if (vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &layoutInfo, nullptr, &m_modelDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout");
        }
    }

    void Engine::CreateGraphicsPipeline() {
        
        std::vector<char> vertShaderCode;
        std::vector<char> fragShaderCode;

        if (m_runRaytracePipeline)
        {
            vertShaderCode = Tools::ReadFile("./Shaders/showTexture.vert.spv");
            fragShaderCode = Tools::ReadFile("./Shaders/showTexture.frag.spv");
        }
        else
        {
            vertShaderCode = Tools::ReadFile("./Shaders/VertexShader.spv");
            fragShaderCode = Tools::ReadFile("./Shaders/FragmentShader.spv");
        }

        VkShaderModule vertShaderModule = Tools::CreateShaderModule(m_device.get(),vertShaderCode);
        VkShaderModule fragShaderModule = Tools::CreateShaderModule(m_device.get(),fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

        if (!m_runRaytracePipeline)
        {
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            auto bindingDescription = Vertex::GetBindingDescription();
            auto attributeDescriptions = Vertex::GetAttributeDescriptions();

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        }
        else
        {
            //Set vertex input to Empty
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        }
     
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
       // inputAssembly.flags = 0;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        if (!m_runRaytracePipeline)
        {
            descriptorSetLayouts = { m_cameraDescriptorSetLayout, m_modelDescriptorSetLayout };
        }
        else
        {
            descriptorSetLayouts = { m_rayTraceGraphicsDescriptorLayout };
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(m_device->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_graphicPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.sampleShadingEnable = VK_TRUE; // Set to VK_TRUE if using sample shading
        multisampleInfo.rasterizationSamples = m_msaaSamples; // Change to desired sample count
        //multisampleInfo.minSampleShading = .2f; // Adjust if sample shading is enabled
        //multisampleInfo.pSampleMask = nullptr; // Set if using a sample mask
        //multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Enable if needed
        //multisampleInfo.alphaToOneEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_graphicPipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(m_device->GetVkDevice(), m_pipelineCache, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(m_device->GetVkDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(m_device->GetVkDevice(), vertShaderModule, nullptr);
    }

    void Engine::CreatePipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        check_vk_result(vkCreatePipelineCache(m_device->GetVkDevice(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
    }

    void Engine::CreateSkyboxCubeMap(std::string cubeMap_texturePath)
    {

    }

    void Engine::CreateFrameResources() {
        m_imageViews.resize(m_swapChain->GetCount());

        for (uint32_t i = 0; i < m_swapChain->GetCount(); i++) {
            m_imageViews[i] = Tools::CreateImageView(m_device.get(), m_swapChain->GetVkImage(i), m_swapChain->GetVkImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }

        //Depth
        VkFormat depthFormat = m_device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        Tools::CreateImage(m_device.get(), m_swapChain->GetVkExtent().width, m_swapChain->GetVkExtent().height, 1, m_msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = Tools::CreateImageView(m_device.get(), m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        
        //MSAA Sample Color 
        VkFormat colorFormat = m_swapChain->GetVkImageFormat();
        Tools::CreateImage(m_device.get(), m_swapChain->GetVkExtent().width, m_swapChain->GetVkExtent().height, 1, m_msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorImage, m_colorImageMemory);
        m_colorImageView = Tools::CreateImageView(m_device.get(), m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);


        m_framebuffers.resize(m_imageViews.size());

        for (size_t i = 0; i < m_imageViews.size(); i++) {
            std::vector<VkImageView> attachments = {
                m_colorImageView,
                m_depthImageView,
                m_imageViews[i],
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapChain->GetVkExtent().width;
            framebufferInfo.height = m_swapChain->GetVkExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_device->GetVkDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void Engine::CreateGraphicsCommandPool(VkCommandPool* commandPool) {
 
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Graphics];

        if (vkCreateCommandPool(m_device->GetVkDevice(), &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }

    }

    bool Engine::HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    //Diffuse
    void Engine::CreateDescriptorPool() {

        int modelNum = GetSceneModelTotalSize(m_Scene.get());
        std::vector<VkDescriptorPoolSize> poolSizes=  {
            //Camera
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,1},
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , static_cast<uint32_t>(modelNum)},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,static_cast<uint32_t>(modelNum)},
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        // Max. number of descriptor sets that can be allocated from this pool (one per object)
        poolInfo.maxSets =1 + static_cast<uint32_t>(modelNum);

        if (vkCreateDescriptorPool(m_device->GetVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void Engine::CreateCameraDescriptorSets()
    {
        VkDescriptorSetLayout layouts[] = {m_cameraDescriptorSetLayout};
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        // Allocate descriptor sets
        if (vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, &m_cameraDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set");
        }

        // Configure the descriptors to refer to buffers
        VkDescriptorBufferInfo cameraBufferInfo = {};
        cameraBufferInfo.buffer = m_Camera->GetBuffer();
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraUniformBuffer);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_cameraDescriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pTexelBufferView = nullptr;

        // Update descriptor sets
        vkUpdateDescriptorSets(m_device->GetVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void Engine::CreateModelDescriptorSets(int shaderBindingNums) 
    {
        //Need to use Model group here
        int sceneModelTotalSize = GetSceneModelTotalSize(m_Scene.get());

        m_modelDescriptorSets.resize(sceneModelTotalSize);

        std::vector<VkDescriptorSetLayout> layouts = { m_modelDescriptorSets.size(), m_modelDescriptorSetLayout };
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_modelDescriptorSets.size());
        allocInfo.pSetLayouts = layouts.data();


        if (vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, m_modelDescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites(shaderBindingNums *m_modelDescriptorSets.size());
        std::vector<VkDescriptorImageInfo> diffuse_imageInfo(m_modelDescriptorSets.size());
        
        for (size_t i = 0; i < m_Scene->GetModelGroupSize(); i++)
        {
           const ModelGroup* t_modelGroup = m_Scene->GetSceneModelGroup(i);

            for (size_t j = 0; j < t_modelGroup->GetModelSize(); j++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = t_modelGroup->GetModelAt(j)->GetModelUniformBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(ModelBufferObject);

                Material* mat = t_modelGroup->GetModelAt(j)->GetMaterial();
                Texture* ambientTexture = mat->GetTexture(TextureType::Ambient).get();

                if (ambientTexture != nullptr) {
                    diffuse_imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    diffuse_imageInfo[j].imageView = ambientTexture->m_imageView;
                    diffuse_imageInfo[j].sampler = ambientTexture->m_sampler;
                 //   std::cout << "Texture file path: " << ambientTexture->fileName << std::endl;
                }
                else
                {
                    diffuse_imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    diffuse_imageInfo[j].imageView = VK_NULL_HANDLE;
                    VkSampler sampler = {};
                    Tools::CreateImageSampler(m_device.get(), 1.0f, 0, sampler);
                    temp_samplers.push_back(sampler);
                    diffuse_imageInfo[j].sampler = sampler;
                }
                                                               
               descriptorWrites[shaderBindingNums * j + 0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
               descriptorWrites[shaderBindingNums * j + 0].dstSet = m_modelDescriptorSets[j];
               descriptorWrites[shaderBindingNums * j + 0].dstBinding = 0;
               descriptorWrites[shaderBindingNums * j + 0].dstArrayElement = 0;
               descriptorWrites[shaderBindingNums * j + 0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
               descriptorWrites[shaderBindingNums * j + 0].descriptorCount = 1;
               descriptorWrites[shaderBindingNums * j + 0].pBufferInfo = &bufferInfo;
               
               descriptorWrites[shaderBindingNums * j + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
               descriptorWrites[shaderBindingNums * j + 1].dstSet = m_modelDescriptorSets[j];
               descriptorWrites[shaderBindingNums * j + 1].dstBinding = 1;
               descriptorWrites[shaderBindingNums * j + 1].dstArrayElement = 0;
               descriptorWrites[shaderBindingNums * j + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
               descriptorWrites[shaderBindingNums * j + 1].descriptorCount = 1;
               descriptorWrites[shaderBindingNums * j + 1].pImageInfo = &diffuse_imageInfo[j];
            }
        }
        vkUpdateDescriptorSets(m_device->GetVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //This function creates graphics part of descriptor layout and descriptor sets
    void Engine::CreateRayTraceGraphicDescriptorResources()
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
             VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        };

        VkDescriptorSetLayoutCreateInfo descriptorLayout = VulkanInitializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
        check_vk_result(vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &descriptorLayout, nullptr, &m_rayTraceGraphicsDescriptorLayout));

        VkDescriptorSetAllocateInfo allocInfo = VulkanInitializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_rayTraceGraphicsDescriptorLayout, 1);
        check_vk_result(vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, &m_rayTraceGraphicsDescriptorSet));

        auto storageImageDescriptor = m_RayTraceModule->GetStorageImage().descriptor;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
             VulkanInitializer::WriteDescriptorSet(m_rayTraceGraphicsDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &storageImageDescriptor)
        };
        vkUpdateDescriptorSets(m_device->GetVkDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }

    void Engine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device->GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device->GetVkDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device->GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_device->GetVkDevice(), buffer, bufferMemory, 0);
    }

    uint32_t Engine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_instance->GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Engine::CreateCommandBuffers() {
        m_commandBuffers.resize(m_swapChain->GetCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        allocInfo.commandPool = m_device->GetGraphicCommandPool();

        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_device->GetVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void Engine::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain->GetVkExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        m_ImGuiLayer->NewFrame();

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        // Bind the camera descriptor set. This is set 0 in all pipelines so it will be inherited
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipelineLayout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_swapChain->GetVkExtent().width;
        viewport.height = (float)m_swapChain->GetVkExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // When enable the VK_DYNAMIC_STATE_SCISSOR in your pipeline, 
        // Vulkan API expects you to explicitly set the scissor region using vkCmdSetScissor() before issuing any draw commands that rely on it.

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain->GetVkExtent();;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        //Drawing Model here -- enclose required(WIP)
        for (size_t i = 0; i < m_Scene->GetModelGroupSize(); i++)
        {
           const ModelGroup* modelGroup = m_Scene->GetSceneModelGroup(i);

            for (uint32_t j = 0; j <  modelGroup->GetModelSize(); j++)
            {
                VkBuffer vertexBuffers[] = { modelGroup->GetModelAt(j)->GetVertexBuffer()};
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffer, modelGroup->GetModelAt(j)->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipelineLayout, 1, 1, &m_modelDescriptorSets[j], 0, nullptr);

                std::vector<uint32_t> indices = modelGroup->GetModelAt(j)->GetIndices();
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            }
        }

        //Draw ImGUI here
        m_ImGuiLayer->DrawFrame(commandBuffer);

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void Engine::RecordRayTraceGraphicCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = VulkanInitializer::CommandBufferBeginInfo();

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain->GetVkExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        m_ImGuiLayer->NewFrame();

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = m_RayTraceModule->GetStorageImage().m_image;
        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        // Acquire barrier for graphics queue
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.srcQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Compute);
        imageMemoryBarrier.dstQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Graphics);
            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_swapChain->GetVkExtent().width;
        viewport.height = (float)m_swapChain->GetVkExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // When enable the VK_DYNAMIC_STATE_SCISSOR in your pipeline, 
        // Vulkan API expects you to explicitly set the scissor region using vkCmdSetScissor() before issuing any draw commands that rely on it.

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain->GetVkExtent();;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Display ray traced image generated by compute shader as a full screen quad
        // Quad vertices are generated in the vertex shader
        // Bind the camera descriptor set. This is set 0 in all pipelines so it will be inherited
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipelineLayout, 0, 1, &m_rayTraceGraphicsDescriptorSet, 0, nullptr);

        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

        m_ImGuiLayer->DrawFrame(commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.srcQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Graphics);
        imageMemoryBarrier.dstQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Compute);
        vkCmdPipelineBarrier(
           commandBuffer,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        check_vk_result(vkEndCommandBuffer(commandBuffer));
    }

    void Engine::CreateSyncObjects() {

        m_waitFences.resize(m_swapChain->GetCount());

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_swapChain->GetCount(); i++) {
            if (vkCreateFence(m_device->GetVkDevice(), &fenceInfo, nullptr, &m_waitFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create fence for a frame!");
            }
        }
    }

    VkResult Engine::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
    {
       return  vkAcquireNextImageKHR(m_device->GetVkDevice(), m_swapChain->GetVkSwapChain(), UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
    }

    void Engine::CreateSubmitInfo()
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializer::SemaphoreCreateInfo();
        // Create a semaphore used to synchronize image presentation
        // Ensures that the image is displayed before we start submitting new commands to the queue
        check_vk_result(vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphores.presentComplete));
        // Create a semaphore used to synchronize command submission
        // Ensures that the image is not presented until all commands have been submitted and executed
        check_vk_result(vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphores.renderComplete));

        // Set up submit info structure
        // Semaphores will stay the same during application lifetime

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        m_submitInfo = VulkanInitializer::SubmitInfo();
        m_submitInfo.pWaitDstStageMask = waitStages;
        m_submitInfo.waitSemaphoreCount = 1;
        m_submitInfo.pWaitSemaphores = &m_Semaphores.presentComplete;
        m_submitInfo.signalSemaphoreCount = 1;
        m_submitInfo.pSignalSemaphores = &m_Semaphores.renderComplete;
    }

    void Engine::DrawFrame() {
        vkWaitForFences(m_device->GetVkDevice(), 1, &m_waitFences[currentFrame], VK_TRUE, UINT64_MAX);
        lastTimeStamp = std::chrono::high_resolution_clock::now();
        tPrevEnd = lastTimeStamp;

        uint32_t imageIndex;

        VkResult result = AcquireNextImage(m_Semaphores.presentComplete, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }


        vkResetFences(m_device->GetVkDevice(), 1, &m_waitFences[currentFrame]);

        vkResetCommandBuffer(m_commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
  
        if (!m_runRaytracePipeline)
        {
            RecordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);
        }
        else
        {
            m_RayTraceModule->RecordComputeCommandBuffer();
            RecordRayTraceGraphicCommandBuffer(m_commandBuffers[currentFrame], imageIndex);
        }

        if (m_runRaytracePipeline)
        {
            vkWaitForFences(m_device->GetVkDevice(), 1, &m_rayTraceResource.fence, VK_TRUE, UINT64_MAX);
            vkResetFences(m_device->GetVkDevice(), 1, &m_rayTraceResource.fence);

            VkSubmitInfo computeSubmitInfo = VulkanInitializer::SubmitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &m_rayTraceResource.commandBuffer;
            check_vk_result(vkQueueSubmit(m_device->GetQueue(QueueFlags::Compute), 1, &computeSubmitInfo, m_rayTraceResource.fence));

            VkResult result = AcquireNextImage(m_Semaphores.presentComplete, &imageIndex);
        }


        m_submitInfo.commandBufferCount = 1;
        m_submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

        if (vkQueueSubmit(m_device->GetQueue(QueueFlags::Graphics), 1, &m_submitInfo, m_waitFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        result = m_swapChain->QueuePresent(m_device->GetQueue(QueueFlags::Present),imageIndex, m_Semaphores.renderComplete);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        check_vk_result(vkQueueWaitIdle(m_device->GetQueue(QueueFlags::Present)));

        currentFrame = (currentFrame + 1) % (m_swapChain->GetCount());
    }

    void Engine::RecreateFrameResources() {
        vkDestroyPipeline(m_device->GetVkDevice(), m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_graphicPipelineLayout, nullptr);
        vkFreeCommandBuffers(m_device->GetVkDevice(), m_device->GetGraphicCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

        DestroyFrameResources();
        CreateFrameResources();
        CreateGraphicsPipeline();
        if (m_runRaytracePipeline)
        {
            m_RayTraceModule->CreateRayTracePipeline();
        }
       // RecordCommandBuffers();
    }

    VkPresentModeKHR Engine::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Engine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    std::vector<const char*> Engine::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool Engine::CheckValidationLayerSupport() {
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

    void Engine::SetLayerStack(LayerStack* in_layerStack) 
    {
        m_layerStack = in_layerStack;
    }

};


