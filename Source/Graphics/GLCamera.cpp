#include "GLCamera.h"
#include <string>
#include <GLEW/glew.h>

GLCamera::GLCamera(float _fovy, int _width,int _height, float _nearZ, float _farZ)
{
    //m_ViewPort.MinDepth = 0.0f;
    //m_ViewPort.MaxDepth = 1.0f;
    
    m_width = _width;
    m_height = _height;
    
    m_aspectRatio = (float)m_width / (float)m_height;

    m_FOVy = _fovy;//2 * glm::atan(glm::tan(0.5f * _fovy) * m_aspectRatio);

    m_nearZ = _nearZ;
    m_farZ = _farZ;

    m_position  = glm::vec3(0, 0, 0);
    m_forward   = glm::vec3(0, 0, -1);
    m_right     = glm::vec3(1, 0, 0);
    m_up        = glm::vec3(0, 1, 0);

    UpdateView();
    UpdateProjection();
    SetForward(glm::vec3(0,0,-1));
}

GLCamera::GLCamera(void)
{

}

GLCamera::~GLCamera(void)
{

}


void GLCamera::UpdateView()
{
	glm::vec3 up, right, forward, position;
	
//	memcpy(&up, &m_up, sizeof(glm::vec3));
//	memcpy(&right, &m_right, sizeof(glm::vec3));
//	memcpy(&forward, &m_forward, sizeof(glm::vec3));
//	memcpy(&position, &m_position, sizeof(glm::vec3));

    glm::vec3 R = glm::vec3(m_right.x,      m_right.y,      m_right.z);
	glm::vec3 U = glm::vec3(m_up.x,         m_up.y,         m_up.z);
	glm::vec3 L = glm::vec3(m_forward.x,    m_forward.y,    m_forward.z);
	glm::vec3 P = glm::vec3(m_position.x,   m_position.y,   m_position.z);

	// Keep camera's axes orthogonal to each other and of unit length.
   L =  glm::normalize(L);
	U =  glm::normalize(glm::cross(L, R));
	// U, L already ortho-normal, so no need to normalize cross product.
    R = glm::cross(U, L);

	// Fill in the view matrix entries.
    float x = -glm::vec3(glm::dot(P, R)).x;
	float y = -glm::vec3(glm::dot(P, U)).x;
	float z = -glm::vec3(glm::dot(P, L)).x;
        
	right   = R;
	up      = U;
	forward = L;

	m_view[0][0] = R.x;
	m_view[1][0] = R.y;
	m_view[2][0] = R.z;
	m_view[3][0] = x;

	m_view[0][1] = U.x;
	m_view[1][1] = U.y;
	m_view[2][1] = U.z;
	m_view[3][1] = y;

	m_view[0][2] = L.x;
	m_view[1][2] = L.y;
	m_view[2][2] = L.z;
	m_view[3][2] = z;

	m_view[0][3] = 0.0f;
	m_view[1][3] = 0.0f;
	m_view[2][3] = 0.0f;
	m_view[3][3] = 1.0f;
        
}

void GLCamera::UpdateProjection()
{

    printf("FOV: %f | AspectRatio: %f | nearZ: %f | farZ: %f\n",m_FOVy,m_aspectRatio,m_nearZ,m_farZ);
    glm::mat4 proj;
    proj =glm::perspective(m_FOVy,m_aspectRatio,m_nearZ,m_farZ);
        
    memcpy(&m_projection, &proj, sizeof(glm::mat4));
//    
//    for(int i=0;i< 4;i++)
//    {
//        for(int j=0; j < 4;j++)
//            printf("%f ",m_projection[i][j]);
//        printf("\n");
//    }
//    printf("PROJECTION END\n");
}

glm::mat4 *GLCamera::GetProjection()
{
//    for(int i=0;i< 4;i++)
//    {
//        for(int j=0; j < 4;j++)
//            printf("%f ",m_projection.m[i][j]);// .m[i][j]);
//        printf("\n");
//    }
//    printf("PROJECTION END\n");
    
    return &m_projection;
}

