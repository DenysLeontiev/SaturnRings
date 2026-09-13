#include "SaturnInstancing.h"
#include <GLFW/glfw3.h>

#include <math.h>

#include <iostream>
#include "../../Helpers.h"

SaturnInstancing::SaturnInstancing(Camera* camera) :
	camera(camera),
	skyColor(0.04f, 0.02f, 0.06f, 1.0f), // 0.02f, 0.02f, 0.05f, 1.0f
	planetShader("resources/shaders/planetInstancing/planetVertex.glsl", "resources/shaders/planetInstancing/planetFragment.glsl"),
	asteroidShader("resources/shaders/planetInstancing/asteroidInstancingVertex.glsl", "resources/shaders/planetInstancing/asteroidInstancingFragment.glsl"),
	planetModel("resources/models/planet/planet.obj"),
	asteroidModel("resources/models/rock/rock.obj"),
    asteroidsVBO(0),
    displaySkybox(true)
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

	modelMatrices = new glm::mat4[ASTEROID_INSTANCES];
}

SaturnInstancing::~SaturnInstancing() {
	delete[] modelMatrices;
}

void SaturnInstancing::OnAttach() {
    std::vector<std::string> faces {
		"resources/skybox/space/right.png",
		"resources/skybox/space/left.png",
		"resources/skybox/space/up.png",
		"resources/skybox/space/down.png",
		"resources/skybox/space/back.png",
		"resources/skybox/space/front.png"
	};
    skybox = new Skybox(faces);

    for (unsigned int i = 0; i < ASTEROID_INSTANCES; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        float angle = (float)i / (float)ASTEROID_INSTANCES * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * beltHeight;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
        model = glm::scale(model, glm::vec3(scale));

        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        modelMatrices[i] = model;
    }

    glGenBuffers(1, &asteroidsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, asteroidsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * ASTEROID_INSTANCES, &modelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < asteroidModel.meshes.size(); i++)
    {
        unsigned int VAO = asteroidModel.meshes[i].VAO;
        glBindVertexArray(VAO);

        std::size_t vec4Size = sizeof(glm::vec4);

        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);

        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);

        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);

        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
}

void SaturnInstancing::OnDetach() {
	glDeleteBuffers(1, &asteroidsVBO);

	delete skybox;
}

void SaturnInstancing::OnUpdate() {
    glClearColor(skyColor[0], skyColor[1], skyColor[2], skyColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    planetShader.use();

    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);

    glm::mat4 saturnSystem = glm::mat4(1.0f);
    saturnSystem = glm::translate(saturnSystem, glm::vec3(0.0f, -3.0f, 0.0f));
    saturnSystem = glm::rotate(saturnSystem, glm::radians(tiltInDegrees), glm::vec3(0.0f, 0.0f, 1.0f));

    planetShader.use();
    
    glm::mat4 planetMatrix = saturnSystem;
    planetMatrix = glm::rotate(planetMatrix, (float)(glfwGetTime()) * planetRotMultiplier, glm::vec3(0.0f, 1.0f, 0.0f));
    planetMatrix = glm::scale(planetMatrix, glm::vec3(4.0f, 4.0f, 4.0f));

    planetShader.setMat4("model", planetMatrix);
    planetShader.setMat4("view", view);
    planetShader.setMat4("projection", projection);
    planetModel.Draw(planetShader);

    asteroidShader.use();
    asteroidShader.setFloat("uTime", (float)glfwGetTime());
    asteroidShader.setMat4("view", view);
    asteroidShader.setMat4("projection", projection);

    glm::mat4 beltModel = saturnSystem;
    beltModel = glm::rotate(beltModel, (float)glfwGetTime() * asteroidsRotMultiplier, glm::vec3(0.0f, 1.0f, 0.0f));
    asteroidShader.setMat4("model", beltModel);
    
    asteroidShader.setInt("texture_diffuse1", 0);
    glActiveTexture(GL_TEXTURE0);
    if (!asteroidModel.textures_loaded.empty()) {
        glBindTexture(GL_TEXTURE_2D, asteroidModel.textures_loaded[0].id);
    }
    
    for (unsigned int i = 0; i < asteroidModel.meshes.size(); i++)
    {
        glBindVertexArray(asteroidModel.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, asteroidModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, ASTEROID_INSTANCES);
        glBindVertexArray(0);
    }

    if (displaySkybox) {
        skybox->Draw(view, projection);
    }
}

void SaturnInstancing::OnImGuiRender() {
    ImGui::Begin(GetName().c_str());

    ImGui::SliderFloat("Planet Rotation Speed", &planetRotMultiplier, 0.0f, 100.0f);
    ImGui::SliderFloat("Asteroids Rotation Speed", &asteroidsRotMultiplier, 0.0f, 100.0f);

    bool beltChanged = false;
    if (ImGui::SliderFloat("Belt height", &beltHeight, 0.0f, 10.0f)) beltChanged = true;
    if (ImGui::SliderFloat("Radius", &radius, 0.0f, 150.0f)) beltChanged = true;
    if (ImGui::SliderFloat("Offset", &offset, 0.0f, 50.0f)) beltChanged = true;

    ImGui::Checkbox("Display Skybox", &displaySkybox);

    if (beltChanged) {
        for (unsigned int i = 0; i < ASTEROID_INSTANCES; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            float angle = (float)i / (float)ASTEROID_INSTANCES * 360.0f;
            
            int maxOffset = (int)(2 * offset * 100);
            if (maxOffset <= 0) maxOffset = 1;

            float displacement = (rand() % maxOffset) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;
            displacement = (rand() % maxOffset) / 100.0f - offset;
            float y = displacement * beltHeight;
            displacement = (rand() % maxOffset) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;
            model = glm::translate(model, glm::vec3(x, y, z));

            float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
            model = glm::scale(model, glm::vec3(scale));

            float rotAngle = static_cast<float>((rand() % 360));
            model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

            modelMatrices[i] = model;
        }

        glBindBuffer(GL_ARRAY_BUFFER, asteroidsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * ASTEROID_INSTANCES, &modelMatrices[0], GL_DYNAMIC_DRAW);
    }

    ImGui::ColorEdit4("Sky", &skyColor[0]);

    ImGui::End();
}