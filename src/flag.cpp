#include <algorithm>

#include "flag.h"

#include <stdexcept>

float getDisplacementValue(Vector2D pos, const FlagSim &sim, int waveParamIndex)
{
    const WaveParams &params = sim.parameter[waveParamIndex];
    
    return params.amplitude * static_cast<float>(sin(dot(normalize(params.direction), pos) * params.omega + sim.accumTime * params.phi));
}

float flagDisplacement(const FlagSim &sim, Vector2D position, float minPosZ)
{
    float displacement = getDisplacementValue(position, sim, 0) + getDisplacementValue(position, sim, 1) + getDisplacementValue(position, sim, 2);
    float positionScale = position[1] / minPosZ;
    return displacement * positionScale;
}

Flag flagCreate(const std::string& flagFilePath)
{
    Flag flag;
    std::vector<Model> models = modelLoad(flagFilePath);

    if(models.size() != 1)
    {
        throw std::runtime_error("[Flag] number of parts do not match!" + std::to_string(models.size()));
    }
    flag.model = models[0];

    flag.flag_displacement = textureLoad("assets/flag/textures/Flag_Displacement.png");

    for (auto& material : flag.model.material){
        material.map_diffuse = textureLoad("assets/flag/textures/Flag_Albedo.png");
        material.map_emission = textureLoad("assets/flag/textures/Cessna_Flag_Emission.png");
        material.map_ambient = textureLoad("assets/flag/textures/Flag_AO.png");
        material.map_normal = textureLoad("assets/flag/textures/Flag_Normals.png");
        material.map_specular = textureLoad("assets/flag/textures/Flag_Specular_Color.png");
        material.map_shininess = textureLoad("assets/flag/textures/Flag_Specular.png");
    }

    flag.minPosZ = -8.0f;

    return flag;
}

void flagDelete(Flag &flag)
{
    modelDelete(flag.model); // already includes texture delete
}

void updateSimulation(FlagSim& flagSim, float speedFactor, float dt)
{
    float dtMultiplier = (flagSim.maxDtFactor - flagSim.minDtFactor) * speedFactor + flagSim.minDtFactor;
    flagSim.accumTime += dtMultiplier * dt;
}
