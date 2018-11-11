#include "MasterRenderer.h"

const char* MasterRenderer::shadowVertexPath = "shadowmapping.vsh";
const char* MasterRenderer::shadowFragmentPath = "shadowmapping.fsh";
const char* MasterRenderer::directionalLightVertexPath = "deferred/directional.vsh";
const char* MasterRenderer::directionalLightFragmentPath = "deferred/directional.fsh";
const char* MasterRenderer::skyboxVertexPath = "skybox.vsh";
const char* MasterRenderer::skyboxFragmentPath = "skybox.fsh";
const char* MasterRenderer::postProcessVertexPath = "postprocessing.vsh";
const char* MasterRenderer::postProcessFragmentPath = "postprocessing.fsh";

MasterRenderer::MasterRenderer() {

	rectangleVAO = GenerateRectangleVAO();

	geometryRenderer = new GeometryRenderer();

	shadowRenderer = new ShadowRenderer(shadowVertexPath, shadowFragmentPath);
	directionalLightRenderer = new DirectionalLightRenderer(directionalLightVertexPath, directionalLightFragmentPath);
	skyboxRenderer = new SkyboxRenderer(skyboxVertexPath, skyboxFragmentPath);
	postProcessRenderer = new PostProcessRenderer(postProcessVertexPath, postProcessFragmentPath);

}

void MasterRenderer::RenderScene(Window* window, RenderTarget* target, Camera* camera, Scene* scene) {

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	shadowRenderer->Render(window, target, camera, scene, true);

	target->geometryFramebuffer->Bind();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, target->geometryFramebuffer->width, target->geometryFramebuffer->height);

	geometryRenderer->Render(window, target, camera, scene, true);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	target->lightingFramebuffer->Bind();

	glBindVertexArray(rectangleVAO);

	directionalLightRenderer->Render(window, target, camera, scene, true);

	glEnable(GL_DEPTH_TEST);

	if (scene->sky->skybox != nullptr) {
		skyboxRenderer->Render(window, target, camera, scene, true);
	}

	target->lightingFramebuffer->Unbind();

	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(rectangleVAO);

	postProcessRenderer->Render(window, target, camera, scene, true);

	glBindVertexArray(0);

}

void MasterRenderer::RenderTexture(Texture* texture) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void MasterRenderer::RenderRectangle(vec3 color) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

uint32_t MasterRenderer::GenerateRectangleVAO() {

	uint32_t vao = 0;
	int8_t vertices[] = { -1, -1, 1, -1, -1, 1, 1, 1 };

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);

	uint32_t verticesBuffer;
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_BYTE, false, 0, NULL);

	return vao;

}