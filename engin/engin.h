#ifndef ENGIN_H
#define ENGIN_H


#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


#include "sets.h"
#include "da.h"
#include "alf.h"

#include "print.h"
#define STRING_FILEIO
#include "Str.h"
#include "err.h"


#define W 16
#define H 16



typedef struct Pos
{
    union {
        struct {
            int x;
            int y;
        };
        uint64_t packed;
    };
} Pos;
DA_TYPEDEF_ARRAY(Pos);



#define POS_Y_MASK 0b00001111
#define POS_Y_UNIT 0b00000001
#define POS_X_MASK 0b11110000
#define POS_X_UNIT 0b00010000

#define POS_CHUNK(x, y) (((x) << 4) | (y))
#define POS_CHUNK_X(pos) ((pos) >> 4)
#define POS_CHUNK_Y(pos) ((pos) & 0b1111)


typedef uint8_t Pos_Chunk;
DA_TYPEDEF_ARRAY(Pos_Chunk);

typedef struct Pos_Globale
{
    Pos chunk;
    Pos_Chunk cell;
} Pos_Globale;


typedef uint8_t Cell;
typedef struct World World;
typedef struct Chunk Chunk;
typedef struct Chunk
{
    union { // 1 * 64bytes 
        Cell arr[H*W];
        Cell arr2D[H][W];
    };

    Pos pos; // 8bytes
    
    // up right down left
    Chunk *close_chunks[4]; // 32bytes
    World *parent; // 8bytes
    
    // cables to light off
    da_Pos_Chunk cables; // 16bytes
    // transistors to light off
    da_Pos_Chunk update; // 16bytes
    // transistors to propagate current
    da_Pos_Chunk light_transistors[2]; // 2*16bytes
    
    // all not gate ; if arr[_] & VAR_MASK != 0 could not propagate current
    da_Pos_Chunk not_gate; // 16bytes
} Chunk;

static_assert(sizeof((Chunk){0}.arr) == sizeof((Chunk){0}.arr2D));
static_assert(sizeof((Chunk){0}.arr) == sizeof(Cell) * W * H);
static_assert(sizeof(Chunk) == 384, "");
// 256-512                4 * 64 + 138 = 256 + 128 = 386 = 6 * 64

// 2 -> 0b01010 0
// 4 -> 0b0101 00

ALF_TYEPDEF(Chunk, 64, 8 /*  1 << 8 == 256  */);
// ALF_TYEPDEF(Chunk, 0, 2 /*  1 << 8 == 256  */);



typedef enum
{
    empty               = 0b00000000,
    cable_on            = 0b00010000,
    cable_off           = 0b00100000,
    bridge              = 0b00110000,

    u_transistor_off    = 0b01000000,
    r_transistor_off    = 0b01000001,
    d_transistor_off    = 0b01000010,
    l_transistor_off    = 0b01000011,

    u_transistor_o      = 0b01000100,
    r_transistor_o      = 0b01000101,
    d_transistor_o      = 0b01000110,
    l_transistor_o      = 0b01000111,
    
    u_transistor_on     = 0b01001000,
    r_transistor_on     = 0b01001001,
    d_transistor_on     = 0b01001010,
    l_transistor_on     = 0b01001011,
    
    
    u_not_gate          = 0b01010000,
    r_not_gate          = 0b01010001,
    d_not_gate          = 0b01010010,
    l_not_gate          = 0b01010011,
    
    u_not_gate__o       = 0b01010100,
    r_not_gate__o       = 0b01010101,
    d_not_gate__o       = 0b01010110,
    l_not_gate__o       = 0b01010111,

    u_not_gate_o        = 0b01011000,
    r_not_gate_o        = 0b01011001,
    d_not_gate_o        = 0b01011010,
    l_not_gate_o        = 0b01011011,
    
    u_not_gate_oo       = 0b01011100,
    r_not_gate_oo       = 0b01011101,
    d_not_gate_oo       = 0b01011110,
    l_not_gate_oo       = 0b01011111
} State;

// 000 00 00
// 
#define TYPE_MASK 0b11110000
#define VARIANT_MASK 0b00001100
#define DIRECTION_MASK 0b00000011
#define CONNECTION_MASK 0b00001111

#define UP_MASK     (1 << up)
#define RIGHT_MASK  (1 << right)
#define DOWN_MASK   (1 << down)
#define LEFT_MASK   (1 << left)

typedef enum state_type
{
    ty_empty        = TYPE_MASK & empty,            // 0b00000000,
    ty_cable_off    = TYPE_MASK & cable_off,        // 0b00010000,
    ty_cable_on     = TYPE_MASK & cable_on,         // 0b00010000,
    ty_transistor   = TYPE_MASK & u_transistor_off, // 0b00100000,
    ty_bridge       = TYPE_MASK & bridge,           // 0b00110000,
    ty_not_gate     = TYPE_MASK & u_not_gate,       // 0b01000000,
} state_type;
typedef enum state_variant
{
    var_off = 0b00000000,
    var_o   = 0b00000100,
    var_on  = 0b00001000,
} state_variant;
typedef enum Direction
{
    up      = 0b00,
    right   = 0b01,
    down    = 0b10,
    left    = 0b11
} Direction;


typedef struct 
{
    Pos pos;
    Chunk *data;
} Item_chunks;

extern void Chunk_print(const Chunk*);
__attribute_maybe_unused__ static void Item_chunks_print(const Item_chunks value)
{
    if (value.data)
        Chunk_print(value.data);
    else printf("No chunk");
}
__attribute_maybe_unused__ static uint64_t Item_chunks_hash(const Item_chunks value, uint64_t seed) 
{
    return value.pos.packed ^ (seed << 1);
}
__attribute_maybe_unused__ static int Item_chunks_equal(const Item_chunks value1, const Item_chunks value2) 
{
    return value1.pos.packed == value2.pos.packed;
}


#define IS_IC_NULL(value) ((value).data == NULL)
#define SET_IC_NULL(value) ((value).data = NULL)
SET_TYPEDEF_HASH_SET(
    Item_chunks, IS_IC_NULL, SET_IC_NULL,
    64, 4, 0.5,
    Item_chunks_hash, Item_chunks_equal, Item_chunks_print
);

typedef struct World
{
    char *name;

    alf_Chunk *chunk_allocator;
    set_Item_chunks chunks;
    bool state;
} World;


World World_make(const char *name);


void World_tick(World *obj);
void Chunk_print(const Chunk *obj);
// void World_basic_print(const World *obj, bool more);

void World_set_cell(World *obj, Pos pos_world, Pos_Chunk pos_chunk, State value);
void Chunk_set_cell(Chunk *obj, Pos_Chunk pos, State value);
Chunk *add_Chunk(World *parent, Pos pos);
bool erase_Chunk(Chunk *obj);
bool erase_Chunk_pos(World *parent, Pos pos);
void World_free(World *obj);




ERR_TYPEDEF(World, char);
err_World_char load_World(const char *file);
char *save_World(World *obj, const char *file);

void save_patern(const World *obj, const char *file, const char *head_msg, Pos_Globale pos1, Pos_Globale pos2);
void add_patern(World *obj, Pos_Globale pos, const char *file);

#endif /* ENGIN_H */
