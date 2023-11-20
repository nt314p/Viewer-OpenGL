#pragma once
#include <cglm\cglm.h>

void glm_vec2_ortho_decomp(vec2 a, vec2 b, vec2 dest);

bool CircleCircleCollisionTime(vec2 p1, vec2 v1, float r1, vec2 p2, vec2 v2, float r2,
    vec2 times);
bool CircleLineCollisionTime(vec2 pc, vec2 vel, float r, vec2 pl, vec2 d, vec2 times);