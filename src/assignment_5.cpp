#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/mesh.h"
#include "mygl/geometry.h"
#include "mygl/camera.h"
#include "mygl/cube_map.h"

#include "planet.h"
#include "plane.h"

enum eCameraFollow
{
    PLANE,
    PLANET,
    PLANET_LOOK_AT_PLANE,
    NONE
};

/* enum for different render modes */
enum eRenderMode
{
    COLOR = 0, // render colors
    NORMAL,    // render normals
    MODE_COUNT
};

/* plane light directions */
const std::vector<Vector3D> planeLightDirs = {
    { 1.0f, 0.0f, 0.0f },  // left wing, red
    { 1.0f, 0.0f, 0.0f },  // left wing, white strobe
    {-1.0f, 0.0f, 0.0f },  // right wing, green
    {-1.0f, 0.0f, 0.0f },  // right wing, white strobe
    { 0.0f, 0.0f, 1.0f },  // rudder, white
    { 0.0f, 1.0f, 0.0f }   // rudder, red strobe
};

/* plane light positions */
const std::vector<Vector3D> planeLightPositions = {
    { 6.270f, 0.925f,  0.371f},  // left wing, red
    { 6.280f, 0.924f,  0.291f},  // left wing, white strobe
    {-6.270f, 0.925f,  0.371f},  // right wing, green
    {-6.280f, 0.924f,  0.291f},  // right wing, white strobe
    { 0.0f,   1.993f, -6.332f},  // rudder, white
    { 0.0f,   2.1f,   -5.703f}   // rudder, red strobe
};

struct SceneLight
{
    Vector3D lightPos;
    Vector3D globalAmbientLightColor;
    Vector3D lightColor;
    float ka;                       // ambient coefficient  [0, 1]
    float kd;                       // diffuse coefficient  [0, 1]
    float ks;                       // specular coefficient [0, 1]
};

/* struct holding all necessary state variables for scene */
struct
{
    /* camera */
    Camera camera;
    eCameraFollow cameraFollow;
    float zoomSpeedMultiplier;

    /* planet */
    Planet planet;

    /* plane */
    Plane plane;

    /* shader */
    eRenderMode renderMode;
    ShaderProgram shaderColor;
    ShaderProgram shaderNormal;
    ShaderProgram shaderFlagColor;
    ShaderProgram shaderFlagNormal;
    ShaderProgram skyboxShader;

    /* skybox modes */
    CubeMap starMapDay;
    CubeMap starMapNight;
    CubeMap skyMapDay;
    CubeMap skyMapNight;

    bool isDay;
    bool isSkyMapMode;

    SceneLight dayLight;
    SceneLight nightLight;
    bool PlaneLightsTurnON;
} sScene;

/* struct holding all state variables for input */
struct
{
    bool mouseLeftButtonPressed = false;
    Vector2D mousePressStart;
    bool keyPressed[Plane::eControl::CONTROL_COUNT] = {false, false, false, false};
} sInput;

