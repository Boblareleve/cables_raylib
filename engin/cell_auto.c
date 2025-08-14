#include "engin.h"


void Chunk_print(const Chunk *chunk)
{
    for (int y = H - 1; y >= 0; y--)
    {
        for (int x = 0; x < W; x++)
        {
                 if ((chunk->arr2D[x][y] & TYPE_MASK) == empty)
                printf(".");
            else if ((chunk->arr2D[x][y] & TYPE_MASK) == cable_off)
                printf("¤");
            else if ((chunk->arr2D[x][y] & TYPE_MASK) == cable_on)
                printf("X");
            else if ((chunk->arr2D[x][y] & TYPE_MASK) == bridge)
                printf("#");
            else if ((chunk->arr2D[x][y] & TYPE_MASK) == ty_transistor)
            {
                if ((chunk->arr2D[x][y] & VARIANT_MASK) == var_off)
                    printf("t");
                else if ((chunk->arr2D[x][y] & VARIANT_MASK) == var_o)
                    printf("T");
                else if ((chunk->arr2D[x][y] & VARIANT_MASK) == var_on)
                    printf("O");
            }
            else if ((chunk->arr2D[x][y] & TYPE_MASK) == ty_not_gate)
            {
                if (chunk->parent->state ? (chunk->arr2D[x][y] & VARIANT_MASK) == var_o
                                      : (chunk->arr2D[x][y] & VARIANT_MASK) == var_on)
                    printf("N");
                else printf("n");
            }
            else printf(" ");
            printf(" ");
            
        }
        /* if (more)
            Chunk_sec_line_print(chunk, y); 
        else printf("\n"); */
    }
    for (int i = 0; i < W; i++)
        printf("--");
    printf("\n");
}


#define __OPPOSITE_DIR(dir) (((dir) + 2) % 4)
static inline Direction opposite_dir(Direction dir)
{
    return __OPPOSITE_DIR(dir);
}
static_assert(up == __OPPOSITE_DIR(down), "up -> down");
static_assert(down == __OPPOSITE_DIR(up), "down -> up");
static_assert(left == __OPPOSITE_DIR(right), "left -> right");
static_assert(right == __OPPOSITE_DIR(left), "right -> left");

/* Pos P*os_Globale_to_Pos(P*os_Globale pos)
{
    return (Pos){
        .x = pos.chunk.x * W + POS_CHUNK_X(pos.cell),
        .y = pos.chunk.y * H + POS_CHUNK_Y(pos.cell)
    };
}
P*os_Globale Pos_to_P*os_Globale(Pos pos)
{
    return (P*os_Globale){
        .chunk = (Pos){
            .x = (pos.x >> 4),
            .y = (pos.y >> 4)
        },
        .cell = ((pos.x & 0xf) << 4)
              |  (pos.y & 0xf)
    };
} */



World World_make(const char *name)
{
    World res = {
        .chunks = set_Item_chunks_make(),
        .state = 0,
        .name = strdup(name),
        .chunk_allocator = calloc(1, sizeof(alf_Chunk))
    };
    return res;
}

Chunk *add_Chunk(World *parent, Pos pos)
{
    Item_chunks *build = set_Item_chunks_insert(
        &parent->chunks,
        (Item_chunks){ 
            .pos = pos,
            .data = alf_Chunk_alloc(parent->chunk_allocator)
        }
    );
    if (build == NULL)
        return NULL;
    assert(build->data);

    build->data->pos = pos;
    build->data->parent = parent;
    
    memset(build->data->arr, 0, sizeof(build->data->arr[0]) * W * H);

    Pos table[4] = {
        (Pos){ .x = 0,  .y = 1 },
        (Pos){ .x = 1,  .y = 0 },
        (Pos){ .x = 0,  .y = -1},
        (Pos){ .x = -1, .y = 0 },
    };
    for (Direction dir = up; dir <= left; dir++)
    {
        Item_chunks *neighbour_chunk = set_Item_chunks_get(
            &parent->chunks, 
            (Item_chunks){ .pos = (Pos) { 
                .x = build->pos.x + table[dir].x, 
                .y = build->pos.y + table[dir].y
            }}
        );
        if (neighbour_chunk == NULL)
            build->data->close_chunks[dir] = NULL;
        else
        {
            build->data->close_chunks[dir] = neighbour_chunk->data;
            build->data->close_chunks[dir]->close_chunks[opposite_dir(dir)] = build->data;
        }
    }

    build->data->light_transistors[0] = (da_Pos_Chunk){0};
    build->data->light_transistors[1] = (da_Pos_Chunk){0};
    build->data->update = (da_Pos_Chunk){0};
    build->data->cables = (da_Pos_Chunk){0};
    build->data->not_gate = (da_Pos_Chunk){0};
    
    int i = 0;
    while (i < 256)
    {
        if ((build->data->arr[i] & TYPE_MASK) == ty_not_gate)
            da_push(&build->data->not_gate, (Pos_Chunk)i);
        i++;
    }
    return build->data;
}

