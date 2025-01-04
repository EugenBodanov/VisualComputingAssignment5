#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 filterColor; // Declare the color multiplier

void main()
{
    vec3 color = texture(skybox, TexCoords).rgb;
    color *= filterColor;
    FragColor = vec4(color, 1.0);
}
