#include "physics.h"

// Computes the orthogonal decomposition of vector `a` with respect to vector `b`
// Requires that vector `b` is a unit vector
// Equivalent to `dest = a - dot(a, b) * b`
void glm_vec2_ortho_decomp(vec2 a, vec2 b, vec2 dest)
{
    glm_vec2_copy(a, dest);
    float dot = glm_vec2_dot(a, b);
    glm_vec2_muladds(b, -dot, dest);
}

// Computes the reflection of incident vector `v` about normal `n`
// Requires that `n` is a unit vector
// Equivalent to `dest = v - 2 * dot(v, n) * n`
void glm_vec2_reflect(vec2 v, vec2 n, vec2 dest)
{
    glm_vec2_copy(v, dest);
    float dot = glm_vec2_dot(v, n);
    glm_vec2_muladds(n, -2 * dot, dest);
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

// Computes the collision time(s), if any, between two circles with position p, velocity v, and radius r
// Returns true if any collision occured - collision times are placed in the `times` parameter
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

// Computes the collision time(s), if any, between a circle with radius r, position pc, and velocity vel,
// and a line with position pl and normalized direction d
// Returns true if any collision occured - collision times are placed in the `times` parameter
// TODO: write simplified methods for horizontal or vertical lines
bool CircleLineCollisionTime(vec2 pc, vec2 vel, float r, vec2 pl, vec2 d, vec2 times)
{
    vec2 dp, pPerp, vPerp;
    glm_vec2_sub(pl, pc, dp);
    glm_vec2_ortho_decomp(dp, d, pPerp);
    glm_vec2_ortho_decomp(vel, d, vPerp);

    float a = glm_vec2_dot(vPerp, vPerp);
    float b = -2 * glm_vec2_dot(pPerp, vPerp);
    float c = glm_vec2_dot(pPerp, pPerp) - r * r;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float a2 = 2 * a;
    float v = sqrtf(discriminant) / a2;
    float u = -b / a2;

    times[0] = u - v; // TODO: check for negative collision times
    times[1] = u + v;

    return true;
}