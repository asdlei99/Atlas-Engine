#include "TerrainRenderer.h"

namespace Atlas {

    namespace Renderer {

        void TerrainRenderer::Init(Graphics::GraphicsDevice* device) {

            uniformBuffer = Buffer::UniformBuffer(sizeof(Uniforms));
            terrainMaterialBuffer = Buffer::UniformBuffer(sizeof(TerrainMaterial), 256);

        }

        void TerrainRenderer::Render(Viewport* viewport, RenderTarget* target, Camera* camera,
            Scene::Scene* scene, Graphics::CommandList* commandList,
            std::unordered_map<void*, uint16_t> materialMap) {

            if (!scene->terrain)
                return;

            Graphics::Profiler::BeginQuery("Terrain");

            auto terrain = scene->terrain;

            terrain->UpdateRenderlist(&camera->frustum, camera->GetLocation());

            std::vector<Terrain::TerrainNode*> detailNodes;
            std::vector<Terrain::TerrainNode*> distanceNodes;

            for (auto node : terrain->renderList) {
                if (node->cell->LoD >= terrain->LoDCount - terrain->detailNodeIdx)
                    detailNodes.push_back(node);
                else
                    distanceNodes.push_back(node);
            }

            auto materials = terrain->storage.GetMaterials();

            std::vector<TerrainMaterial> terrainMaterials(materials.size());

            for (size_t i = 0; i < materials.size(); i++) {
                if (materials[i]) {
                    terrainMaterials[i].idx = (uint32_t)materialMap[&materials[i]];
                    terrainMaterials[i].roughness = materials[i]->roughness;
                    terrainMaterials[i].metalness = materials[i]->metalness;
                    terrainMaterials[i].ao = materials[i]->ao;
                    terrainMaterials[i].displacementScale = materials[i]->displacementScale;
                    terrainMaterials[i].normalScale = materials[i]->normalScale;
                    terrainMaterials[i].tiling = materials[i]->tiling;
                }

            }

            terrainMaterialBuffer.SetData(terrainMaterials.data(), 0, terrainMaterials.size());

            terrain->vertexArray.Bind(commandList);

            terrain->storage.baseColorMaps.Bind(commandList, 3, 3);
            terrain->storage.roughnessMaps.Bind(commandList, 3, 4);
            terrain->storage.aoMaps.Bind(commandList, 3, 5);
            terrain->storage.normalMaps.Bind(commandList, 3, 6);
            terrain->storage.displacementMaps.Bind(commandList, 3, 7);

            Uniforms uniforms = {
                .heightScale = terrain->heightScale,
                .displacementDistance = terrain->displacementDistance,

                .tessellationFactor = terrain->tessellationFactor,
                .tessellationSlope = terrain->tessellationSlope,
                .tessellationShift = terrain->tessellationShift,
                .maxTessellationLevel = terrain->maxTessellationLevel
            };

            auto frustumPlanes = camera->frustum.GetPlanes();
            for (int32_t i = 0; i < 6; i++)
                uniforms.frustumPlanes[i] = frustumPlanes[i];

            uniformBuffer.SetData(&uniforms, 0, 1);

            terrainMaterialBuffer.Bind(commandList, 3, 8);
            uniformBuffer.Bind(commandList, 3, 9);

            for (uint8_t i = 0; i < 2; i++) {

                std::vector<Terrain::TerrainNode*> nodes;

                PipelineConfig config;
                switch (i) {
                    case 0: config = GeneratePipelineConfig(target, terrain, true);
                    Graphics::Profiler::BeginQuery("Detail");
                    nodes = detailNodes;
                    break;
                case 1: config = GeneratePipelineConfig(target, terrain, false);
                    Graphics::Profiler::BeginQuery("Distance");
                    nodes = distanceNodes;
                    break;
                default: break;
                }

                auto pipeline = PipelineManager::GetPipeline(config);
                commandList->BindPipeline(pipeline);

                for (auto node : nodes) {

                    node->cell->heightField.Bind(commandList, 3, 0);
                    node->cell->normalMap.Bind(commandList, 3, 1);
                    node->cell->splatMap.Bind(commandList, 3, 2);

                    auto tileScale = terrain->resolution * powf(2.0f,
                        (float)(terrain->LoDCount - node->cell->LoD) - 1.0f);

                    PushConstants constants = {
                        .nodeSideLength = node->sideLength,
                        .tileScale = tileScale,
                        .patchSize = float(terrain->patchSizeFactor),
                        .normalTexelSize = 1.0f / float(node->cell->normalMap.width),

                        .leftLoD = node->leftLoDStitch,
                        .topLoD = node->topLoDStitch,
                        .rightLoD = node->rightLoDStitch,
                        .bottomLoD = node->bottomLoDStitch,

                        .nodeLocation = node->location
                    };

                    commandList->PushConstants("constants", &constants);

                    commandList->Draw(terrain->patchVertexCount, 64);

                }

                Graphics::Profiler::EndQuery();

            }

            Graphics::Profiler::EndQuery();

        }

        PipelineConfig TerrainRenderer::GeneratePipelineConfig(RenderTarget *target,
            Ref<Terrain::Terrain>& terrain, bool detailConfig) {

            const auto shaderConfig = ShaderConfig {
                {"terrain/terrain.vsh", VK_SHADER_STAGE_VERTEX_BIT},
                {"terrain/terrain.tcsh", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
                {"terrain/terrain.tesh", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
                {"terrain/terrain.fsh", VK_SHADER_STAGE_FRAGMENT_BIT},
            };

            auto pipelineDesc = Graphics::GraphicsPipelineDesc {
                .frameBuffer = target->gBufferFrameBuffer,
                .vertexInputInfo = terrain->vertexArray.GetVertexInputState(),
            };

            pipelineDesc.assemblyInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            pipelineDesc.tessellationInfo.patchControlPoints = 4;
            pipelineDesc.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            pipelineDesc.rasterizer.polygonMode = terrain->wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;

            return detailConfig ? PipelineConfig(shaderConfig, pipelineDesc) :
                   PipelineConfig(shaderConfig, pipelineDesc, {"DISTANCE"});

        }

    }

}