#version 330 core

in vec3 a_position;
in vec3 a_normal;
in vec4 a_color;

out vec3 v_normal;
out vec3 v_worldPos;
out vec4 v_color;

layout(std140) uniform Matrices {
    mat4 proj;
    mat4 view;
};

uniform mat4 model;

void main()
{
    vec4 worldPos = model * vec4(a_position, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    v_normal = normalize(normalMatrix * a_normal);
    v_worldPos = worldPos.xyz;
    v_color = a_color;

    gl_Position = proj * view * worldPos;
}