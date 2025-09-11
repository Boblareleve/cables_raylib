#include <assert.h>
// #include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct Vec2
{
    float x,y;
} Vec2;
typedef struct Rect
{
    union {
        Vec2 pos;
        struct {
            float x, y;
        };
    };
    union {
        Vec2 dim;
        struct {
            float width, height;
        };
    };
} Rect;
typedef struct Camera
{
    // Vec2 offset;            // Camera offset (displacement from target)
    Vec2 target;         // Camera target (rotation and zoom origin)
    float zoom;             // Camera zoom (scaling), should be 1.0f by default
} Camera;


static inline bool is_Vec2_colliding_Rect(Vec2 point, Rect rec)
{
    return (
        (point.x >= rec.x) && (point.x <= (rec.x + rec.width))
     && (point.y >= rec.y) && (point.y <= (rec.y + rec.height))
    );
}

static inline float v2_dot_prod(const Vec2 a, const Vec2 b)
{
    assert(!isnan(a.x));
    assert(!isnan(a.y));
    assert(!isnan(b.x));
    assert(!isnan(b.y));
    float res = a.x * b.x + a.y * b.y;
    assert(!isnan(res));
    return (res);
}
static inline Vec2 v2_add(const Vec2 a, const Vec2 b)
{
    assert(!isnan(a.x));
    assert(!isnan(a.y));
    assert(!isnan(b.x));
    assert(!isnan(b.y));
    Vec2 res = (Vec2){a.x + b.x, a.y + b.y};
    assert(!isnan(res.x));
    assert(!isnan(res.y));
    return (res);
}
static inline Vec2 v2_diff(const Vec2 a, const Vec2 b)
{ 
    assert(!isnan(a.x));
    assert(!isnan(a.y));
    assert(!isnan(b.x));
    assert(!isnan(b.y));
    Vec2 res = (Vec2){a.x - b.x, a.y - b.y};
    assert(!isnan(res.x));
    assert(!isnan(res.y));
    return (res);
}
static inline float v2_lenght(const Vec2 a)
{
    assert(!isnan(a.x));
    assert(!isnan(a.y));
    assert(a.x * a.x + a.y * a.y >= 0.);
    float res = sqrt(a.x * a.x + a.y * a.y);
    assert(!isnan(res));
    return (res);
}
static inline float v2_dist(const Vec2 a, const Vec2 b)
{
    assert(!isnan(a.x));
    assert(!isnan(a.y));
    assert(!isnan(b.x));
    assert(!isnan(b.y));
    float res = v2_lenght(v2_diff(b, a));
    assert(!isnan(res));
    return (res);
}
static inline Vec2 v2_scal_prod(const float lambda, const Vec2 v)
{
    assert(!isnan(lambda));
    assert(!isnan(v.x));
    assert(!isnan(v.y));
    Vec2 res = (Vec2){lambda * v.x, lambda * v.y};
    assert(!isnan(res.x));
    assert(!isnan(res.y));
    return (res);
}

// (Vec2){0} => (Vec2){0, 1}
static inline Vec2 v2_normalize(const Vec2 v)
{
    assert(!isnan(v.x));
    assert(!isnan(v.y));
    if (v.x == 0. && v.y == 0)
        return (Vec2){0., 1.};
    
    float lenght = v2_lenght(v);
    Vec2 res = v2_scal_prod(1./lenght, v);
    assert(!isnan(res.x));
    assert(!isnan(res.y));
    return (res);
}

#ifdef PRINT_H
static inline char *v2_sprint(const Vec2 v)
{
    assert(print_buffer_size + 4+2*10 < BUFFER_PRINT_SIZE);

    char *res = &print_buffer[print_buffer_size];
    print_buffer_size += sprintf(res, "(%.2f,%.2f)", v.x, v.y) + 1 /* '\0' */; 
    
    return (res);
}
#endif /* PRINT_H */
static inline void v2_print(const Vec2 v)
{
    printf("(%.2f,%.2f)", v.x, v.y); 
}

// rand in [a, b]
static inline Vec2 v2_rand(float a, float b)
{
    assert(!isnan(b-a));
    return (Vec2){
        a + (float)rand()/(float)((float)RAND_MAX/(b-a)),
        a + (float)rand()/(float)((float)RAND_MAX/(b-a))
    };
}
