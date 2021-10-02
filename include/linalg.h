#ifndef LINALG_H_
#define LINALG_H_

#include <math.h>
#include <stdbool.h>

static const float PI_f = 3.14159265358979323846f;
static const float PI_2_f = 1.57079632679489661923f;
static const float PI_4_f = 0.78539816339744830962f;

//
// Generic functions
//

static inline float lerp(float a, float b, float t) { return (1 - t)*a + t*b; }
static inline float degrees(float rad) { return rad * 180.0f / PI_f; }
static inline float radians(float deg) { return deg * PI_f / 180.0f; }

static inline float max(float a, float b) { return a > b ? a : b; }
static inline float min(float a, float b) { return a < b ? a : b; }
static inline float clamp(float x, float min, float max) { return x < min ? min : x > max ? max : x; }

// Polynomial easing of the form f(x) = x^power with x in [0, 1] and power >= 0.
// A power of 0 interpolates linearly.
static inline float polynomial_easing(float x, float power) { return power == 0 ? x : powf(x, power); }
static inline float polynomial_ease_in(float x, float power) { return polynomial_easing(x, power); }
static inline float polynomial_ease_out(float x, float power) { return polynomial_easing(1 - x, power); }
static inline float polynomial_ease_inout(float x, float power) { return x < 0.5f ? polynomial_easing(2 * x, power) * 0.5 : (1 - polynomial_easing(1 - 2 * (x - 0.5), power)) * 0.5 + 0.5; }

//
// Vector maths
//

typedef struct vec2_t vec2_t;

struct vec2_t
{
    float x;
    float y;
};

static inline vec2_t vec2_add(vec2_t lhs, vec2_t rhs) { return (vec2_t){lhs.x + rhs.x, lhs.y + rhs.y}; }
static inline vec2_t vec2_sub(vec2_t lhs, vec2_t rhs) { return (vec2_t){lhs.x - rhs.x, lhs.y - rhs.y}; }
static inline vec2_t vec2_mul(vec2_t lhs, vec2_t rhs) { return (vec2_t){lhs.x * rhs.x, lhs.y * rhs.y}; }
static inline vec2_t vec2_div(vec2_t lhs, vec2_t rhs) { return (vec2_t){lhs.x / rhs.x, lhs.y / rhs.y}; }
static inline vec2_t vec2_mul_scalar(vec2_t v, float s) { return (vec2_t){v.x * s, v.y * s}; }
static inline vec2_t vec2_div_scalar(vec2_t v, float s) { return (vec2_t){v.x / s, v.y / s}; }
static inline vec2_t vec2_neg(vec2_t v) { return (vec2_t){-v.x, -v.y}; }
static inline float vec2_length_sq(vec2_t v) { return v.x*v.x + v.y*v.y; }
static inline float vec2_length(vec2_t v) { return sqrtf(v.x*v.x + v.y*v.y); }
static inline float vec2_dot(vec2_t lhs, vec2_t rhs) { return lhs.x*rhs.x + lhs.y*rhs.y; }
static inline vec2_t vec2_normal(vec2_t v) { return (vec2_t){-v.y, v.x}; }
static inline vec2_t vec2_normalize(vec2_t v) { return vec2_mul_scalar(v, 1.0f / vec2_length(v)); }
static inline vec2_t vec2_lerp(vec2_t v1, vec2_t v2, float t) { return (vec2_t){lerp(v1.x, v2.x, t), lerp(v1.y, v2.y, t)}; }
static inline float vec2_dist(vec2_t v1, vec2_t v2) { return vec2_length(vec2_sub(v1, v2)); }
static inline float vec2_dist_sq(vec2_t v1, vec2_t v2) { return vec2_length_sq(vec2_sub(v1, v2)); }
static inline float vec2_angle(vec2_t v1, vec2_t v2) { return atan2(v2.y, v2.x) - atan2(v1.y, v1.x); }

static inline vec2_t vec2_scale(vec2_t v, float s) { return vec2_mul_scalar(v, s); }
static inline vec2_t vec2_translate(vec2_t v, vec2_t t) { return vec2_add(v, t); }
static inline vec2_t vec2_rotate(vec2_t v, float angle)
{
    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);
    return (vec2_t){v.x*cos_angle - v.y*sin_angle, v.x*sin_angle + v.y*cos_angle};
}

//
// Matrix maths
//

typedef struct mat3_t mat3_t;
typedef struct vec3_t vec3_t;

struct vec3_t
{
    float x;
    float y;
    float z;
};

struct mat3_t
{
    vec3_t x;
    vec3_t y;
    vec3_t z;
};

static inline vec3_t vec3_mul_scalar(vec3_t v, float scalar) { return (vec3_t){v.x*scalar, v.y*scalar, v.z*scalar}; }

static inline mat3_t mat3_scaling(float scaling)
{
    return (mat3_t){
        .x = {scaling,       0, 0},
        .y = {      0, scaling, 0},
        .z = {      0,       0, 1},
    };
}