bool erase_Chunk(Chunk *chunk)
{
    if (!set_Item_chunks_erase(&chunk->parent->chunks, (Item_chunks){ .pos =  chunk->pos }))
        return false;

    for (Direction dir = up; dir <= left; dir++)
        if (chunk->close_chunks[dir])
            chunk->close_chunks[dir]->close_chunks[opposite_dir(dir)] = NULL;
    
    alf_Chunk_release(chunk->parent->chunk_allocator, chunk);
    /* TODO: if added set of color unreference it too */
    return true;
}

bool erase_Chunk_pos(World *parent, Pos pos)
{
    Item_chunks *item_chunk = set_Item_chunks_get(
        &parent->chunks,
        (Item_chunks){ .pos = pos }
    );
    if (!item_chunk->data)
        return false;
    return erase_Chunk(item_chunk->data);
}

static inline Pos_Chunk mouv_in_chunk_up(Chunk **chunk, Pos_Chunk pos)
{
    if ((pos & POS_Y_MASK) == H - 1)
    {
        *chunk = (*chunk)->close_chunks[up];
        return (pos & POS_X_MASK) | 0;
    }
    return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) + POS_Y_UNIT);
}
static inline Pos_Chunk mouv_in_chunk_right(Chunk **chunk, Pos_Chunk pos)
{
    if (POS_CHUNK_X(pos) >= W - 1)
    {
        *chunk = (*chunk)->close_chunks[right];
        return 0 | (pos & POS_Y_MASK);
    }
    return ((pos & POS_X_MASK) + POS_X_UNIT) | (pos & POS_Y_MASK);
}
static inline Pos_Chunk mouv_in_chunk_down(Chunk **chunk, Pos_Chunk pos)
{
    if ((pos & POS_Y_MASK) <= 0)
    {
        *chunk = (*chunk)->close_chunks[down];
        return (pos & POS_X_MASK) | (H - 1);
    }
    return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) - POS_Y_UNIT);
}
static inline Pos_Chunk mouv_in_chunk_left(Chunk **chunk, Pos_Chunk pos)
{
    if (POS_CHUNK_X(pos) == 0)
    {
        *chunk = (*chunk)->close_chunks[left];
        return ((W - 1) << 4) | (pos & POS_Y_MASK);
    }
    return ((pos & POS_X_MASK) - POS_X_UNIT) | (pos & POS_Y_MASK);
}
// chunk{IO} :|
static inline Pos_Chunk mouv_in_chunk(Chunk **chunk, Pos_Chunk pos, Direction dir)
{
    if (0) printf("mouv in %p: x%u y%u \tdir: %d\n", 
        (void*)*chunk, 
        (int)POS_CHUNK_X(pos),
        (int)POS_CHUNK_Y(pos),
        dir
    );
    assert(chunk);

    switch (dir)
    {
    case up:
        // if ((pos & POS_Y_MASK) == H - 1)
        // {
        //     *chunk = (*chunk)->close_chunks[up];
        //     return (pos & POS_X_MASK) | 0;
        // }
        // return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) + POS_Y_UNIT);
        return mouv_in_chunk_up(chunk, pos); 
    case right:
        // if (POS_CHUNK_X(pos) >= W - 1)
        // {
        //     *chunk = (*chunk)->close_chunks[right];
        //     return 0 | (pos & POS_Y_MASK);
        // }
        // return ((pos & POS_X_MASK) + POS_X_UNIT) | (pos & POS_Y_MASK);
        return mouv_in_chunk_right(chunk, pos); 
    case down:
        // if ((pos & POS_Y_MASK) <= 0)
        // {
        //     *chunk = (*chunk)->close_chunks[down];
        //     return (pos & POS_X_MASK) | (H - 1);
        // }
        // return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) - POS_Y_UNIT);
        return mouv_in_chunk_down(chunk, pos);
    case left:
        // if (POS_CHUNK_X(pos) == 0)
        // {
        //     *chunk = (*chunk)->close_chunks[left];
        //     return ((W - 1) << 4) | (pos & POS_Y_MASK);
        // }
        // return ((pos & POS_X_MASK) - POS_X_UNIT) | (pos & POS_Y_MASK);
        return mouv_in_chunk_left(chunk, pos);
    default:
        *chunk = NULL;
        return pos;
    }
}



