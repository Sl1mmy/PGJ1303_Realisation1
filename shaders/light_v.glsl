#version 330 core

in vec3 a_position;
in vec3 a_normal;
out vec3 v_normal;
out vec3 v_worldPos;

layout(std140) uniform Matrices
{
    mat4 worldMatrix;
    mat4 viewProjMatrix;
};

void main()
{
    vec4 worldPos = worldMatrix * vec4(a_position, 1);
    vec4 worldNrm = worldMatrix * vec4(a_normal, 0);
    gl_Position = viewProjMatrix * worldPos;
    v_normal = worldNrm.xyz;
    v_worldPos = worldPos.xyz;
}
