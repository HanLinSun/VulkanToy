#include <Vulkan/VulkanBaseRenderer.h>
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
    
        for (auto& modelGroup : scene->GetSceneModelGroups())
        {
            sum += modelGroup->GetModelSize();
        }
        return sum;
    }

    VulkanBaseRenderer::VulkanBaseRenderer(Window* windowptr)
    {
        m_window = reinterpret_cast<GLFWwindow*>(windowptr->GetNativeWindow());
        //glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

        ScreenWidth = windowptr->GetWidth();
        ScreenHeight = windowptr->GetHeight();

        const char* applicationName = "Vulkan Renderer";
        m_instance = new Instance(applicationName);
        CreateSurface();
        m_instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,VK_KHR_MAINTENANCE3_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, m_surface);

        m_device= m_instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit);
        m_swapChain = m_device->CreateSwapChain(m_surface, 3 , m_window);

        imageCount = m_swapChain->GetCount();
        //msaaSamples = GetMaxUsableSampleCount(m_instance->GetPhysicalDevice());
        m_Camera = new Camera(m_device, m_swapChain->GetVkExtent().width / m_swapChain->GetVkExtent().height);

    }
    void VulkanBaseRenderer::Run(Timestep deltaTime) 
    {
            UpdateCamera(deltaTime);
            DrawFrame();
    }


    void VulkanBaseRenderer::OnEvent(Event& e)
    {

    }

    void VulkanBaseRenderer::UpdateCamera(Timestep deltaTime)
    {
        if (Input::IsKeyPressed(APP_KEY_W))
        {
            m_Camera->UpdateTransform_Z(deltaTime.GetSeconds());
        }

        if (Input::IsKeyPressed(APP_KEY_S))
        {
            m_Camera->UpdateTransform_Z(-1 * deltaTime.GetSeconds());
        }

        if (Input::IsKeyPressed(APP_KEY_A))
        {
            m_Camera->UpdateTransform_X(-1*deltaTime.GetSeconds());
        }

        if (Input::IsKeyPressed(APP_KEY_D))
        {
            m_Camera->UpdateTransform_X(deltaTime.GetSeconds());
        }

        if (Input::IsKeyPressed(APP_KEY_Q))
        {
            m_Camera->UpdateTransform_Y(deltaTime.GetSeconds());
        }

        if (Input::IsKeyPressed(APP_KEY_E))
        {
            m_Camera->UpdateTransform_Y(-1*deltaTime.GetSeconds());
        }

        auto mousePosition = Input::GetMousePosition();
        if (Input::IsMouseButtonPressed(APP_MOUSE_BUTTON_LEFT))
        {
            double sensitivity = 0.2f;
            float deltaX = static_cast<float>(previousX - mousePosition.first)*sensitivity;
            float deltaY = static_cast<float>(previousY - mousePosition.second)*sensitivity;
           
            m_Camera->RotateAroundUpAxis(deltaX*deltaTime.GetSeconds());
            m_Camera->RotateAroundRightAxis(deltaY * deltaTime.GetSeconds());
            previousX = mousePosition.first;
            previousY = mousePosition.second;
        }
        m_Camera->UpdateViewMatrix();
        m_Camera->UpdateBufferMemory();
    }

    void VulkanBaseRenderer::InitGUILayerAttribute()
    {
        //ImGUI Layer(by default put it on top) 
        // need rewrite inthe future
        m_ImGuiLayer = reinterpret_cast<ImGuiLayer*>(m_layerStack->Top());
        
        VkExtent2D swapChainExtent = m_swapChain->GetVkExtent();
        VkFormat format = m_swapChain->GetVkImageFormat();
        m_ImGuiLayer->InitImGUIAttribute(m_device->GetVkDevice(), swapChainExtent, format, m_imageViews, m_swapChain->GetVkExtent().width, m_swapChain->GetVkExtent().height);
        m_ImGuiLayer->CreateImGuiDescriptorPool();
        m_ImGuiLayer->CreateImGuiRenderPass(m_swapChain->GetVkImageFormat());
        CreateCommandPool(&(m_ImGuiLayer->m_imGuiCommandPool));
        m_ImGuiLayer->CreateImGuiCommandBuffers();
        m_ImGuiLayer->CreateImGuiFramebuffer(m_imageViews);


        ImGui_ImplGlfw_InitForVulkan(m_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_instance->GetVkInstance();
        init_info.PhysicalDevice = m_instance->GetPhysicalDevice();
        init_info.Device = m_device->GetVkDevice();
        init_info.QueueFamily = m_device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Graphics];
        init_info.Queue = m_device->GetQueue(QueueFlags::Graphics);
        init_info.PipelineCache = VK_NULL_HANDLE;

        init_info.DescriptorPool = m_ImGuiLayer->m_imGuiDescriptorPool;
        init_info.RenderPass = m_ImGuiLayer->m_imGuiRenderPass;

        init_info.Subpass = 0;
        init_info.MinImageCount =imageCount;
        init_info.ImageCount =imageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }


    void VulkanBaseRenderer::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanBaseRenderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void VulkanBaseRenderer::CreateScene()
    {
        if (m_Scene == nullptr)
        {
            m_Scene = new Scene();
        }
    }
    void VulkanBaseRenderer::LoadModel(std::string model_path, std::string model_folder_path)
    {
        Loader loader(m_device, m_device->GetGraphicCommandPool());
        ModelGroup* modelgroup=new ModelGroup();
        loader.LoadModel(model_path, model_folder_path, modelgroup);
        m_Scene->AddModelGroup(modelgroup);
    }
    void VulkanBaseRenderer::InitVulkan() {

        SetupDebugMessenger();
        CreateScene();

        //By default we use this
        LoadModel(MODEL_PATH,MODEL_FILE_PATH);

        CreateRenderPass();
        CreateCameraDescriptorSetLayout();
        CreateModelDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool(&m_commandPool);
        CreateFrameResources();

        CreateDescriptorPool();
        CreateCameraDescriptorSets();
        
        CreateModelDescriptorSets(2);
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void VulkanBaseRenderer::Destroy()
    {
        vkDeviceWaitIdle(m_device->GetVkDevice());
        Cleanup();
    }
    void VulkanBaseRenderer::DestroyFrameResources() {
        for (size_t i = 0; i < m_imageViews.size(); i++)
        {
            vkDestroyImageView(m_device->GetVkDevice(), m_imageViews[i], nullptr);
        }

        vkDestroyImageView(m_device->GetVkDevice(), m_depthImageView, nullptr);
        vkFreeMemory(m_device->GetVkDevice(), m_depthImageMemory, nullptr);
        vkDestroyImage(m_device->GetVkDevice(), m_depthImage, nullptr);

        for (size_t i = 0; i < m_framebuffers.size(); i++) {
            vkDestroyFramebuffer(m_device->GetVkDevice(), m_framebuffers[i], nullptr);
        }
    }

    void VulkanBaseRenderer::Cleanup() {
        DestroyFrameResources();
    
        vkDestroyPipeline(m_device->GetVkDevice(), m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_graphicPipelineLayout, nullptr);
        vkDestroyRenderPass(m_device->GetVkDevice(), m_renderPass, nullptr);

        vkDestroyDescriptorPool(m_device->GetVkDevice(), m_descriptorPool, nullptr);


        vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_modelDescriptorSetLayout, nullptr);


        for (size_t i = 0; i < m_swapChain->GetCount(); i++) {
            vkDestroySemaphore(m_device->GetVkDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_device->GetVkDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_device->GetVkDevice(), m_inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_device->GetVkDevice(), m_commandPool, nullptr);
        m_ImGuiLayer->Destroy();

        vkDestroyDevice(m_device->GetVkDevice(), nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_instance->GetVkInstance(), m_debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_instance->GetVkInstance(), m_surface, nullptr);
        vkDestroyInstance(m_instance->GetVkInstance(), nullptr);

        glfwDestroyWindow(m_window);

        glfwTerminate();
    }
    void VulkanBaseRenderer::RecreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }
        ImGui_ImplVulkan_SetMinImageCount(imageCount);

        vkDeviceWaitIdle(m_device->GetVkDevice());

        DestroyFrameResources();

        m_swapChain->Recreate();

        CreateFrameResources();
     
        m_ImGuiLayer->CreateImGuiRenderPass(m_swapChain->GetVkImageFormat());
        m_ImGuiLayer->CreateImGuiFramebuffer(m_imageViews);
        m_ImGuiLayer->CreateImGuiCommandBuffers();
    }

    void VulkanBaseRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    void VulkanBaseRenderer::SetupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_instance->GetVkInstance(), &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void VulkanBaseRenderer::CreateSurface() {
        if (glfwCreateWindowSurface(m_instance->GetVkInstance(), m_window, nullptr, &m_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void VulkanBaseRenderer::CreateRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChain->GetVkImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkFormat depthFormat = m_device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
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

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;


        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_device->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    //GUI Pass
    void VulkanBaseRenderer::CreateCameraDescriptorSetLayout() {
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

    void VulkanBaseRenderer::CreateModelDescriptorSetLayout() {

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

    void VulkanBaseRenderer::CreateGraphicsPipeline() {
        auto vertShaderCode = readFile("./Shaders/VertexShader.spv");
        auto fragShaderCode = readFile("./Shaders/FragmentShader.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

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
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;

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

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_cameraDescriptorSetLayout, m_modelDescriptorSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(m_device->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_graphicPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_graphicPipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
       
        if (vkCreateGraphicsPipelines(m_device->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(m_device->GetVkDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(m_device->GetVkDevice(), vertShaderModule, nullptr);
    }

    void VulkanBaseRenderer::CreateFrameResources() {
        m_imageViews.resize(m_swapChain->GetCount());

        for (uint32_t i = 0; i < m_swapChain->GetCount(); i++) {
            m_imageViews[i] = Tools::CreateImageView(m_device,m_swapChain->GetVkImage(i), m_swapChain->GetVkImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }

  
        //Depth
        VkFormat depthFormat = m_device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        Tools::CreateImage(m_device, m_swapChain->GetVkExtent().width, m_swapChain->GetVkExtent().height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = Tools::CreateImageView(m_device, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        m_framebuffers.resize(m_imageViews.size());

        for (size_t i = 0; i < m_imageViews.size(); i++) {
            std::vector<VkImageView> attachments = {
                m_imageViews[i],
                m_depthImageView,
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
    void VulkanBaseRenderer::CreateCommandPool(VkCommandPool* commandPool) {
 
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Graphics];

        if (vkCreateCommandPool(m_device->GetVkDevice(), &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        //Compute command pool
    }

    bool VulkanBaseRenderer::HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    //Diffuse
    void VulkanBaseRenderer::CreateDescriptorPool() {

        int modelNum = GetSceneModelTotalSize(m_Scene);

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
        //// Max. number of descriptor sets that can be allocated from this pool (one per object)
        poolInfo.maxSets =1 + static_cast<uint32_t>(modelNum);

        if (vkCreateDescriptorPool(m_device->GetVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void VulkanBaseRenderer::CreateCameraDescriptorSets()
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

    void VulkanBaseRenderer::CreateModelDescriptorSets(int shaderBindingNums) 
    {
        //Need to use Model group here
        int sceneModelTotalSize = GetSceneModelTotalSize(m_Scene);

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

        std::vector<std::shared_ptr<ModelGroup>> scene_ModelGroup = m_Scene->GetSceneModelGroups();
        
        for (size_t i = 0; i < scene_ModelGroup.size(); i++)
        {
           ModelGroup* t_modelGroup =scene_ModelGroup[i].get();

            for (size_t j = 0; j < t_modelGroup->GetModelSize(); j++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = t_modelGroup->GetModelAt(j)->GetModelUniformBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(ModelBufferObject);

                Material* mat = t_modelGroup->GetModelAt(j)->GetMaterial();
                Texture* ambientTexture = mat->GetTexture(TextureType::Ambient).get();

                std::cout << "Material name: " << mat->m_name << std::endl;
      
                if (ambientTexture != nullptr) {
                    diffuse_imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    diffuse_imageInfo[j].imageView = ambientTexture->m_imageView;
                    diffuse_imageInfo[j].sampler = ambientTexture->m_sampler;
                    std::cout << "Texture file path: " << ambientTexture->fileName << std::endl;
                }
                else
                {
                    diffuse_imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    diffuse_imageInfo[j].imageView = VK_NULL_HANDLE;
                    VkSampler sampler = {};
                    Tools::CreateImageSampler(m_device, 1.0f, 0, sampler);
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

    void VulkanBaseRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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

    uint32_t VulkanBaseRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_instance->GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VulkanBaseRenderer::CreateCommandBuffers() {
        m_commandBuffers.resize(m_swapChain->GetCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_device->GetVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void VulkanBaseRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

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

        std::vector<std::shared_ptr<ModelGroup>> scene_ModelGroup = m_Scene->GetSceneModelGroups();

        for (size_t i = 0; i < scene_ModelGroup.size(); i++)
        {
           ModelGroup* modelGroup = scene_ModelGroup[i].get();

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
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void VulkanBaseRenderer::CreateSyncObjects() {
        m_imageAvailableSemaphores.resize(m_swapChain->GetCount());
        m_renderFinishedSemaphores.resize(m_swapChain->GetCount());
        m_inFlightFences.resize(m_swapChain->GetCount());

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_swapChain->GetCount(); i++) {
            if (vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device->GetVkDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void VulkanBaseRenderer::DrawFrame() {
        vkWaitForFences(m_device->GetVkDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        lastTimeStamp = std::chrono::high_resolution_clock::now();
        tPrevEnd = lastTimeStamp;

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device->GetVkDevice(), m_swapChain->GetVkSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_device->GetVkDevice(), 1, &m_inFlightFences[currentFrame]);

        vkResetCommandBuffer(m_commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        RecordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);

        m_ImGuiLayer->DrawUI(currentFrame, imageIndex);

        //m_Camera->UpdateBufferMemory();

        //After record new command buffer need to submit them
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::array<VkCommandBuffer, 2> submitCommandBuffers = { m_commandBuffers[currentFrame] , m_ImGuiLayer->m_imGuiCommandBuffers[currentFrame] };

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
        submitInfo.pCommandBuffers = submitCommandBuffers.data();

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_device->GetQueue(QueueFlags::Graphics), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_swapChain->GetVkSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_device->GetQueue(QueueFlags::Present), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanBaseRenderer::RecreateFrameResources() {
        vkDestroyPipeline(m_device->GetVkDevice(), m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_graphicPipelineLayout, nullptr);
        vkFreeCommandBuffers(m_device->GetVkDevice(), m_device->GetGraphicCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

        DestroyFrameResources();
        CreateFrameResources();
        CreateGraphicsPipeline();
       // RecordCommandBuffers();
    }

    VkShaderModule VulkanBaseRenderer::CreateShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_device->GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkPresentModeKHR VulkanBaseRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanBaseRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

    bool VulkanBaseRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    std::vector<const char*> VulkanBaseRenderer::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VulkanBaseRenderer::CheckValidationLayerSupport() {
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

    std::vector<char> VulkanBaseRenderer::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void VulkanBaseRenderer::SetLayerStack(LayerStack* in_layerStack) 
    {
        m_layerStack = in_layerStack;
    }
};