static void light_cable(Chunk *chunk, Pos_Chunk pos, Direction dir)
{
    static int light_pile_height = 0;
    
    if (0) printf("light(%d) %p: x%u y%u \tdir: %d\n", light_pile_height,
        (void*)chunk, 
        (int)POS_CHUNK_X(pos),
        (int)POS_CHUNK_Y(pos),
        dir
    );
    light_pile_height++;

    // not a populated Chunk
    if (chunk == NULL)
        ;
    else if ((chunk->arr[pos] & TYPE_MASK) == cable_off)
    {
        chunk->arr[pos] = cable_on | (CONNECTION_MASK & chunk->arr[pos]);

        da_push(&chunk->cables, pos);

        Chunk *tmp = chunk;
        Pos_Chunk next_pos = mouv_in_chunk_up(&tmp, pos);
        light_cable(tmp, next_pos, up);
        tmp = chunk; next_pos = mouv_in_chunk_right(&tmp, pos);
        light_cable(tmp, next_pos, right);
        tmp = chunk; next_pos = mouv_in_chunk_down(&tmp, pos);
        light_cable(tmp, next_pos, down);
        tmp = chunk; next_pos = mouv_in_chunk_left(&tmp, pos);
        light_cable(tmp, next_pos, left);

    }
    else if ((chunk->arr[pos] & TYPE_MASK) == bridge)
    {
        Chunk *tmp = chunk;
        Pos_Chunk next_pos = mouv_in_chunk(&tmp, pos, dir);
        light_cable(tmp, next_pos, dir);
    }
    else if ((chunk->arr[pos] & TYPE_MASK) == ty_transistor)
    {
        if (dir != opposite_dir(chunk->arr[pos] & DIRECTION_MASK))
        {
            if ((chunk->arr[pos] & VARIANT_MASK) == var_off)
            {
                chunk->arr[pos] += var_o;
                da_push(&chunk->update, pos);
            }
            else if ((chunk->arr[pos] & VARIANT_MASK) == var_o)
            {
                chunk->arr[pos] += var_o;
                da_push(&chunk->light_transistors[!chunk->parent->state], pos);
            }
        }
    }
    else if ((chunk->arr[pos] & TYPE_MASK) == ty_not_gate)
    {
        if (dir != opposite_dir(chunk->arr[pos] & DIRECTION_MASK))
        {
            if (0) printf("not gate alimented\n");
            chunk->arr[pos] |= !chunk->parent->state ? 0b0100 /* var_o */ : 0b1000/* var_on */;
        }
        else if (0) printf("no flow back\n");
    }
    
    light_pile_height--;
}



void Chunk_cable_tick(Chunk *chunk)
{
    while (chunk->cables.size > 0)
    {
        Pos_Chunk pos_cable = da_pop(&chunk->cables);
        chunk->arr[pos_cable] = cable_off | (CONNECTION_MASK & chunk->arr[pos_cable]);
    }
}

void Chunk_update_tick(Chunk *chunk)
{
    while (chunk->update.size > 0)
    {
        Pos_Chunk pos = da_pop(&chunk->update);
        
        // if (chunk->arr[pos] & VARIANT_MASK )  //(chunk->arr[pos] - u_transistor_off) % 3)
            //chunk->arr[pos] - (chunk->arr[pos] - u_transistor_off) % 3;
        // also reinitilize var_on
        chunk->arr[pos] &= ~VARIANT_MASK; 
    }
}

void Chunk_light_transistor_tick(Chunk *chunk)
{
    while (chunk->light_transistors[chunk->parent->state].size > 0)
    {
        Pos_Chunk pos = da_pop(&chunk->light_transistors[chunk->parent->state]);
        Chunk *tmp = chunk;

        light_cable(
            tmp, 
            mouv_in_chunk(&tmp, pos, chunk->arr[pos] & DIRECTION_MASK),
            chunk->arr[pos] & DIRECTION_MASK
        );
    }
}