/* GLFW callback function for keyboard events */
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /* close window on escape */
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        screenshotToPNG("screenshot.png");
    }

    /* input for camera control */
    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::NONE;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
        resetCameraRotation(sScene.camera);
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANE;
        sScene.camera.lookAt = sScene.plane.basePosition;
        sScene.camera.position = sScene.plane.basePosition + BASE_CAM_FOLLOW_OFFSET;
        resetCameraRotation(sScene.camera);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANET;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANET_LOOK_AT_PLANE;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
    }

    /* toggle render mode */
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        sScene.renderMode = static_cast<eRenderMode>((static_cast<int>(sScene.renderMode) + 1) % eRenderMode::MODE_COUNT);
        switch(sScene.renderMode)
        {
            case eRenderMode::COLOR:
                std::cout << "Render mode: COLOR" << std::endl;
                break;
            case eRenderMode::NORMAL:
                std::cout << "Render mode: NORMAL" << std::endl;
                break;
            default:
                std::cout << "Render mode: UNKNOWN" << std::endl;
        }
    }

    /* toggle between day and night time lighting; M for Mode */
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        sScene.isDay = !sScene.isDay;
    }

    /* toggle between sky map and star map*/
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
       sScene.isSkyMapMode = !sScene.isSkyMapMode;
    }

    /* input for plane control */
    if (key == GLFW_KEY_W)
    {
        sInput.keyPressed[Plane::eControl::FASTER] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S)
    {
        sInput.keyPressed[Plane::eControl::SLOWER] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_A)
    {
        sInput.keyPressed[Plane::eControl::LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D)
    {
        sInput.keyPressed[Plane::eControl::RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_SPACE)
    {
        sInput.keyPressed[Plane::eControl::UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_LEFT_CONTROL)
    {
        sInput.keyPressed[Plane::eControl::DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

/* GLFW callback function for mouse position events */
void mousePosCallback(GLFWwindow *window, double x, double y)
{
    /* called on cursor position change */
    if (sInput.mouseLeftButtonPressed)
    {
        Vector2D diff = sInput.mousePressStart - Vector2D(static_cast<float>(x), static_cast<float>(y));
        cameraUpdateOrbit(sScene.camera, diff, 0.0f);
        sInput.mousePressStart = Vector2D(static_cast<float>(x), static_cast<float>(y));
    }
}

/* GLFW callback function for mouse button events */
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        sInput.mouseLeftButtonPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(static_cast<float>(x), static_cast<float>(y));
    }
}

/* GLFW callback function for mouse scroll events */
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    cameraUpdateOrbit(sScene.camera, {0, 0}, -sScene.zoomSpeedMultiplier * static_cast<float>(yoffset));
}

/* GLFW callback function for window resize event */
void windowResizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    sScene.camera.width = static_cast<float>(width);
    sScene.camera.height = static_cast<float>(height);
}

/* function to setup and initialize the whole scene */
void sceneInit(float width, float height)
{
    /* initialize camera */
    sScene.camera = cameraCreate(width, height, BASE_FOV, 0.1f, 500.0f, sScene.plane.basePosition + BASE_CAM_FOLLOW_OFFSET, sScene.plane.basePosition);
    sScene.cameraFollow = eCameraFollow::PLANE;
    sScene.zoomSpeedMultiplier = 0.05f;

    /* setup objects in scene and create opengl buffers for meshes */
    sScene.plane = planeLoad("assets/plane/Cessna.obj", "assets/flag/flag_uibk_textured.obj");
    sScene.planet = planetLoad("assets/planet/earth-cartoon.obj");

    /* Skybox vertices and indices */
    std::vector<Vector3D> cubeVertices = {
            {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}
    };
    std::vector<unsigned int> cubeIndices = {
            0, 1, 2, 0, 2, 3,  // Front face
            5, 4, 7, 5, 7, 6,  // Back face
            4, 0, 3, 4, 3, 7,  // Left face
            1, 5, 6, 1, 6, 2,  // Right face
            3, 2, 6, 3, 6, 7,  // Top face
            4, 5, 1, 4, 1, 0   // Bottom face
    };

    sScene.starMapDay = cubeMapCreate(
            cubeVertices,
            cubeIndices,
            {
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_0.png",
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_1.png",
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_2.png",
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_3.png",
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_4.png",
                    "assets/skybox/hiptyc_2020_4k_gal_with_syferfontein_18d_clear_puresky_4k/n_5.png"
            }
    );

    sScene.starMapNight = sScene.starMapDay; // same texture for night

    sScene.skyMapDay = cubeMapCreate(
            cubeVertices,
            cubeIndices,
            {
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_0.png",
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_1.png",
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_2.png",
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_3.png",
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_4.png",
                    "assets/skybox/table_mountain_2_puresky_4k_mirrored/n_5.png"
            }
    );

    sScene.skyMapNight = sScene.skyMapDay;

    /* Create a light source for day and night */
    sScene.isDay = true;
    sScene.isSkyMapMode = true;

    sScene.dayLight.lightColor = Vector3D(1.0f, 0.9f, 0.8f);
    sScene.dayLight.lightPos = Vector3D(0.0f, 500.0f, 0.0f);
    sScene.dayLight.globalAmbientLightColor = Vector3D(182.0/255, 0.5f, 0.6f);
    sScene.dayLight.ka = 0.5f;
    sScene.dayLight.kd = 0.9f;
    sScene.dayLight.ks = 0.6f;

    sScene.nightLight.lightColor = Vector3D(0.9f, 0.5f, 0.2f);
    sScene.nightLight.lightPos = Vector3D(100.0f, 100.0f, 0.0f); // The position of the light source is lower at night -> less direct light angle hitting the planet's surface
    sScene.nightLight.globalAmbientLightColor = Vector3D(0.3, 0.9, 0.6);
    sScene.nightLight.ka = 0.1f;
    sScene.nightLight.kd = 0.3f;
    sScene.nightLight.ks = 0.2f;

    /* load shader from file */
    sScene.shaderColor = shaderLoad("shader/default.vert", "shader/color.frag");
    sScene.shaderNormal = shaderLoad("shader/default.vert", "shader/normal.frag");
    sScene.shaderFlagColor = shaderLoad("shader/flag.vert", "shader/flag.frag");
    sScene.shaderFlagNormal = shaderLoad("shader/flag.vert", "shader/normal.frag");
    sScene.skyboxShader = shaderLoad("shader/skybox.vert", "shader/skybox.frag");

    sScene.renderMode = eRenderMode::COLOR;
}

/* function to move and update objects in scene (e.g., rotate cube according to user input) */
void sceneUpdate(float dt)
{
    planeMove(sScene.plane, sInput.keyPressed, dt);
    planetRotate(sScene.planet, getPlaneTurningVector(sScene.plane), sScene.plane.speed, dt);

    if (sScene.cameraFollow == eCameraFollow::PLANE)
    {
        cameraFollow(sScene.camera, sScene.plane.position);

        /* change fov depending on speed */
        sScene.camera.fov = getSpeedFov(sScene.plane);
    }
    else if (sScene.cameraFollow == eCameraFollow::PLANET)
    {
        cameraFollow(sScene.camera, sScene.planet.position);
        setCameraRotation(sScene.camera, Matrix3D(sScene.planet.rotation));
    }
    else if (sScene.cameraFollow == eCameraFollow::PLANET_LOOK_AT_PLANE)
    {
        sScene.camera.lookAt = inverse(Matrix3D(sScene.planet.rotation)) * sScene.plane.position;
        setCameraRotation(sScene.camera, Matrix3D(sScene.planet.rotation));
    }
}

/* 
 * function to render all objects in the scene using their diffuse colors or their normals
 * (depending on shader program and renderNormal flag)
 */
void renderColor(ShaderProgram& shader, bool renderNormal) {
    /* setup camera and model matrices */
    Matrix4D proj = cameraProjection(sScene.camera);
    Matrix4D view = cameraView(sScene.camera);
    glUseProgram(shader.id);
    shaderUniform(shader, "uProj",  proj);
    shaderUniform(shader, "uView",  view);
    shaderUniform(shader, "uModel",  sScene.plane.transformation);

   if (!renderNormal) {
        shaderUniform(shader, "uCameraPos", cameraPosition(sScene.camera));

        const SceneLight& light = sScene.isDay ? sScene.dayLight : sScene.nightLight;

        shaderUniform(shader, "uLight.globalAmbientLightColor", light.globalAmbientLightColor);
        shaderUniform(shader, "uLight.lightColor", light.lightColor);
        shaderUniform(shader, "uLight.lightPos", light.lightPos);
        shaderUniform(shader, "uLight.ka", light.ka);
        shaderUniform(shader, "uLight.kd", light.kd);
        shaderUniform(shader, "uLight.ks", light.ks);
    }

    glActiveTexture(GL_TEXTURE6);
    if (sScene.isDay) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, sScene.isSkyMapMode ? sScene.skyMapDay.texture.id : sScene.starMapDay.texture.id);
    } else {
        glBindTexture(GL_TEXTURE_CUBE_MAP, sScene.isSkyMapMode ? sScene.skyMapNight.texture.id : sScene.starMapNight.texture.id);
    }
    shaderUniform(shader, "uCubeMap", 6);

    /* render plane */
    for(unsigned int i = 0; i < sScene.plane.partModel.size(); i++)
    {
        auto& model = sScene.plane.partModel[i];
        auto& transform = sScene.plane.partTransformations[i];

        glBindVertexArray(model.mesh.vao);

        shaderUniform(shader, "uModel", sScene.plane.transformation * transform);

        for(auto& material : model.material)
        {
            if (!renderNormal)
            {
                /* set material properties */
                shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
                shaderUniform(shader, "uMaterial.ambient", material.ambient);
                shaderUniform(shader, "uMaterial.specular", material.specular);
                shaderUniform(shader, "uMaterial.shininess", material.shininess);
                shaderUniform(shader, "uMaterial.emission", material.emission);
                /* Texture binding */
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.map_diffuse.id);
                shaderUniform(shader, "map_diffuse", 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.map_normal.id);
                shaderUniform(shader, "map_normal", 1);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material.map_ambient.id);
                shaderUniform(shader, "map_ambient", 2);

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material.map_emission.id);
                shaderUniform(shader, "map_emission", 3);

                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, material.map_shininess.id);
                shaderUniform(shader, "map_shininess", 4);

                bool hasSpecular = (i == 0);
                shaderUniform(shader, "hasSpecular", hasSpecular);

                if (hasSpecular) {
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, material.map_specular.id);
                    shaderUniform(shader, "map_specular", 5);
                }
            }
            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }
    }


    /* render planet */
    for(unsigned int i=0; i < sScene.planet.partModel.size(); i++)
    {
        auto& model = sScene.planet.partModel[i];
        glBindVertexArray(model.mesh.vao);

        shaderUniform(shader, "uModel", sScene.planet.transformation);

        for(auto& material : model.material)
        {
            if (!renderNormal)
            {
                /* set material properties */
                shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
                shaderUniform(shader, "uMaterial.ambient", material.ambient);
                shaderUniform(shader, "uMaterial.specular", material.specular);
                shaderUniform(shader, "uMaterial.shininess", material.shininess);
                shaderUniform(shader, "uMaterial.emission", material.emission);

                /* Texture binding */
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.map_diffuse.id);
                shaderUniform(shader, "map_diffuse", 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.map_normal.id);
                shaderUniform(shader, "map_normal", 1);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material.map_ambient.id);
                shaderUniform(shader, "map_ambient", 2);

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material.map_emission.id);
                shaderUniform(shader, "map_emission", 3);

                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, material.map_shininess.id);
                shaderUniform(shader, "map_shininess", 4);

                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, material.map_specular.id);
                shaderUniform(shader, "map_specular", 5);
            }
            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }
    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderFlag(ShaderProgram& shader, bool renderNormal) {
    Matrix4D proj = cameraProjection(sScene.camera);
    Matrix4D view = cameraView(sScene.camera);

    glUseProgram(shader.id);

    glActiveTexture(GL_TEXTURE6);
    if (sScene.isDay) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, sScene.isSkyMapMode ? sScene.skyMapDay.texture.id : sScene.starMapDay.texture.id);
    } else {
        glBindTexture(GL_TEXTURE_CUBE_MAP, sScene.isSkyMapMode ? sScene.skyMapNight.texture.id : sScene.starMapNight.texture.id);
    }
    shaderUniform(shader, "uCubeMap", 6);

    shaderUniform(shader, "uProj", proj);
    shaderUniform(shader, "uView", view);
    shaderUniform(shader, "uModel", sScene.plane.transformation *
                                   sScene.plane.flagModelMatrix *
                                   sScene.plane.flagNegativeRotation);

    if (!renderNormal) {
        shaderUniform(shader, "uCameraPos", cameraPosition(sScene.camera));

        const SceneLight& light = sScene.isDay ? sScene.dayLight : sScene.nightLight;

        shaderUniform(shader, "uLight.globalAmbientLightColor", light.globalAmbientLightColor);
        shaderUniform(shader, "uLight.lightColor", light.lightColor);
        shaderUniform(shader, "uLight.lightPos", light.lightPos);
        shaderUniform(shader, "uLight.ka", light.ka);
        shaderUniform(shader, "uLight.kd", light.kd);
        shaderUniform(shader, "uLight.ks", light.ks);
    }

    glBindVertexArray(sScene.plane.flag.model.mesh.vao);

    for (int i = 0; i < 3; ++i) {
        shaderUniform(shader, "amplitudes[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].amplitude);
        shaderUniform(shader, "phases[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].phi);
        shaderUniform(shader, "frequencies[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].omega);
        shaderUniform(shader, "directions[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].direction);
    }

    shaderUniform(shader, "zPosMin", sScene.plane.flag.minPosZ);
    shaderUniform(shader, "accumTime", sScene.plane.flagSim.accumTime);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, sScene.plane.flag.flag_displacement.id);
    shaderUniform(shader, "map_displacement", 6);
    shaderUniform(shader, "displacementScale", 0.1f);


    for (const auto& material : sScene.plane.flag.model.material) {
        if (!renderNormal) {
            shaderUniform(shader, "uMaterial.ambient", material.ambient);
            shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
            shaderUniform(shader, "uMaterial.specular", material.specular);
            shaderUniform(shader, "uMaterial.shininess", material.shininess);
            /* Texture binding */
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.map_diffuse.id);
            shaderUniform(shader, "map_diffuse", 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.map_normal.id);
            shaderUniform(shader, "map_normal", 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material.map_ambient.id);
            shaderUniform(shader, "map_ambient", 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, material.map_emission.id);
            shaderUniform(shader, "map_emission", 3);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, material.map_shininess.id);
            shaderUniform(shader, "map_shininess", 4);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, material.map_specular.id);
            shaderUniform(shader, "map_specular", 5);
        } else {
            shaderUniform(shader, "isFlag", true);
        }
        glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT,
                       (const void*)(material.indexOffset * sizeof(unsigned int)));
    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderSkybox(ShaderProgram& skyboxShader, CubeMap& skyMapDay, CubeMap& skyMapNight, CubeMap& starMapDay, CubeMap& starMapNight, Camera& camera, bool isDay, bool isSkyMapMode) {
    glDepthFunc(GL_LEQUAL); // Skybox in background
    glUseProgram(skyboxShader.id);

    Matrix4D proj = cameraProjection(camera);
    Matrix4D view = Matrix4D(Matrix3D(cameraView(camera)));

    // Choose the current map based on the mode and time of day
    CubeMap& currentMap = isSkyMapMode
                          ? (isDay ? skyMapDay : skyMapNight)
                          : (isDay ? starMapDay : starMapNight);

    Vector3D filterColor = isDay ? Vector3D(1.0f, 1.0f, 1.0f) : Vector3D(0.2f, 0.2f, 0.4f);

    // Uniforms for Shader
    shaderUniform(skyboxShader, "projection", proj);
    shaderUniform(skyboxShader, "view", view);
    shaderUniform(skyboxShader, "filterColor", filterColor);

    // Bind Skybox-VAO and Cube-Map-Texture
    glBindVertexArray(currentMap.mesh.vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, currentMap.texture.id);

    // Draw Skybox
    glDrawElements(GL_TRIANGLES, currentMap.mesh.size_ibo, GL_UNSIGNED_INT, 0);

    // Reset to OpenGL-States
    glBindVertexArray(0);
    glUseProgram(0);
    glDepthFunc(GL_LESS);
}

