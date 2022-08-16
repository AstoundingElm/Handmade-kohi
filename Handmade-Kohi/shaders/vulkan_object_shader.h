#pragma once

#include "../defines.h"
#include "../engine/vulkanTypes.h"
#include "../engine/rendererTypes.inl"
#include "vulkan_shader_utils.h"
#include "../engine/vulkan_pipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vulkan_object_shader_create(vulkanContext * context,  vulkan_object_shader * out_shader){

     char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    
    for(u32 i  =0; i < OBJECT_SHADER_STAGE_COUNT; i++){

        if(!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, out_shader->stages)){

            KERROR("Unable to create %s shader module for '%s'", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;

        }
    }

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
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false,
            &out_shader->pipeline)) {
        KERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }



    return true;

};

void vulkan_object_shader_destroy(vulkanContext * context, struct vulkan_object_shader * shader){

    vulkan_pipeline_destroy(context, &shader->pipeline);

for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){

    vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
    shader->stages[i].handle =0;
}

};

void vulkan_object_shader_use(vulkanContext * context,struct  vulkan_object_shader * shader){

    u32 image_index = context->imageIndex;
    vulkan_pipeline_bind(&context->graphicsCommandBuffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
    
}