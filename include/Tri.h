#pragma once
#ifndef TRI_H
#define TRI_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace std;

class Tri
{
public:
    Tri(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, 
        glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3);

    glm::vec3 getClosest(glm::vec3 pos);

    bool intersectWithRay(
        glm::vec3 ray_dir,
        glm::vec3 ray_pos,
        float& t
    );

    bool intersectWithBox(
        glm::vec3 box_center,
        glm::vec3 box_r
    );

    void print()
    {
        cout << m_a.x << " " << m_a.y << " " << m_a.z << endl;
        cout << m_b.x << " " << m_b.y << " " << m_b.z << endl;
        cout << m_c.x << " " << m_c.y << " " << m_c.z << endl;
    }

    inline glm::vec3 getV1() { return m_a; };
    inline glm::vec3 getV2() { return m_b; };
    inline glm::vec3 getV3() { return m_c; };

    inline glm::vec2 getUV1() { return m_uv1; };
    inline glm::vec2 getUV2() { return m_uv2; };
    inline glm::vec2 getUV3() { return m_uv3; };

    inline void updateVertices(glm::vec3 a, glm::vec3 b, glm::vec3 c)
    {
        m_a = a;
        m_b = b;
        m_c = c;
    }

private:
    glm::vec3 m_a;
    glm::vec3 m_b;
    glm::vec3 m_c;

    glm::vec2 m_uv1;
    glm::vec2 m_uv2;
    glm::vec2 m_uv3;
};

#endif // !TRI_H