static inline mat3_t mat3_translation(float x, float y)
{
    return (mat3_t){
        .x = {1, 0, 0},
        .y = {0, 1, 0},
        .z = {x, y, 1},
    };
}

static inline mat3_t mat3_rotation(float angle)
{
    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);
    return (mat3_t){
        .x = {cos_angle, -sin_angle, 0},
        .y = {sin_angle,  cos_angle, 0},
        .z = {        0,          0, 1},
    };
}

static inline vec3_t mat3_mul_vec(mat3_t lhs, vec3_t rhs)
{
    return (vec3_t){
        .x = lhs.x.x*rhs.x + lhs.y.x*rhs.y + lhs.z.x*rhs.z,
        .y = lhs.x.y*rhs.x + lhs.y.y*rhs.y + lhs.z.y*rhs.z,
        .z = lhs.x.z*rhs.x + lhs.y.z*rhs.y + lhs.z.z*rhs.z,
    };
}

static inline mat3_t mat3_mul(mat3_t lhs, mat3_t rhs)
{
    return (mat3_t){
        .x = mat3_mul_vec(lhs, rhs.x),
        .y = mat3_mul_vec(lhs, rhs.y),
        .z = mat3_mul_vec(lhs, rhs.z),
    };
}

static inline float mat3_det(mat3_t m)
{
    return m.x.x*m.y.y*m.z.z + m.x.y*m.y.z*m.z.x + m.x.z*m.y.x*m.z.y
        - m.x.z*m.y.y*m.z.x - m.x.y*m.y.x*m.z.z - m.x.x*m.y.z*m.z.y;
}

static inline mat3_t mat3_adjugate(mat3_t m)
{
    return (mat3_t){
        .x = {m.y.y*m.z.z - m.z.y*m.y.z, m.z.y*m.x.z - m.x.y*m.z.z, m.x.y*m.y.z - m.y.y*m.x.z},
        .y = {m.y.z*m.z.x - m.z.z*m.y.x, m.z.z*m.x.x - m.x.z*m.z.x, m.x.z*m.y.x - m.y.z*m.x.x},
        .z = {m.y.x*m.z.y - m.z.x*m.y.y, m.z.x*m.x.y - m.x.x*m.z.y, m.x.x*m.y.y - m.y.x*m.x.y}
    };
}

static inline mat3_t mat3_inverse(mat3_t m)
{
    const float inv_det = 1.f / mat3_det(m);
    const mat3_t adjugate = mat3_adjugate(m);
    return (mat3_t){
        .x = vec3_mul_scalar(adjugate.x, inv_det),
        .y = vec3_mul_scalar(adjugate.y, inv_det),
        .z = vec3_mul_scalar(adjugate.z, inv_det),
    };
}

//
// BBox maths
//

typedef struct bbox2_t bbox2_t;

struct bbox2_t
{
    vec2_t min;
    vec2_t max;
};

static inline vec2_t bbox2_center(bbox2_t b) { return vec2_mul_scalar(vec2_add(b.min, b.max), 0.5f); }
static inline bool bbox2_contain(bbox2_t b, vec2_t v) { return !(v.x < b.min.x || v.x > b.max.x || v.y < b.min.y || v.y > b.max.y); }
static inline bool bbox2_intersect(bbox2_t b1, bbox2_t b2) { return !(b1.max.x < b2.min.x || b1.min.x > b2.max.x || b1.max.y < b2.min.y || b1.min.y > b2.max.y); }
static inline vec2_t bbox2_size(bbox2_t b) { return (vec2_t){fabs(b.max.x - b.min.x), fabs(b.max.y - b.min.y)}; }

//
// Circle maths
//

typedef struct circle_t circle_t;

struct circle_t
{
    vec2_t center;
    float radius;
};

static inline bool circle_contain(circle_t c, vec2_t v) { return vec2_dist_sq(c.center, v) <= c.radius*c.radius; }
static inline bool circle_intersect(circle_t c1, circle_t c2) { return vec2_dist_sq(c1.center, c2.center) <= (c1.radius+c2.radius)*(c1.radius*c2.radius); }

//
// Ray maths
//

typedef struct ray_t ray_t;
typedef struct ray_hit_t ray_hit_t;

struct ray_t
{
    vec2_t o;
    // The ray direction is assumed normalized for all the operations.
    vec2_t dir;
};

struct ray_hit_t
{
    float t;
    bool valid;
};

static inline ray_t to_ray(vec2_t from, vec2_t to) { return (ray_t){from, vec2_normalize(vec2_sub(to, from))}; }
static inline ray_hit_t ray_circle_intersect(ray_t r, circle_t c)
{
    const float radius_sq = c.radius*c.radius;
    const vec2_t to_center = vec2_sub(c.center, r.o);
    const float dist_sq = vec2_length_sq(to_center);
    const float t_proj = vec2_dot(to_center, r.dir);
    const float d_sq = dist_sq - t_proj*t_proj;
    if (d_sq > radius_sq) return (ray_hit_t){.valid = false};
    const float td = sqrtf(radius_sq - d_sq);
    return (ray_hit_t){t_proj - td, true};
}

#endif // LINALG_H_