// 0b 11 00 00
void Chunk_not_gate_tick(Chunk *chunk)
{
    // not a 'while' bc the array is readonly
    foreach_ptr (Pos_Chunk, it, &chunk->not_gate)
    {
        assert(chunk->parent->state == 0 || chunk->parent->state == 1);
        if (((chunk->arr[*it] & VARIANT_MASK) & (chunk->parent->state ? var_o : var_on)) == 0)
        {
            if (0) printf("not gate light out\n");
            light_cable(
                chunk,
                mouv_in_chunk(
                    &chunk, 
                    *it,
                    chunk->arr[*it] & DIRECTION_MASK
                ),
                chunk->arr[*it] & DIRECTION_MASK
            );
        }
        if (0) printf(" notgate (%02x): %02x\n", *it, chunk->arr[*it]);
    }
}

void World_tick(World *wrd)
{
    if (0) {
        printf("\n\nstart tick (%d):\n", wrd->state);
        for_set (Item_chunks, item, &wrd->chunks)
            foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
                printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    }
    
    for_set (Item_chunks, item, &wrd->chunks)
        foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
            printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    for_set (Item_chunks, item, &wrd->chunks)
        foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
            item->data->arr[*pos] &= !wrd->state ? ~var_o : ~var_on;

    // light off all cable
    for_set (Item_chunks, item, &wrd->chunks)
        Chunk_cable_tick(item->data);
    
    // light off every transistor (var_o and var_on)
    for_set (Item_chunks, item, &wrd->chunks)
        Chunk_update_tick(item->data);
    
    // pass current old light transistor
    for_set (Item_chunks, item, &wrd->chunks)
        Chunk_light_transistor_tick(item->data);
    
    // not gate 
    for_set (Item_chunks, item, &wrd->chunks)
        Chunk_not_gate_tick(item->data);

    if (0) {
        printf("end tick:\n");
        for_set (Item_chunks, item, &wrd->chunks)
            foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
                printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    }
    wrd->state = !wrd->state;
}

static inline bool is_connectable_cell(Cell cell)
{
    return ((cell & TYPE_MASK) == cable_off)
         | ((cell & TYPE_MASK) == cable_on)
         | ((cell & TYPE_MASK) == bridge);
}

static inline bool update_cells_connections(Chunk *chunk, const Pos_Chunk pos)
{
    if ((chunk->arr[pos] & TYPE_MASK) == cable_off
     || (chunk->arr[pos] & TYPE_MASK) == cable_on
    ) {
        for (Direction dir = up; dir <= left; dir++)
        {
            Chunk *new_chunk = chunk;
            Pos_Chunk new_pos = mouv_in_chunk(&new_chunk, pos, dir);
            // assert(new_chunk == chunk);
            
            if (new_chunk && (new_chunk->arr[new_pos] & TYPE_MASK) != empty)
            {
                chunk->arr[pos] |= 0b1 << dir;
                if ((new_chunk->arr[new_pos] & TYPE_MASK) == cable_off
                 || (new_chunk->arr[new_pos] & TYPE_MASK) == cable_on) 
                    new_chunk->arr[new_pos] |= 0b1 << opposite_dir(dir);
            }
        }
    }
    else if ((chunk->arr[pos] & TYPE_MASK) == empty)
    {
        assert(empty == chunk->arr[pos]);

        for (Direction dir = up; dir <= left; dir++)
        {
            Chunk *new_chunk = chunk;
            Pos_Chunk new_pos = mouv_in_chunk(&new_chunk, pos, dir);

            if (!new_chunk)
                continue;
            if ((new_chunk->arr[new_pos] & TYPE_MASK) == cable_off
              ||(new_chunk->arr[new_pos] & TYPE_MASK) == cable_on
              ||(new_chunk->arr[new_pos] & TYPE_MASK) == bridge)
                new_chunk->arr[new_pos] &= ~(1 << opposite_dir(dir));
            // else if ((new_chunk->arr[new_pos] & TYPE_MASK) == bridge)
            //     assert(0);
            
        }
    }
    else if ((chunk->arr[pos] & TYPE_MASK) == bridge)
    {
        Chunk *up_chunk = chunk;
        const Pos_Chunk up_pos = mouv_in_chunk_up(&up_chunk, pos);
        Chunk *down_chunk = chunk;
        const Pos_Chunk down_pos = mouv_in_chunk_down(&down_chunk, pos);
       
        if (up_chunk && (up_chunk->arr[up_pos] & TYPE_MASK) != empty)
        {
            if (down_chunk && (down_chunk->arr[down_pos] & TYPE_MASK) != empty)
            {
                chunk->arr[pos] |= 1 << up | 1 << down;
                if (is_connectable_cell(up_chunk->arr[up_pos]))
                    up_chunk->arr[up_pos] |= 1 << down;
                if (is_connectable_cell(down_chunk->arr[down_pos]))
                    down_chunk->arr[down_pos] |= 1 << up;
            }
            
            /* else if ((up_chunk->arr[up_pos] & TYPE_MASK) != bridge
             || (up_chunk->arr[up_pos] & DOWN_MASK))
            {
                
            }
            else 
            {

            } */
        }

        Chunk *left_chunk = chunk;
        const Pos_Chunk left_pos = mouv_in_chunk_left(&left_chunk, pos);
        Chunk *right_chunk = chunk;
        const Pos_Chunk right_pos = mouv_in_chunk_right(&right_chunk, pos);
        if (right_chunk && ((right_chunk->arr[right_pos] & TYPE_MASK) != empty))
        {
            if (left_chunk && (left_chunk->arr[left_pos] & TYPE_MASK) != empty)
            {
                chunk->arr[pos] |= (1 << left) | (1 << right);
                if (is_connectable_cell(left_chunk->arr[left_pos]))
                    left_chunk->arr[left_pos] |= (1 << right);
                if (is_connectable_cell(right_chunk->arr[right_pos]))
                    right_chunk->arr[right_pos] |= (1 << left);
            }
            
            //
        }
        
        /* if (((left_chunk->arr[left_pos] & TYPE_MASK) != empty))
            ;
        { // if a bridge with down connection or not an empty

            if (((down_chunk->arr[up_pos] & TYPE_MASK) == empty))
                ;
            else if ((down_chunk->arr[down_pos] & TYPE_MASK) != bridge
                || (down_chunk->arr[down_pos] & UP_MASK))
            {
                chunk->
            }
        } */
    }

    return true;
}

