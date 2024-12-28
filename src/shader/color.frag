#version 330 core

struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    vec3 emission;
    float shininess;
};

struct Light
{
    vec3 lightPos;
    vec3 globalAmbientLightColor;
    vec3 lightColor;
    float ka;                       // ambient coefficient  [0, 1]
    float kd;                       // diffuse coefficient  [0, 1]
    float ks;                       // specular coefficient [0, 1]
};

uniform Light uLight;
uniform vec3 uCameraPos; // camera position needed for specular computations

in vec3 tNormal;
in vec3 tFragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material uMaterial;
//uniform sampler2D map_emission;

vec3 directionalLight(vec3 normal, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - tFragPos);
    vec3 viewDir = normalize(uCameraPos - tFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient Component: ojects always get some color from surrounding light sources
    vec3 ambientComponent = uLight.ka * uMaterial.ambient * uLight.globalAmbientLightColor;

    // Diffuse Component: The more a part of an object faces the light source the brihter it gets
    vec3 diffuseComponent = uLight.kd * uMaterial.diffuse * uLight.lightColor * dot(normal, lightDir);

    // Specular component
    float specFactor = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
    vec3 specularComponent = uLight.ks * uMaterial.specular * uLight.lightColor * specFactor;

    return ambientComponent + diffuseComponent + specularComponent;
}

void main(void)
{
    vec3 normal = normalize(tNormal);

    // Compute the directional/global light contribution
    vec3 lightResult = directionalLight(normal, uLight.lightPos);

    //vec3 emissionFromTexture = texture(map_emission, TexCoords).rgb;

    vec3 finalColor = lightResult + uMaterial.emission; //+ emissionFromTexture;
    
    FragColor = vec4(finalColor, 1.0);
}
