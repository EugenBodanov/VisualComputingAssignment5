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
in vec3 normal;

out vec4 FragColor;

uniform Material uMaterial;
uniform sampler2D map_diffuse;
uniform sampler2D map_ambient;
uniform sampler2D map_emission;
uniform sampler2D map_shininess;
uniform sampler2D map_normal;
uniform sampler2D map_specular;
uniform bool hasSpecular;
uniform mat4 uModel;

/*
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
}*/

vec3 blinnPhongIllumination(
    vec3 normal, 
    vec3 fragPos, 
    vec3 cameraPos, 
    vec3 lightPos,
    vec3 ambientMaterial, 
    vec3 diffuseMaterial, 
    vec3 specularMaterial,
    float shininess
) {
    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 viewDir = normalize(cameraPos - fragPos);

    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 ambientComponent =
        uLight.ka * ambientMaterial * uLight.globalAmbientLightColor;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseComponent =
        uLight.kd * diffuseMaterial * uLight.lightColor * diff;

    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float specFactor = pow(specAngle, shininess);
    vec3 specularComponent = uLight.ks * specularMaterial.rgb * uLight.lightColor * specFactor;

    return ambientComponent + diffuseComponent + specularComponent;
}


void main(void)
{
    Light dummy = uLight; // compiler get rid of uniforms, which are unused in main 
    Material dummy2 = uMaterial;

    vec3 tex_diffuse = texture(map_diffuse, TexCoords).rgb;
    vec3 tex_ambient = texture(map_ambient, TexCoords).rgb;
    vec4 tex_emission = texture(map_emission, TexCoords);
    float tex_shininess = texture(map_shininess, TexCoords).r * 1000.0;
    vec3 tex_normals = texture(map_normal, TexCoords).rgb; // x, y, z
    vec3 tex_specular = texture(map_specular, TexCoords).rgb;

    vec3 ambientMaterial = tex_diffuse * tex_ambient;
    vec3 n_objectSpace  = tex_normals * 2.0 - 1.0;
    vec3 n_world = normalize( mat3(transpose(inverse(mat3(uModel)))) * n_objectSpace );

    float s = 0.25;
    vec3 n_s = normalize(s * n_world + (1.0 - s) * normal);

    float d = dot(tNormal, normalize(uCameraPos - tFragPos));
    if (d < 0.0) {
        n_s = -n_s;
    }

    vec3 blinnResult = blinnPhongIllumination(
        n_s,
        tFragPos,
        uCameraPos,
        uLight.lightPos,
        ambientMaterial,
        tex_diffuse,
        tex_specular,
        tex_shininess
    );

    float alpha = texture(map_diffuse, TexCoords).a;

    vec4 finalColor = vec4(blinnResult, alpha) + tex_emission;

/*
    vec4 tex_specular2 = vec4(0.0);
    if (hasSpecular) {
        tex_specular2 = texture(map_specular, TexCoords);
    }*/

    FragColor = finalColor;
}
