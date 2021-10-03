#ifndef WORLD_H_
#define WORLD_H_

typedef struct world_t world_t;

struct world_t
{
    struct
    {
        float north;
        float south;
        float east;
        float west;
    } bounds;
};

#endif // WORLD_H_