glm::mat4* GLCamera::GetView()
{
    return &m_view;
}

void GLCamera::SetLookAt(glm::vec3 _target)
{
    glm::vec3 forward;
    forward.x = _target.x - m_position.x;
    forward.y = _target.y - m_position.y;
    forward.z = _target.z - m_position.z;

    SetForward(forward);
}

void GLCamera::SetForward(glm::vec3 forward)
{
        glm::vec3 pos = glm::vec3(m_position.x,m_position.y,m_position.z);
	glm::vec3 direction = glm::vec3(forward.x,forward.y,forward.z);
	direction = glm::normalize(direction);
	glm::vec3 up2 = glm::vec3(0, 1, 0);

        if (direction == glm::vec3(0,0,0))
		direction = glm::vec3(0, 0, 1);

	else if (direction == glm::vec3(0, -1, 0))
		up2 = glm::vec3(0, 0, 1);

	else if (direction == glm::vec3(0,1,0))
		up2 = glm::vec3(0, 0, -1);

        
	m_view = glm::lookAt(pos,pos+direction,up2);

	m_right = glm::vec3(m_view[0][0], m_view[1][0], m_view[2][0]);
	m_up = glm::vec3(m_view[0][1], m_view[1][1], m_view[2][1]);
	m_forward = glm::vec3(m_view[0][2], m_view[1][2], m_view[2][2]);
    

        
	UpdateView();
        
}

void GLCamera::Move(glm::vec3 _move)
{
    m_position.x += _move.x;
    m_position.y += _move.y;
    m_position.z += _move.z;
    
    UpdateView();
    
}

void GLCamera::Move(float _move)
{
    m_position.x += (m_forward.x)*_move;
    m_position.y += (m_forward.y)*_move;
    m_position.z += (m_forward.z)*_move;
    
    UpdateView();
    
}

void GLCamera::Strafe(float delta) 
{
	m_position += delta * m_right;

	UpdateView();
}

void GLCamera::Roll(float angle) 
{
	glm::mat4 R = glm::rotate(angle, m_forward);
	glm::vec4 r = (R * glm::vec4(m_right, 0));
	m_right.x = r.x;
	m_right.y = r.y;
	m_right.z = r.z;
	glm::vec4 u = (R * glm::vec4(m_up, 0));
	m_up.x = u.x;
	m_up.y = u.y;
	m_up.z = u.z;

	UpdateView();
}

void GLCamera::Pitch(float angle) 
{
	glm::mat4 R = glm::rotate(angle, m_right);
	glm::vec4 l = (R * glm::vec4(m_forward, 0));
	m_forward.x = l.x;
	m_forward.y = l.y;
	m_forward.z = l.z;
	glm::vec4 u = (R * glm::vec4(m_up, 0));
	m_up.x = u.x;
	m_up.y = u.y;
	m_up.z = u.z;

	UpdateView();
}

void GLCamera::Yaw(float angle) 
{
	glm::mat4 R = glm::rotate(angle, glm::vec3(0, 1, 0));
	glm::vec4 l = (R * glm::vec4(m_forward, 0));
	m_forward.x = l.x;
	m_forward.y = l.y;
	m_forward.z = l.z;
	glm::vec4 r = (R * glm::vec4(m_right, 0));
	m_right.x = r.x;
	m_right.y = r.y;
	m_right.z = r.z;

	UpdateView();
}

void GLCamera::RotateY(float angle) 
{   
	glm::mat4 R = glm::rotate(angle, glm::vec3(0, 1, 0));
	glm::vec4 l = (glm::vec4(m_forward, 0) * R);
	m_forward.x = l.x;
	m_forward.y = l.y;
	m_forward.z = l.z;
	glm::vec4 r = (glm::vec4(m_right, 0) * R);
	m_right.x = r.x;
	m_right.y = r.y;
	m_right.z = r.z;
	glm::vec4 u = (glm::vec4(m_up, 0) * R);
	m_up.x = u.x;
	m_up.y = u.y;
	m_up.z = u.z;

	UpdateView();
}