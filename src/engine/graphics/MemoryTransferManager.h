#ifndef AE_MEMORYUPLOADMANAGER_H
#define AE_MEMORYUPLOADMANAGER_H

#include "Common.h"

#define VMA_STATS_STRING_ENABLED 0
#include <vk_mem_alloc.h>
#include <functional>

namespace Atlas {

    namespace Graphics {

        class GraphicsDevice;
        class MemoryManager;
        class Buffer;
        class Image;

        class MemoryTransferManager {

        public:
            MemoryTransferManager(GraphicsDevice* device, MemoryManager* memManager,
                uint32_t transferQueueFamilyIndex, VkQueue transferQueue);

            ~MemoryTransferManager();

            void UploadBufferData(void* data, Buffer* buffer, VkBufferCopy bufferCopyDesc);

            void UploadImageData(void* data, Image* image, VkOffset3D offset, VkExtent3D extent,
                uint32_t layerOffset, uint32_t layerCount);

            void RetrieveImageData(void* data, Image* image, VkOffset3D offset, VkExtent3D extent,
                uint32_t layerOffset, uint32_t layerCount, bool block = true);

            void GenerateMipMaps(Image* image);

            void GenerateMipMaps(Image* image, VkCommandBuffer cmd);

            void ImmediateSubmit(std::function<void(VkCommandBuffer)>&& function);

        private:
            struct StagingBufferAllocation {
                VkBuffer buffer;
                VmaAllocation allocation;
            };

            StagingBufferAllocation CreateStagingBuffer(size_t size);

            void DestroyStagingBuffer(StagingBufferAllocation& allocation);

            VkFence fence;
            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;

            uint32_t transferQueueFamilyIndex;
            VkQueue transferQueue;

            GraphicsDevice* device;
            MemoryManager* memoryManager;

        };

    }

}

#endif