/* function to draw all objects in the scene */
void sceneDraw()
{
    /* clear framebuffer color */
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render skybox -------------*/
    renderSkybox(sScene.skyboxShader, sScene.skyMapDay, sScene.skyMapNight, sScene.starMapDay,sScene.starMapNight, sScene.camera, sScene.isDay, sScene.isSkyMapMode);

    /*------------ render scene -------------*/
    {
        if (sScene.renderMode == eRenderMode::COLOR)
        {
            renderColor(sScene.shaderColor, false);
            renderFlag(sScene.shaderFlagColor, false);
        }
        else if (sScene.renderMode == eRenderMode::NORMAL)
        {
            renderColor(sScene.shaderNormal, true);
            renderFlag(sScene.shaderFlagNormal, true);
        }
    }
    glCheckError();

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char **argv)
{
    /* create window/context */
    int width = 1280;
    int height = 720;
    GLFWwindow *window = windowCreate("Assignment 5 - Texturing", width, height);
    if (!window)
    {
        return EXIT_FAILURE;
    }

    /* set window callbacks */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);

    /* setup scene */
    sceneInit(static_cast<float>(width), static_cast<float>(height));

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;

    /* loop until user closes window */
    while (!glfwWindowShouldClose(window))
    {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update model matrix of cube */
        timeStampNew = glfwGetTime();
        sceneUpdate(static_cast<float>(timeStampNew - timeStamp));
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();

        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }

    /*-------- cleanup --------*/
    /* delete opengl shader and buffers */
    shaderDelete(sScene.shaderColor);
    shaderDelete(sScene.shaderNormal);
    planeDelete(sScene.plane);
    planetDelete(sScene.planet);

    /* cleanup glfw/glcontext */
    windowDelete(window);

    return EXIT_SUCCESS;
}
