#include "plane.h"

#include <stdexcept>
#include <iostream>

Plane planeLoad(const std::string& planeFilePath, const std::string& flagFilePath)
{
    std::vector<Model> models = modelLoad(planeFilePath);

    if(models.size() != Plane::ePart::PART_COUNT)
    {
        throw std::runtime_error("[Plane] number of parts do not match!" + std::to_string(models.size()));
    }

    Plane plane;
    plane.partModel.resize(models.size());
    plane.partTransformations.resize(Plane::ePart::PART_COUNT, Matrix4D::identity());
    plane.position = plane.basePosition;
    /*
    plane.noEmissionTexture = textureLoad("assets/plane/textures/Cessna_Body_Albedo.png");
    plane.body_ao = textureLoad("assets/plane/textures/Cessna_Body_AO.png");
    plane.body_emission = textureLoad("assets/plane/textures/Cessna_Body_Emission.png");
    plane.body_glossy = textureLoad("assets/plane/textures/Cessna_Body_Glossy.png");
    plane.body_normals = textureLoad("assets/plane/textures/Cessna_Body_Normals.png");
    plane.body_specular_color = textureLoad("assets/plane/textures/Cessna_Body_Specular_Color.png");
    */

    plane.noEmissionTexture = textureCreateSingleColor(1, 1, {0.0f, 0.0f, 0.0f});

    for(const auto& obj : models)
    {
        if (obj.name == "Hull") {
        plane.partModel[Plane::HULL] = obj;
            if (!plane.partModel[Plane::HULL].material.empty()) {
                for (auto& material : plane.partModel[Plane::HULL].material) {
                    material.map_diffuse = textureLoad("assets/plane/textures/Cessna_Body_Albedo.png");
                    material.map_normal = textureLoad("assets/plane/textures/Cessna_Body_Normals.png");
                    material.map_specular = textureLoad("assets/plane/textures/Cessna_Body_Specular_Color.png");
                    material.map_ambient = textureLoad("assets/plane/textures/Cessna_Body_AO.png");
                    material.map_emission = textureLoad("assets/plane/textures/Cessna_Body_Emission.png");
                    material.map_shininess = textureLoad("assets/plane/textures/Cessna_Body_Glossy.png");
                }
            } else {
                std::cerr << "[Error] No materials found in HULL part!" << std::endl;
            }
        }
        else if(obj.name == "Glass") {
            plane.partModel[Plane::WINDOWS] = obj;
            if (!plane.partModel[Plane::WINDOWS].material.empty()) {
                for (auto& material : plane.partModel[Plane::WINDOWS].material) {
                    material.map_diffuse = textureLoad("assets/plane/textures/Cessna_Glass_Albedo.png");
                    material.map_normal = textureLoad("assets/plane/textures/Cessna_Glass_Normals.png");
                    material.map_ambient = textureLoad("assets/plane/textures/Cessna_Glass_AO.png");
                    material.map_emission = textureLoad("assets/plane/textures/Cessna_Glass_Emission.png");
                    material.map_shininess = textureLoad("assets/plane/textures/Cessna_Glass_Glossy.png");
                }
            } else {
                std::cerr << "[Error] No materials found in Glass part!" << std::endl;
            }
        }
        else if(obj.name == "Propeller") plane.partModel[Plane::PROPELLER] = obj;
        else if(obj.name == "StrobeRudder")
        {
            plane.partModel[Plane::STROBE_RUDDER] = obj;
            plane.emissionColors[Plane::STROBE_RUDDER] = obj.material[0].emission;
            plane.emissionTextures[Plane::STROBE_RUDDER] = obj.material[0].map_emission;
        }
        else if(obj.name == "LightLeftWing") 
        {
            plane.partModel[Plane::LIGHT_LEFT_WING] = obj;
            plane.emissionColors[Plane::LIGHT_LEFT_WING] = obj.material[0].emission;
            plane.emissionTextures[Plane::LIGHT_LEFT_WING] = obj.material[0].map_emission;
        }
        else if(obj.name == "StrobeRightWing")
        {
            plane.partModel[Plane::STROBE_RIGHT_WING] = obj;
            plane.emissionColors[Plane::STROBE_RIGHT_WING] = obj.material[0].emission;
            plane.emissionTextures[Plane::STROBE_RIGHT_WING] = obj.material[0].map_emission;
        }
        else if(obj.name == "StrobeLeftWing")
        {
            plane.partModel[Plane::STROBE_LEFT_WING] = obj;
            plane.emissionColors[Plane::STROBE_LEFT_WING] = obj.material[0].emission;
            plane.emissionTextures[Plane::STROBE_LEFT_WING] = obj.material[0].map_emission;
        }
        else if(obj.name == "LightRightWing")
        {
            plane.partModel[Plane::LIGHT_RIGHT_WING] = obj;
            plane.emissionColors[Plane::LIGHT_RIGHT_WING] = obj.material[0].emission;
            plane.emissionTextures[Plane::LIGHT_RIGHT_WING] = obj.material[0].map_emission;
        }
        else if(obj.name == "LightRudder")
        {
            plane.partModel[Plane::LIGHT_RUDDER] = obj;
            plane.emissionColors[Plane::LIGHT_RUDDER] = obj.material[0].emission;
            plane.emissionTextures[Plane::LIGHT_RUDDER] = obj.material[0].map_emission;
        }
        else if(obj.name == "FlagConnector") {
            plane.partModel[Plane::FLAG_CONNECTOR] = obj;
            if (!plane.partModel[Plane::FLAG_CONNECTOR].material.empty()) {
                for (auto& material : plane.partModel[Plane::FLAG_CONNECTOR].material) {
                    material.map_diffuse = textureLoad("assets/plane/textures/Cessna_Rope_Albedo.png");
                    material.map_normal = textureLoad("assets/plane/textures/Cessna_Rope_Normals.png");
                    material.map_ambient = textureLoad("assets/plane/textures/Cessna_Rope_AO.png");
                    material.map_emission = textureLoad("assets/plane/textures/Cessna_Rope_Emission.png");
                    material.map_shininess = textureLoad("assets/plane/textures/Cessna_Rope_Glossy.png");
                }
            } else {
                std::cerr << "[Error] No materials found in FlagConnector part!" << std::endl;
            }
        }
        else throw std::runtime_error("[Plane] unkown part name: " + obj.name);
    }

    plane.flag = flagCreate(flagFilePath);
    plane.flagModelMatrix = flagPlane::trans;

    return plane;
}

