#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uCameraPos;

out vec3 tNormal;
out vec3 tFragPos;
out vec2 TexCoords;
out vec3 ReflectDir;

void main(void)
{
    gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
    tFragPos = vec3(uModel * vec4(aPosition, 1.0));
    // tNormal = mat3(transpose(inverse(uModel))) * aNormal;
    TexCoords = aUV;
    tNormal = normalize(mat3(uModel) * aNormal);
    vec3 I = normalize(tFragPos - uCameraPos);
    ReflectDir = reflect(I, normalize(tNormal));
}
