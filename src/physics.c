#include "physics.h"

// Computes the orthogonal decomposition of vector `a` with respect to vector `b`
// Requires that vector `b` is a unit vector
// Equivalent to `dest = a - dot(a, b) * b`
inline void glm_vec2_ortho_decomp(vec2 a, vec2 b, vec2 dest)
{
    glm_vec2_copy(a, dest);
    float dot = glm_vec2_dot(a, b);
    glm_vec2_muladds(b, -dot, dest);
}

// Computes the reflection of incident vector `v` about normal `n`
// Requires that `n` is a unit vector
// Equivalent to `dest = v - 2 * dot(v, n) * n`
void glm_vec2_reflect(vec2 v, vec2 n, vec2 dest) // TODO: in place version?
{
    glm_vec2_copy(v, dest);
    float dot = glm_vec2_dot(v, n);
    glm_vec2_muladds(n, -2 * dot, dest);
}

// Computes the reflection of incident vector `v` about the normal
// defined by the x axis (`axis` = 0) or y axis (`axis` = 1)
void glm_vec2_reflect_axis(vec2 v, int axis, vec2 dest) // TODO: in place version?
{
    glm_vec2_copy(v, dest);

    axis = axis == 1 ? 1 : -1; // y -> axis = 1; x -> axis = -1

    dest[0] *= -axis; // reflects acros y if axis = 1 (y)
    dest[1] *= axis; // reflects across x if axis = -1 (x)
}

// Computes the unit normal of vector `v`
// Equivalent to `dest = (v.y, -v.x) / len(v)`
void glm_vec2_normal(vec2 v, vec2 dest)
{
    glm_vec2_normalize_to(v, dest);
    float tempX = dest[0];
    dest[0] = dest[1];
    dest[1] = -tempX;
}

/**
 * @brief Computes the collision time(s), if any, between two circles with position p, 
 * velocity v, and radius r
 * @param[out] times Collision times are stored in this parameter. Only valid if the function
 * returns `true`
 * @returns `true` if any collision occured
 */
bool CircleCircleCollisionTime(vec2 p1, vec2 v1, float r1, vec2 p2, vec2 v2, float r2,
    vec2 times)
{
    vec2 dp, dv;
    glm_vec2_sub(p2, p1, dp); // delta of the positions
    glm_vec2_sub(v2, v1, dv); // delta of the velocities
    float r = r1 + r2; // overall radius

    // Collision times is a quadratic function of t
    // a * t^2 + b * t + c = 0
    float a = glm_vec2_dot(dv, dv);
    float b = 2 * glm_vec2_dot(dp, dv);
    float c = glm_vec2_dot(dp, dp) - r * r;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float a2 = 2 * a;
    float v = sqrtf(discriminant) / a2;
    float u = -b / a2;

    times[0] = u - v; // TODO: check for negative collision times
    times[1] = u + v;

    return true;
}

/**
 * @brief Computes the collision time(s), if any, between a moving circle and a line.
 * Line direction `dir` must be normalized.
 * @param[out] times Collision times are stored in this parameter. Only valid if the
 * function returns `true`.
 * @returns `true` if any collision occured (even collisions in the past)
 */
bool CircleLineCollisionTime(vec2 pCircle, vec2 velocity, float radius, vec2 pLine, vec2 dir,
    vec2 times)
{
    vec2 dp, pPerp, vPerp; // TODO: write simplified methods for horizontal or vertical lines
    glm_vec2_sub(pLine, pCircle, dp);
    glm_vec2_ortho_decomp(dp, dir, pPerp);
    glm_vec2_ortho_decomp(velocity, dir, vPerp);

    float a = glm_vec2_dot(vPerp, vPerp);
    float b = -2 * glm_vec2_dot(pPerp, vPerp);
    float c = glm_vec2_dot(pPerp, pPerp) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float a2 = 2 * a;
    float v = sqrtf(discriminant) / a2;
    float u = -b / a2;

    times[0] = u - v; // TODO: check for negative collision times
    times[1] = u + v;

    return true;
}