void planeDelete(Plane &plane)
{
    flagDelete(plane.flag);

    for (auto &model : plane.partModel)
    {
        modelDelete(model);
    }

    textureDelete(plane.noEmissionTexture);
    for (auto& model : plane.partModel) {
        for (auto& material : model.material) {
            textureDelete(material.map_diffuse);
            textureDelete(material.map_normal);
            textureDelete(material.map_specular);
            textureDelete(material.map_ambient);
            textureDelete(material.map_emission);
            textureDelete(material.map_shininess);
        }
    }
    /*
    textureDelete(plane.body_ao);
    textureDelete(plane.body_emission);
    textureDelete(plane.body_glossy);
    textureDelete(plane.body_normals);
    textureDelete(plane.body_specular_color);*/
    plane.partModel.clear();
    plane.partTransformations.clear();
}

void planeThrottleControl(Plane &plane, int throttle, float dt)
{
    /* adjust plane speed */
    plane.speed = std::max(plane.minSpeed, std::min(plane.speed + throttle * dt * 10, plane.maxSpeed));
}

void planeTurningControl(Plane &plane, int turningDirection, float dt)
{
    /* rotate due to turning control */
    float dY = dt * turningDirection * 0.5f;
    plane.angles.y += dY;
    plane.dYAngle += dY - dt * plane.dYAngle / static_cast<float>(M_PI_4);
    plane.angles.z -= dt * turningDirection / 1.5f + dt * plane.angles.z / static_cast<float>(M_PI_4);
    
    if (plane.angles.y > static_cast<float>(M_PI))
    {
        plane.angles.y -= static_cast<float>(M_PI) * 2.0f;
    }
    else if (plane.angles.y < -static_cast<float>(M_PI))
    {
        plane.angles.y += static_cast<float>(M_PI) * 2.0f;
    }

    plane.rotation = Matrix4D::rotationY(plane.angles.y) * Matrix4D::rotationZ(plane.angles.z);

    /* negate some of the plane's local rotation for the flag */
    if (std::abs(plane.angles.z) > std::numeric_limits<float>::epsilon() || std::abs(plane.dYAngle) > std::numeric_limits<float>::epsilon())
    {
        plane.flagNegativeRotation = Matrix4D::rotationY(-plane.dYAngle) * Matrix4D::rotationZ(-plane.angles.z * 0.75f);
    }
    else
    {
        plane.flagNegativeRotation = Matrix4D::identity();
    }
}