/* static inline void update_cells_connections(Chunk *chunk, Pos_Chunk pos, Cell old_value, Cell value)
{
    if ((value & TYPE_MASK) == empty)
    {
        if (0) for (Direction dir = up; dir <= left; dir++)
        {
            Chunk *chunk_mouv = chunk;
            Pos_Chunk pos_mouv = mouv_in_chunk(&chunk_mouv, pos, dir);
            if (chunk_mouv == NULL)
                continue;
            
            if ((chunk_mouv->arr[pos_mouv] & TYPE_MASK) == cable_off
             || (chunk_mouv->arr[pos_mouv] & TYPE_MASK) == cable_on
            ) {
                // printf("~> %u -> %u\n", 
                //     chunk_mouv->arr[pos_mouv], 
                //     chunk_mouv->arr[pos_mouv] & ~(1 << opposite_dir(dir))
                // );
                chunk_mouv->arr[pos_mouv] &= ~(1 << opposite_dir(dir));
            }
            else if ((chunk_mouv->arr[pos_mouv] & TYPE_MASK) == bridge)
            {
                do {
                    // printf("~> %u -> %u\n", 
                    //     chunk_mouv->arr[pos_mouv], 
                    //     chunk_mouv->arr[pos_mouv] & ~(1 << opposite_dir(dir))
                    // );
                    chunk_mouv->arr[pos_mouv] &= ~(1 << opposite_dir(dir));
                    pos_mouv = mouv_in_chunk(&chunk_mouv, pos_mouv, dir);
                    
                } while (chunk_mouv && (chunk_mouv->arr[pos_mouv] & TYPE_MASK) == bridge);
                
                // maybe some redundancy can be avoided there
                if (chunk_mouv == NULL)
                    continue;
                
                if ((chunk_mouv->arr[pos_mouv] & TYPE_MASK) == cable_off
                 || (chunk_mouv->arr[pos_mouv] & TYPE_MASK) == cable_on
                ) {
                    // printf("~> %u -> %u\n", 
                    //     chunk_mouv->arr[pos_mouv], 
                    //     chunk_mouv->arr[pos_mouv] & ~(1 << opposite_dir(dir))
                    // );
                    chunk_mouv->arr[pos_mouv] &= ~(1 << opposite_dir(dir));
                }
            }
        }
    }
    
    else if ((value & TYPE_MASK) == cable_off 
          || (value & TYPE_MASK) == cable_on
    ) {
        for (Direction dir = up; dir <= left; dir++)
        {
            Chunk *chunk_mouv = chunk;
            Pos_Chunk pos_mouv = mouv_in_chunk(&chunk_mouv, pos, dir);
            if (chunk_mouv == NULL
             || (chunk_mouv->arr[pos_mouv] & TYPE_MASK) == empty)
            {
                // printf("~> %u -> ", chunk->arr[pos]);
                // chunk->arr[pos] &= ~(1 << dir);
                // printf(" %u\n", chunk->arr[pos]);
            }
            else if (((chunk_mouv->arr[pos_mouv] & TYPE_MASK) == bridge))
            {
                // if (old_value == empty)
                //     assert(0);
                // else
                //     chunk->arr[pos] |= 0 ? (1 << dir) : 0;
            }
            else
            {
                
                printf("|> %u -> ", chunk->arr[pos]);
                chunk->arr[pos] |= (1 << (dir));
                printf("%u\tdir %d\n", chunk->arr[pos], dir);

                printf(" |> %u -> ", chunk_mouv->arr[pos_mouv]);
                chunk_mouv->arr[pos_mouv] |= (1 << opposite_dir(dir));
                printf("%u\n", chunk_mouv->arr[pos_mouv]);
            }
            // else ;// assert(0);
        }
    }
    else if ((value & TYPE_MASK) == bridge)   // bridge have connection info for 
                                              // calculation speedup (avoid a while loop just underthere ; I made you said underwhere :P)
    {
        // assert(0);
    }
} */

