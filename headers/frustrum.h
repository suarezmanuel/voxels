// frustum.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Plane {
    glm::vec3 normal;
    float distance;

    Plane() = default;

    Plane(const glm::vec4& v) : normal(v.x, v.y, v.z), distance(v.w) {
        float length = glm::length(normal);
        normal /= length;
        distance /= length;
    }

    float getSignedDistanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

struct Frustum {
    Plane planes[6]; // 0:Left, 1:Right, 2:Bottom, 3:Top, 4:Near, 5:Far

    void update(const glm::mat4& viewProjectionMatrix) {
        const glm::mat4& m = glm::transpose(viewProjectionMatrix);
        planes[0] = Plane(m[3] + m[0]); // Left
        planes[1] = Plane(m[3] - m[0]); // Right
        planes[2] = Plane(m[3] + m[1]); // Bottom
        planes[3] = Plane(m[3] - m[1]); // Top
        planes[4] = Plane(m[3] + m[2]); // Near
        planes[5] = Plane(m[3] - m[2]); // Far
    }

    // AABB check (Axis-Aligned Bounding Box)
    // In frustum.h, replace the existing isBoxVisible function with this one:

bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const {
    // Check box against all 6 planes
    for (int i = 0; i < 6; i++) {
        glm::vec3 p_vertex = min;
        glm::vec3 n_vertex = max;

        // Find the "positive" and "negative" vertices of the AABB
        // along the direction of the plane's normal.
        if (planes[i].normal.x >= 0) {
            p_vertex.x = max.x;
            n_vertex.x = min.x;
        }
        if (planes[i].normal.y >= 0) {
            p_vertex.y = max.y;
            n_vertex.y = min.y;
        }
        if (planes[i].normal.z >= 0) {
            p_vertex.z = max.z;
            n_vertex.z = min.z;
        }

        // If the "positive" vertex is on the negative side of the plane,
        // the entire box is outside, so we can cull it.
        if (planes[i].getSignedDistanceToPoint(p_vertex) < 0.0f) {
            return false;
        }

        // OPTIONAL BUT RECOMMENDED: A check for objects fully inside the frustum.
        // If the "negative" vertex is on the positive side, the box is fully
        // contained by this plane. We don't need to check other planes.
        // This is a small optimization. The main culling logic is the check above.
        // if (planes[i].getSignedDistanceToPoint(n_vertex) < 0.0f) {
        //     // Box intersects the plane. We must continue checking other planes.
        // }
    }

    // If the box was not culled by any of the planes, it must be visible.
    return true;
}
};