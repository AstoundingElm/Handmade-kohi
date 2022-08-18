#pragma once

#include "../defines.h"
#include "../engine/vulkanTypes.h"
#include "../engine/rendererTypes.inl"
#include "vulkan_shader_utils.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/vulkan_buffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

KINLINE void vulkan_object_shader_update_global_state(vulkanContext * context, struct vulkan_object_shader * shader){

   u32 image_index = context->imageIndex;
    VkCommandBuffer command_buffer = context->graphicsCommandBuffers[image_index].handle;
    VkDescriptorSet global_descriptor = shader->global_descriptor_sets[image_index];

    // Bind the global descriptor set to be updated.
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &global_descriptor, 0, 0);

    // Configure the descriptors for the given index.
    u32 range = sizeof(global_uniform_object);
    u64 offset = 0;

    // Copy data to buffer
    vulkan_buffer_load_data(context, &shader->global_uniform_buffer, offset, range, 0, &shader->global_ubo);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->global_uniform_buffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    // Update descriptor sets.
    VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor_write.dstSet = shader->global_descriptor_sets[image_index];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->device.logicalDevice, 1, &descriptor_write, 0, 0);
    
};

KINLINE b8 vulkan_object_shader_create(vulkanContext * context,  vulkan_object_shader * out_shader){

     char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    
    for(u32 i  =0; i < OBJECT_SHADER_STAGE_COUNT; i++){

        if(!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, out_shader->stages)){

            KERROR("Unable to create %s shader module for '%s'", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;

        }
    }

    VkDescriptorSetLayoutBinding global_ubo_layout_binding;
    global_ubo_layout_binding.binding = 0;
    global_ubo_layout_binding.descriptorCount = 1;
    global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_ubo_layout_binding.pImmutableSamplers = 0;
    global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutCreateInfo global_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    global_layout_info.bindingCount = 1;
    global_layout_info.pBindings = &global_ubo_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logicalDevice, &global_layout_info, context->allocator, &out_shader->global_descriptor_set_layout));

    // Global descriptor pool: Used for global items such as view/projection matrix.
    VkDescriptorPoolSize global_pool_size;
    global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_pool_size.descriptorCount = context->swapchain.imageCount;

    VkDescriptorPoolCreateInfo global_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    global_pool_info.poolSizeCount = 1;
    global_pool_info.pPoolSizes = &global_pool_size;
    global_pool_info.maxSets = context->swapchain.imageCount;
    VK_CHECK(vkCreateDescriptorPool(context->device.logicalDevice, &global_pool_info, context->allocator, &out_shader->global_descriptor_pool));

    VkViewport viewport;
    viewport.x = 0.f;
    viewport.y = (f32)context->framebufferHeight;
    viewport.width = (f32)context->framebufferWidth;
    viewport.height = (f32)context->framebufferHeight;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebufferWidth;
    scissor.extent.height = context->framebufferHeight;

    u32 offset = 0;
    const i32 attribute_count =  1;

    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
    VkFormat formats[attribute_count] ={
    VK_FORMAT_R32G32B32_SFLOAT};
    u64 sizes[attribute_count] = {

        sizeof(vec3)
    };
    for(u32 i = 0; i < attribute_count; ++i){

        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
         attribute_descriptions[i].format = formats[i];
          attribute_descriptions[i].offset = offset;
          offset += sizes[i];

    };

    const i32 descriptor_set_layout_count = 1;
    VkDescriptorSetLayout layouts[1] = {out_shader->global_descriptor_set_layout};

     // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    kzeroMemory(stage_create_infos, sizeof(stage_create_infos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stage_create_infos[i].sType = out_shader->stages[i].shader_stage_create_info.sType;
        stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
    }

    if (!vulkan_graphics_pipeline_create(
            context,
            &context->mainRenderPass,
            attribute_count,
            attribute_descriptions,
            descriptor_set_layout_count,
            layouts,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false,
            &out_shader->pipeline)) {
        KERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

      if (!vulkan_buffer_create(
            context,
            sizeof(global_uniform_object),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &out_shader->global_uniform_buffer)) {
        KERROR("Vulkan buffer creation failed for object shader.");
        return false;
    }

    // Allocate global descriptor sets.
    VkDescriptorSetLayout global_layouts[3] = {
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout};

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = out_shader->global_descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logicalDevice, &alloc_info, out_shader->global_descriptor_sets));

    return true;

};

KINLINE void vulkan_object_shader_destroy(vulkanContext * context, struct vulkan_object_shader * shader){

    VkDevice logical_device = context->device.logicalDevice;

    vulkan_buffer_destroy(context, &shader->global_uniform_buffer);

    vulkan_pipeline_destroy(context, &shader->pipeline);

    vkDestroyDescriptorPool(logical_device, shader->global_descriptor_pool, context->allocator);

    vkDestroyDescriptorSetLayout(logical_device, shader->global_descriptor_set_layout, context->allocator);

for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){

    vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
    shader->stages[i].handle =0;
}

};

KINLINE void vulkan_object_shader_use(vulkanContext * context,struct  vulkan_object_shader * shader){

    u32 image_index = context->imageIndex;
    vulkan_pipeline_bind(&context->graphicsCommandBuffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
    
}