void Chunk_delete_cell(Chunk *chunk, Pos_Chunk pos)
{
    assert(chunk);

    if ((chunk->arr[pos] & TYPE_MASK) == cable_on)
       da_erase_value(&chunk->cables, pos);
    else if ((chunk->arr[pos] & TYPE_MASK) == ty_transistor)
    {
        if ((chunk->arr[pos] & VARIANT_MASK) != var_off)
            da_erase_value(&chunk->update, pos);
        if ((chunk->arr[pos] & VARIANT_MASK) == var_on)
        {
            da_erase_value(&chunk->light_transistors[0], pos);
            da_erase_value(&chunk->light_transistors[1], pos);
        }
    }
    else if ((chunk->arr[pos] & TYPE_MASK) == ty_not_gate)
        da_erase_value(&chunk->not_gate, pos);
    chunk->arr[pos] = empty;
}

bool Chunk_set_cell(Chunk *chunk, Pos_Chunk pos, State value)
{
    if (value == chunk->arr[pos])
        return true;
    
    // Cell old_value = chunk->arr[pos];
    Chunk_delete_cell(chunk, pos);
    chunk->arr[pos] = value;

    if ((value & TYPE_MASK) == ty_transistor)
    {
        if ((value & VARIANT_MASK) >= var_o)
            da_push(&chunk->update, pos);
        if ((value & VARIANT_MASK) == var_on)
            da_push(&chunk->light_transistors[chunk->parent->state], pos);
    }
    else if ((value & TYPE_MASK) == ty_not_gate)
        da_push(&chunk->not_gate, pos);
    else if ((value & TYPE_MASK) == cable_on)
        da_push(&chunk->cables, pos);
    
    return update_cells_connections(chunk, pos);
}

