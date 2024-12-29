#version 330 core

in vec3 tNormal;
in vec3 tFragPos;
out vec4 FragColor;

uniform vec3 uCameraPos;
uniform bool isFlag;

struct Material {
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    vec3 emission;
    float shininess;
};

struct Light {
    vec3 lightPos;
    vec3 globalAmbientLightColor;
    vec3 lightColor;
    float ka;
    float kd;
    float ks;
};

/*
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    vec3 direction;
    float angle;
};*/

uniform Material uMaterial;
uniform Light uLight;
//uniform PointLight lights[6];
uniform int numLights;
uniform bool planeLightsOn;

vec3 directionalLight(vec3 normal, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - tFragPos);
    vec3 viewDir = normalize(uCameraPos - tFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 ambientComponent = uLight.ka * uMaterial.ambient * uLight.globalAmbientLightColor;
    vec3 diffuseComponent = uLight.kd * uMaterial.diffuse * uLight.lightColor * max(dot(normal, lightDir), 0.0);
    float specFactor = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
    vec3 specularComponent = uLight.ks * uMaterial.specular * uLight.lightColor * specFactor;

    return ambientComponent + diffuseComponent + specularComponent;
}

void main(void)
{
    vec3 normal = normalize(tNormal);

    vec3 viewDir = normalize(uCameraPos - tFragPos);
    if (isFlag) {
        normal = normalize(normal);
    }
    // if (isFlag && dot(normal, viewDir) < 0.0)
    // {
    //     normal = -normal;
    // }

    vec3 lightResult = directionalLight(normal, uLight.lightPos);

/*
    vec3 pointLightResult = vec3(0.0);
    for (int i = 0; i < numLights; i++)
    {
        if (!planeLightsOn) {
            continue;
        }
        vec3 fragToLight = normalize(lights[i].position - tFragPos);
        float theta = dot(fragToLight, normalize(lights[i].direction));
        float cosCutoff = cos(lights[i].angle);

        if (theta >= cosCutoff) {
            float diff = max(dot(normal, fragToLight), 0.0);
            float distance = length(lights[i].position - tFragPos);
            float attenuation = lights[i].intensity / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * distance * distance);
            vec3 diffuse = diff * lights[i].color * attenuation;
            pointLightResult += diffuse * uMaterial.diffuse;
        }
    }
*/
    //vec3 finalColor = lightResult + pointLightResult + uMaterial.emission;
    vec3 finalColor = lightResult + uMaterial.emission;
    FragColor = vec4(finalColor, 1.0);
}
