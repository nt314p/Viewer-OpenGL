#pragma once
#include <cglm\cglm.h>

typedef struct Interaction
{
    uint32_t id; // the id of the body the object is interacting with
    float time; // time of hit, global time
} Interaction;

void glm_vec2_ortho_decomp(vec2 a, vec2 b, vec2 dest);
void glm_vec2_reflect(vec2 v, vec2 n, vec2 dest);
void glm_vec2_reflect_axis(vec2 v, int axis, vec2 dest);

bool CircleCircleCollisionTime(vec2 p1, vec2 v1, float r1, vec2 p2, vec2 v2, float r2,
    vec2 times);
bool CircleLineCollisionTime(vec2 pc, vec2 vel, float r, vec2 pl, vec2 d, vec2 times);