Chunk *get_Chunk_by_Pos(World *wrd, Pos pos)
{
    Item_chunks *item_chunk = set_Item_chunks_get(
        &wrd->chunks, 
        (Item_chunks){ .pos = get_chunk_Pos(pos) }
    );
    if (!item_chunk)
        return NULL;
    return item_chunk->data;
}
bool World_set_cell(World *wrd, Pos pos, State value)
{
    Chunk *chunk = get_Chunk_by_Pos(wrd, pos);
    if (chunk == NULL)
    {
        if ((value & TYPE_MASK) == empty)
            return true;

        chunk = add_Chunk(wrd, get_chunk_Pos(pos));
    }
    assert(chunk);
    
    return Chunk_set_cell(chunk, get_cell_pos(pos), value);
}
bool World_set_orth_lign_cell(World *wrd, Pos pos1, Pos pos2, State value)
{
    if (pos1.x == pos2.x
     && pos1.y == pos2.y)
        return World_set_cell(wrd, pos1, value);

    if (pos1.x > pos2.x) SWAP(pos1.x, pos2.x);
    if (pos1.y > pos2.y) SWAP(pos1.y, pos2.y);
    
    Pos chunk_pos1 = get_chunk_Pos(pos1);
    Pos_Chunk cell_pos1 = get_cell_pos(pos1);

    Item_chunks *item_chunk = set_Item_chunks_get(&wrd->chunks, (Item_chunks){ .pos = chunk_pos1 });
    Chunk *chunk = (item_chunk == NULL) ? add_Chunk(wrd, chunk_pos1)
                                        : item_chunk->data;
    
    
    bool res = true;
    bool axis = ABS(pos1.x - pos2.x) >= ABS(pos1.y - pos2.y); // true -> x
    assert(axis == 0 || axis == 1);

    if ( axis && pos1.x > pos2.x)
    {
        SWAP(pos1, pos2);
        pos1.x++;
        pos2.x--;
    }
    if (!axis && pos1.y > pos2.y)
    {
        SWAP(pos1, pos2);
        pos1.y++;
        pos2.y--;
    }

    
    Pos_Chunk i = cell_pos1;
    
    Chunk *prev_chunk = &(Chunk){ .pos = { chunk->pos.x - axis, chunk->pos.y - !axis } };

    for (int xy =  (axis ? pos1.x : pos1.y);
             xy <= (axis ? pos2.x : pos2.y);
        xy++,
        i = axis ? mouv_in_chunk_right(&chunk, i) : mouv_in_chunk_up(&chunk, i)
    ) {
        if (!chunk) chunk = add_Chunk(wrd, (Pos){
                                            .x = prev_chunk->pos.x +  axis,
                                            .y = prev_chunk->pos.y + !axis
                                        }
        );
        res = res && Chunk_set_cell(chunk, i, value);

        prev_chunk = chunk;
    }
    /* if (ABS(pos1.x - pos2.x) < ABS(pos1.y - pos2.y))
    {
        if (pos1.y > pos2.y)
        {
            Pos pos_tmp = pos1;
            pos1 = pos2;
            pos2 = pos_tmp;
            pos1.y++;
            pos2.y--;
        }
        Pos_Chunk i = cell_pos1;
        
        Chunk *prev_chunk = &(Chunk){ .pos = { chunk->pos.x, chunk->pos.y - 1 } };

        for (int y = pos1.y; y <= pos2.y; y++, i = mouv_in_chunk_up(&chunk, i))
        {
            if (!chunk) chunk = add_Chunk(wrd, (Pos){
                                                .x = prev_chunk->pos.x,
                                                .y = prev_chunk->pos.y + 1
                                            }
            );
            res = res && Chunk_set_cell(chunk, i, value);

            prev_chunk = chunk;
        }
    }
    else
    {
        if (pos1.x > pos2.x)
        {
            Pos pos_tmp = pos1;
            pos1 = pos2;
            pos2 = pos_tmp;
            pos1.x++;
            pos2.x--;
        }
        Pos_Chunk i = cell_pos1;

        Chunk *prev_chunk = &(Chunk){ .pos = { chunk->pos.x - 1, chunk->pos.y } };

        for (int x = pos1.x; x <= pos2.x; x++, i = mouv_in_chunk_right(&chunk, i))
        {
            if (!chunk) chunk = add_Chunk(wrd, (Pos){
                                                    .x = prev_chunk->pos.x + 1,
                                                    .y = prev_chunk->pos.y
                                                }
            );
            res = res && Chunk_set_cell(chunk, i, value);

            prev_chunk = chunk;
        }
    }
     */
    return res;
}

// TODO use optimized chunk follow
bool World_set_lign_cell(World *wrd, Pos pos1, Pos pos2, State value)
{
    if (pos1.x == pos2.x
     && pos1.y == pos2.y)
        return World_set_cell(wrd, pos1, value);

    // if (pos1.x > pos2.x) SWAP(pos1.x, pos2.x);
    // if (pos1.y > pos2.y) SWAP(pos1.y, pos2.y);
    
    // Pos chunk_pos1 = get_chunk_Pos(pos1);
    // Pos_Chunk cell_pos1 = get_cell_pos(pos1);
    // Item_chunks *item_chunk = set_Item_chunks_get(&wrd->chunks, (Item_chunks){ .pos = chunk_pos1 });
    // Chunk *chunk = (item_chunk == NULL) ? add_Chunk(wrd, chunk_pos1)
    //                                     : item_chunk->data;
    
    bool res = true;

    float dx = ABS(pos1.x - pos2.x);
    float dy = ABS(pos1.y - pos2.y);
    
    int ix = 0;
    int iy = 0;
    if (pos1.x < pos2.x)
        ix = 1;
    else ix = -1;
    if (pos1.y < pos2.y)
        iy = 1;
    else iy = -1;

    float e = 0;
    for (int i = 0; i <= dx + dy; i++)
    {
        res = res && World_set_cell(wrd, pos1, value);
        const float e1 = e - dy;
        const float e2 = e + dx;
        if (ABS(e1) < ABS(e2))
        {
            e = e1;
            pos1.x += ix;
        }
        else
        {
            e = e2;
            pos1.y += iy;
        }
    }
    return res;
}



