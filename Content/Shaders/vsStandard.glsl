#version 450

in vec3 m_position;

uniform mat4 m_matView;
uniform mat4 m_matProj;

void main()
{
    gl_Position = m_matProj*m_matView* vec4(m_position, 1.0);
}