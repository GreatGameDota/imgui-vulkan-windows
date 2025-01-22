#include "VulkanWindow.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebufferResizeCallback2(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<VulkanWindow *>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized2 = true;
}

void VulkanWindow::initWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window2 = glfwCreateWindow(640, 480, "Vulkan2", nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window2, this);
    glfwSetFramebufferSizeCallback(m_Window2, framebufferResizeCallback2);
}

void VulkanWindow::initVulkan()
{
    if (glfwCreateWindowSurface(m_Instance, m_Window2, nullptr, &m_Surface2) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");

    createSwapChain2();
    createImageViews2();
    createRenderPass2();

    createDescriptorSetLayout2();
    createGraphicsPipeline2();
    createCommandPool(&m_CommandPool2);
    createDepthResources2();

    createFramebuffers2();
    createUniformBuffers2();
    createDescriptorPool2();
    createCommandBuffers2();
    createSyncObjects2();

    createTexture();
    updateIndexBuffer();
}

void VulkanWindow::drawFrame2()
{
    vkWaitForFences(m_Device, 1, &m_InFlightFences2[currentFrame2], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain2, UINT64_MAX, m_ImageAvailableSemaphores2[currentFrame2], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain2();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(m_Device, 1, &m_InFlightFences2[currentFrame2]);

    auto commandBuffer = m_CommandBuffers2[currentFrame2];
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass2;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers2[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChainExtent2;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // RENDER -----------------------------------------
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline2);

    VkBuffer vertexBuffers[] = {m_VertexBuffer2};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer2, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout2, 0, 1, &m_DescriptorSets2[currentFrame2], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    updateUniformBuffer2(currentFrame2);

    result = submitCommandBuffers(&commandBuffer, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized2)
    {
        m_FramebufferResized2 = false;
        recreateSwapChain2();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame2 = (currentFrame2 + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanWindow::cleanup()
{
    vkDeviceWaitIdle(m_Device);
    cleanupSwapChain2();
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout2, nullptr);

    vkDestroySampler(m_Device, m_TextureSampler, nullptr);
    vkDestroyImageView(m_Device, m_TextureImageView, nullptr);
    vkDestroyImage(m_Device, m_TextureImage, nullptr);
    vkFreeMemory(m_Device, m_TextureImageMemory, nullptr);

    vkDestroyBuffer(m_Device, m_IndexBuffer2, nullptr);
    vkFreeMemory(m_Device, m_IndexBufferMemory2, nullptr);

    vkDestroyBuffer(m_Device, m_VertexBuffer2, nullptr);
    vkFreeMemory(m_Device, m_VertexBufferMemory2, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores2[i], nullptr);
        vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores2[i], nullptr);
        vkDestroyFence(m_Device, m_InFlightFences2[i], nullptr);
    }

    vkDestroyCommandPool(m_Device, m_CommandPool2, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface2, nullptr);

    glfwDestroyWindow(m_Window2);
}