bool Chunk_delete_partial(Chunk *chunk, Pos start, Pos end)
{
    if (start.packed == 0 && end.x == W - 1 && end.y == H - 1)
        erase_Chunk(chunk);
    
    assert(chunk);
    assert(0 <= start.x && start.x < W );
    assert(0 <= start.y && start.y < H );

    bool res = true;

    for (int cx = start.x; cx <= end.x; cx++)
        for (int cy = start.y; cy <= end.y; cy++)
            res = res && Chunk_set_cell(chunk, POS_CHUNK(cx, cy), empty);

    return res;
}

bool World_delete_area(World *wrd, Pos pos1, Pos pos2)
{
    if (pos1.x > pos2.x) SWAP(pos1.x, pos2.x);
    if (pos1.y > pos2.y) SWAP(pos1.y, pos2.y);
    
    Pos chunk_pos1 = get_chunk_Pos(pos1);
    Pos chunk_pos2 = get_chunk_Pos(pos2);
    Pos_Chunk cell_pos1 = get_cell_pos(pos1);
    Pos_Chunk cell_pos2 = get_cell_pos(pos2);

    bool res = true;
    
    // AAAA
    // DEEB
    // DEEB
    // CCCC
    // E
    // for (int x = chunk_pos1.x + 1; x < chunk_pos1.x; x++)
    //     for (int y = chunk_pos1.y + 1; y < chunk_pos1.y; y++)
    //         erase_Chunk_pos(wrd, (Pos){ x, y });
    
    
    
    Chunk *chunk = UNWRAPE_ITEM_CHUNK(set_Item_chunks_get(&wrd->chunks, (Item_chunks){ .pos = chunk_pos1 }));

    for (int x = chunk_pos1.x; x <= chunk_pos2.x; x++)
    {
        for (int y = chunk_pos1.y; y <= chunk_pos2.y; y++)
        {
            if (chunk) Chunk_delete_partial(
                chunk,
                (Pos){
                    x == chunk_pos1.x ? POS_CHUNK_X(cell_pos1) : 0,
                    y == chunk_pos1.y ? POS_CHUNK_Y(cell_pos1) : 0,
                },
                (Pos){
                    x == chunk_pos2.x ? POS_CHUNK_X(cell_pos2) : W - 1,
                    y == chunk_pos2.y ? POS_CHUNK_Y(cell_pos2) : H - 1,
                }
            );
            // printf("delete in chunk: %i,%i found ", x, y);
            // if (chunk) printf("%i,%i\n", chunk->pos.x, chunk->pos.y);
            // else printf("NULL\n");


            if (y < chunk_pos2.y)
            {
                if (chunk) chunk = chunk->close_chunks[up];
                else chunk = UNWRAPE_ITEM_CHUNK(set_Item_chunks_get(&wrd->chunks, (Item_chunks){ .pos = (Pos){ x, y + 1 } }));
            }
        }
        if (x < chunk_pos2.x)
            chunk = UNWRAPE_ITEM_CHUNK(set_Item_chunks_get(&wrd->chunks, (Item_chunks){ .pos = (Pos){ x + 1, chunk_pos1.y } }));
        
    }
    // X
    /* if (chunk) chunk = chunk->close_chunks[right];
    else chunk = get_Chunk_by_Pos(wrd, (Pos){ chunk_pos2.x, chunk_pos1.y });
    if (chunk) res = res && Chunk_delete_partial(chunk, POS_CHUNK_TO_POS(cell_pos1), (Pos){ 15, 0 });

    for (int y = chunk_pos1.y + 1; y < chunk_pos1.y; y++)
    { // B
        int x = chunk_pos1.x;
        
    }
    for (int x = chunk_pos1.x + 1; x < chunk_pos1.x; x++)
    { // C
        
    }
    for (int y = chunk_pos1.y + 1; y < chunk_pos1.y; y++)
    { // D
        
    } 
     */
    
    return res;
}




void Chunk_free(Chunk *chunk)
{
    
    da_free(&chunk->light_transistors[0]);
    da_free(&chunk->light_transistors[1]);
    da_free(&chunk->update);
    da_free(&chunk->cables);
    da_free(&chunk->not_gate);
}
void Item_chunks_free(Item_chunks *obj)
{
    Chunk_free(obj->data);
}

void World_free(World *wrd)
{
    set_Item_chunks_free_fun_ptr(&wrd->chunks, Item_chunks_free);
    alf_Chunk_free(wrd->chunk_allocator);
    free(wrd->name);
    free(wrd->chunk_allocator);
}
