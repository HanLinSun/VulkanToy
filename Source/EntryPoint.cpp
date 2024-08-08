#include "Headers/Vulkan/VulkanBaseRenderer.h"
#include "Headers/RenderApplication.h"

int main() {
    //Renderer::VulkanBaseRenderer app;

        Renderer::RenderApplication render_app;
    try {
        render_app.Run();
        //app.run();
        //  app.application.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