void planePitchControl(Plane &plane, int pitchDirection, float dt) 
{
    /* adjust position and pitch due to pitch control */
    if (!(plane.position.y <= plane.minHeight + 0.5f && pitchDirection < 0) && !(plane.position.y >= plane.maxHeight - 0.5f && pitchDirection > 0))
    {
        plane.angles.x += dt * -pitchDirection / 4.0f;
    }
    plane.angles.x -= dt * plane.angles.x / static_cast<float>(M_PI_4);

    plane.rotation = plane.rotation * Matrix4D::rotationX(plane.angles.x);
    plane.position.y = std::max(plane.minHeight, std::min(plane.position.y - dt * plane.angles.x * plane.speed, plane.maxHeight));
}

void animatePropeller(Plane &plane, float dt)
{
    /* propeller animation (changes turning rate depending on speed) */
    float propellerRotation = dt * (plane.minPropellerRotationSpeed + getSpeedFactor(plane) * (plane.maxPropellerRotationSpeed - plane.minPropellerRotationSpeed));
    plane.partTransformations[Plane::ePart::PROPELLER] = Matrix4D::rotationZ(propellerRotation) * plane.partTransformations[Plane::ePart::PROPELLER];
}

void planeMove(Plane &plane, bool control[], float dt)
{
    /* retrieve input for controls */
    int throttle = +control[Plane::eControl::FASTER] - control[Plane::eControl::SLOWER];
    int turningDirection = +control[Plane::eControl::LEFT] - control[Plane::eControl::RIGHT];
    int pitchDirection = +control[Plane::eControl::UP] - control[Plane::eControl::DOWN];

    /* calculate the plane's rotations and position */
    planeThrottleControl(plane, throttle, dt);
    planeTurningControl(plane, turningDirection, dt);
    planePitchControl(plane, pitchDirection, dt);

    /* calculate the plane's final transformation */
    plane.transformation = Matrix4D::translation(plane.position) * plane.rotation;

    /* additional animations */
    animatePropeller(plane, dt);

    /* update flag simulation */
    updateSimulation(plane.flagSim, getSpeedFactor(plane), dt);
}

Vector3D getPlaneTurningVector(Plane &plane)
{
    /* return Matrix3D::rotationY(-plane.angles.z) * Matrix3D::rotationY(plane.angles.y) * Vector3D(-1.0f, 0.0f, 0.0f); */
    return Matrix3D::rotationY(plane.angles.y) * Vector3D(-1.0f, 0.0f, 0.0f);
}

float getSpeedFactor(Plane &plane)
{
    return (plane.speed - plane.minSpeed) / (plane.maxSpeed - plane.minSpeed);
}

float getSpeedFov(Plane &plane)
{
    float fovInDegrees = plane.minFov + getSpeedFactor(plane) * (plane.maxFov - plane.minFov);
    return static_cast<float>(to_radians(fovInDegrees));
}

void setLightEmission(Plane &plane, bool emission)
{
    for (auto const& [part, color] : plane.emissionColors)
    {
        /* assumes that each part in emission color has only one material */
        plane.partModel[part].material[0].emission = emission ? color : Vector3D(0.0f, 0.0f, 0.0f);
        plane.partModel[part].material[0].map_emission = emission ? plane.emissionTextures[part] : plane.noEmissionTexture;
    }
}

void setLightEmission(Plane &plane, bool emission, Plane::ePart part)
{
    /* assumes that each part in emission color has only one material */
    plane.partModel[part].material[0].emission = emission ? plane.emissionColors[part] : Vector3D(0.0f, 0.0f, 0.0f);
    plane.partModel[part].material[0].map_emission = emission ? plane.emissionTextures[part] : plane.noEmissionTexture;
}

