#include "planet.h"

#include <stdexcept>
#include <iostream>

Planet planetLoad(const std::string &planetFilePath)
{

    Planet planet;
    planet.partModel = modelLoad(planetFilePath);
    planet.noEmissionTexture = textureCreateSingleColor(1, 1, {0.0f, 0.0f, 0.0f});

    if(planet.partModel.size() <= 0)
    {
        throw std::runtime_error("[Planet] no parts could be loaded!");
    }

    /* go over all models and find all materials with emission -> save in emission color map */
    for (size_t part_id = 0u; part_id < planet.partModel.size(); part_id++)
    {
        //std::cout << "Part " << part_id << ": " << planet.partModel[part_id].name << std::endl;
        std::map<int, Vector3D> emissionColors;
        std::map<int, Texture> emissionTextures;
        auto &part = planet.partModel[part_id];
        for (auto mat_id=0u; mat_id < planet.partModel[part_id].material.size(); mat_id++)
        {
            auto mat = planet.partModel[part_id].material[mat_id];

            // Texture setUp
            if (part.name == "Boats"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Boats_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Boats_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Boats_Emission.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Boats_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Boats_Normals.png");
                mat.map_specular = textureLoad("assets/planet/textures/Boats_Specular_Color.png");
            } else if (part.name == "Continent"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Continents_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Continents_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Continents_Emission.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Continents_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Continents_Normals.png");
                mat.map_specular = textureLoad("assets/planet/textures/Continents_Specular_Color.png");
            } else if (part.name == "Houses"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Houses_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Houses_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Houses_Emit.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Houses_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Houses_Normal.png");
                mat.map_specular = textureLoad("assets/planet/textures/Houses_Specular_Color.png");
            } else if (part.name == "Ocean"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Ocean_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Ocean_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Ocean_Emission.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Ocean_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Ocean_Normals.png");
                mat.map_specular = textureLoad("assets/planet/textures/Ocean_Specular_Color.png");
            } else if (part.name == "Trees"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Trees_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Trees_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Trees_Emission.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Trees_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Trees_Normals.png");
                mat.map_specular = textureLoad("assets/planet/textures/Trees_Specular_Color.png");
            } else if (part.name == "Vistas"){
                mat.map_diffuse = textureLoad("assets/planet/textures/Vistas_Albedo.png");
                mat.map_ambient = textureLoad("assets/planet/textures/Vistas_AO.png");
                mat.map_emission = textureLoad("assets/planet/textures/Vistas_Emission.png");
                mat.map_shininess = textureLoad("assets/planet/textures/Vistas_Glossy.png");
                mat.map_normal = textureLoad("assets/planet/textures/Vistas_Normals.png");
                mat.map_specular = textureLoad("assets/planet/textures/Vistas_Specular_Color.png");
            }
            
            if (mat.emission.x > 0.0f || mat.emission.y > 0.0f || mat.emission.z > 0.0f)
            {
                emissionColors[mat_id] = planet.partModel[part_id].material[mat_id].emission;
            }
            emissionTextures[mat_id] = planet.partModel[part_id].material[mat_id].map_emission;
        }
        if (emissionColors.size() > 0)
        {
            planet.emissionColors[part_id] = emissionColors;
        }
        if (emissionTextures.size() > 0)
        {
            planet.emissionTextures[part_id] = emissionTextures;
        }
    }

    return planet;
}

void planetDelete(Planet &planet)
{
    for (auto &model : planet.partModel)
    {
        modelDelete(model);
    }

    textureDelete(planet.noEmissionTexture);
    planet.partModel.clear();
}

void planetRotate(Planet &planet, Vector3D rotationVec, float planeSpeed, float dt)
{
    Matrix4D planetRotation = Matrix4D::rotation(dt * planeSpeed / 100, rotationVec);
    
    planet.rotation = planetRotation * planet.rotation;
    planet.transformation = planetRotation * planet.transformation;
}

void setEmisson(Planet &planet, bool emission)
{
    for (auto &pair : planet.emissionColors)
    {
        int part_id = pair.first;
        std::map<int, Vector3D> emission_colors = pair.second;
        for (auto &mat_pair : emission_colors) {
            int mat_id = mat_pair.first;
            Vector3D color = mat_pair.second;
            planet.partModel[part_id].material[mat_id].emission = emission ? color : Vector3D(0.0, 0.0, 0.0);
        }
    }
    for (auto &pair : planet.emissionTextures)
    {
        int part_id = pair.first;
        std::map<int, Texture> emission_textures = pair.second;
        for (auto &mat_pair : emission_textures) {
            int mat_id = mat_pair.first;
            Texture texture = mat_pair.second;
            planet.partModel[part_id].material[mat_id].map_emission = emission ? texture : planet.noEmissionTexture;
        }
    }
}
