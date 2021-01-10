// BLAEngine Copyright (C) 2016-2020 Vincent Petrella. All rights reserved.
#pragma once

#include "StdInclude.h"
#include "System.h"
#include "System/Vulkan/Context.h"
#include "Rendering/RenderPass.h"

#define VK_CHECK_RESULT(x) x

namespace BLA
{
    VkViewport MakeViewport(float width, float height, float minDepth, float maxDepth);

    VkRect2D MakeRect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY);

    struct VulkanRenderPassInstance
    {
        const BaseRenderPassInstance* m_renderPassInstancePtr;

        VkDescriptorSet m_descriptorSets;
    };

    class VulkanRenderPass : public Gpu::RenderPassImplementation
    {
    protected:
        VkRenderPass m_vkRenderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_vkPipeline = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_vkDescriptorSetLayout = VK_NULL_HANDLE;
        blaVector<VulkanRenderPassInstance> m_currentInstances;
    
    public:

    void RegisterRenderPassInstance(const System::Vulkan::Context* vulkanInterface, const BaseRenderPassInstance& rpInstance);

        void CreatePipeline(
            Gpu::RenderPassDescriptor& renderPassDescriptor,
            VkDevice device,
            const VkAllocationCallbacks* allocator,
            VkPipelineCache pipelineCache,
            VkSampleCountFlagBits MSAASamples,
            VkShaderModule vertexModule,
            VkShaderModule fragmentModule);

        void CreateVKRenderPass(VkDevice device);

        void BuildCommandBuffers(blaVector<VulkanRenderPassInstance>& renderPassInstance,
                                 System::Vulkan::WindowInfo* vkWindow) const;
    };
}