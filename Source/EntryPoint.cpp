#include "Headers/Vulkan/VulkanBaseRenderer.h"
#include "Headers/RenderApplication.h"
#include <Log.h>

int main() 
{
    Renderer::Log::Init();
 
    Renderer::RenderApplication render_app;
    try {
        render_app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
