#include "engin.h"


void Chunk_print(const Chunk *obj)
{
    for (int y = H - 1; y >= 0; y--)
    {
        for (int x = 0; x < W; x++)
        {
                 if ((obj->arr2D[x][y] & TYPE_MASK) == empty)
                printf(".");
            else if ((obj->arr2D[x][y] & TYPE_MASK) == cable_off)
                printf("¤");
            else if ((obj->arr2D[x][y] & TYPE_MASK) == cable_on)
                printf("X");
            else if ((obj->arr2D[x][y] & TYPE_MASK) == bridge)
                printf("#");
            else if ((obj->arr2D[x][y] & TYPE_MASK) == ty_transistor)
            {
                if ((obj->arr2D[x][y] & VARIANT_MASK) == var_off)
                    printf("t");
                else if ((obj->arr2D[x][y] & VARIANT_MASK) == var_o)
                    printf("T");
                else if ((obj->arr2D[x][y] & VARIANT_MASK) == var_on)
                    printf("O");
            }
            else if ((obj->arr2D[x][y] & TYPE_MASK) == ty_not_gate)
            {
                if (obj->parent->state ? (obj->arr2D[x][y] & VARIANT_MASK) == var_o
                                      : (obj->arr2D[x][y] & VARIANT_MASK) == var_on)
                    printf("N");
                else printf("n");
            }
            else printf(" ");
            printf(" ");
            
        }
        /* if (more)
            Chunk_sec_line_print(obj, y); 
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

bool erase_Chunk(Chunk *obj)
{
    if (!set_Item_chunks_erase(&obj->parent->chunks, (Item_chunks){ .pos =  obj->pos }))
        return false;

    for (Direction dir = up; dir <= left; dir++)
        if (obj->close_chunks[dir])
            obj->close_chunks[dir]->close_chunks[opposite_dir(dir)] = NULL;
    
    alf_Chunk_release(obj->parent->chunk_allocator, obj);
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

// obj{IO} :|
static Pos_Chunk mouv_in_chunk(Chunk **obj, Pos_Chunk pos, Direction dir)
{
    if (0) printf("mouv in %p: x%u y%u \tdir: %d\n", 
        (void*)*obj, 
        (int)POS_CHUNK_X(pos),
        (int)POS_CHUNK_Y(pos),
        dir
    );
    assert(obj);

    switch (dir)
    {
    case up:
        if ((pos & POS_Y_MASK) >= H - 1)
        {
            // assert(0);
            *obj = (*obj)->close_chunks[up];
            return (pos & POS_X_MASK) | 0;
        }
        return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) + POS_Y_UNIT);
    case right:
        if (POS_CHUNK_X(pos) >= W - 1)
        {
            *obj = (*obj)->close_chunks[right];
            return 0 | (pos & POS_Y_MASK);
        }
        return ((pos & POS_X_MASK) + POS_X_UNIT) | (pos & POS_Y_MASK);
    case down:
        if ((pos & POS_Y_MASK) <= 0)
        {
            *obj = (*obj)->close_chunks[down];
            return (pos & POS_X_MASK) | (H - 1);
        }
        return (pos & POS_X_MASK) | ((pos & POS_Y_MASK) - POS_Y_UNIT);
    case left:
        if (POS_CHUNK_X(pos) == 0)
        {
            *obj = (*obj)->close_chunks[left];
            return ((W - 1) << 4) | (pos & POS_Y_MASK);
        }
        return ((pos & POS_X_MASK) - POS_X_UNIT) | (pos & POS_Y_MASK);
    default:
        *obj = NULL;
        return pos;
    }
}



static void light_cable(Chunk *obj, Pos_Chunk pos, Direction dir)
{
    static int light_pile_height = 0;
    
    if (0) printf("light(%d) %p: x%u y%u \tdir: %d\n", light_pile_height,
        (void*)obj, 
        (int)POS_CHUNK_X(pos),
        (int)POS_CHUNK_Y(pos),
        dir
    );
    light_pile_height++;

    // not a populated Chunk
    if (obj == NULL)
        ;
    else if ((obj->arr[pos] & TYPE_MASK) == cable_off)
    {
        obj->arr[pos] = cable_on | (CONNECTION_MASK & obj->arr[pos]);

        da_push(&obj->cables, pos);

        Chunk *tmp = obj;
        Pos_Chunk next_pos = mouv_in_chunk(&tmp, pos, up);
        light_cable(tmp, next_pos, up);
        tmp = obj; next_pos = mouv_in_chunk(&tmp, pos, right);
        light_cable(tmp, next_pos, right);
        tmp = obj; next_pos = mouv_in_chunk(&tmp, pos, down);
        light_cable(tmp, next_pos, down);
        tmp = obj; next_pos = mouv_in_chunk(&tmp, pos, left);
        light_cable(tmp, next_pos, left);

    }
    else if ((obj->arr[pos] & TYPE_MASK) == bridge)
    {
        Chunk *tmp = obj;
        Pos_Chunk next_pos = mouv_in_chunk(&tmp, pos, dir);
        light_cable(tmp, next_pos, dir);
    }
    else if ((obj->arr[pos] & TYPE_MASK) == ty_transistor)
    {
        if (dir != opposite_dir(obj->arr[pos] & DIRECTION_MASK))
        {
            if ((obj->arr[pos] & VARIANT_MASK) == var_off)
            {
                obj->arr[pos] += var_o;
                da_push(&obj->update, pos);
            }
            else if ((obj->arr[pos] & VARIANT_MASK) == var_o)
            {
                obj->arr[pos] += var_o;
                da_push(&obj->light_transistors[!obj->parent->state], pos);
            }
        }
    }
    else if ((obj->arr[pos] & TYPE_MASK) == ty_not_gate)
    {
        if (dir != opposite_dir(obj->arr[pos] & DIRECTION_MASK))
        {
            if (0) printf("not gate alimented\n");
            obj->arr[pos] |= !obj->parent->state ? 0b0100 /* var_o */ : 0b1000/* var_on */;
        }
        else if (0) printf("no flow back\n");
    }
    
    light_pile_height--;
}



void Chunk_cable_tick(Chunk *obj)
{
    while (obj->cables.size > 0)
    {
        Pos_Chunk pos_cable = da_pop(&obj->cables);
        obj->arr[pos_cable] = cable_off | (CONNECTION_MASK & obj->arr[pos_cable]);
    }
}

void Chunk_update_tick(Chunk *obj)
{
    while (obj->update.size > 0)
    {
        Pos_Chunk pos = da_pop(&obj->update);
        
        // if (obj->arr[pos] & VARIANT_MASK )  //(obj->arr[pos] - u_transistor_off) % 3)
            //obj->arr[pos] - (obj->arr[pos] - u_transistor_off) % 3;
        // also reinitilize var_on
        obj->arr[pos] &= ~VARIANT_MASK; 
    }
}

void Chunk_light_transistor_tick(Chunk *obj)
{
    while (obj->light_transistors[obj->parent->state].size > 0)
    {
        Pos_Chunk pos = da_pop(&obj->light_transistors[obj->parent->state]);
        Chunk *tmp = obj;

        light_cable(
            tmp, 
            mouv_in_chunk(&tmp, pos, obj->arr[pos] & DIRECTION_MASK),
            obj->arr[pos] & DIRECTION_MASK
        );
    }
}


// 0b 11 00 00
void Chunk_not_gate_tick(Chunk *obj)
{
    // not a 'while' bc the array is readonly
    foreach_ptr (Pos_Chunk, it, &obj->not_gate)
    {
        assert(obj->parent->state == 0 || obj->parent->state == 1);
        if (((obj->arr[*it] & VARIANT_MASK) & (obj->parent->state ? var_o : var_on)) == 0)
        {
            if (0) printf("not gate light out\n");
            light_cable(
                obj,
                mouv_in_chunk(
                    &obj, 
                    *it,
                    obj->arr[*it] & DIRECTION_MASK
                ),
                obj->arr[*it] & DIRECTION_MASK
            );
        }
        if (0) printf(" notgate (%02x): %02x\n", *it, obj->arr[*it]);
    }
}

void World_tick(World *obj)
{
    if (0) {
        printf("\n\nstart tick (%d):\n", obj->state);
        // foreach_ptr (Chunk, it, &obj->chunks.items)
        for_set (Item_chunks, item, &obj->chunks)
            foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
                printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    }
    
    // foreach_ptr (Chunk, it, &obj->chunks.items)
    //     foreach_ptr (Pos_Chunk, pos, &it->not_gate)
    //         it->arr[*pos] &= !obj->state ? ~var_o : ~var_on;
    
    for_set (Item_chunks, item, &obj->chunks)
        foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
            printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    for_set (Item_chunks, item, &obj->chunks)
        foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
            item->data->arr[*pos] &= !obj->state ? ~var_o : ~var_on;

    // light off all cable
    for_set (Item_chunks, item, &obj->chunks)
        Chunk_cable_tick(item->data);
    
    // light off every transistor (var_o and var_on)
    for_set (Item_chunks, item, &obj->chunks)
        Chunk_update_tick(item->data);
    
    // pass current old light transistor
    for_set (Item_chunks, item, &obj->chunks)
        Chunk_light_transistor_tick(item->data);
    
    // not gate 
    for_set (Item_chunks, item, &obj->chunks)
        Chunk_not_gate_tick(item->data);

    if (0) {
        printf("end tick:\n");
        for_set (Item_chunks, item, &obj->chunks)
            foreach_ptr (Pos_Chunk, pos, &item->data->not_gate)
                printf("\tnotgate (%02x): %02x\n", *pos, item->data->arr[*pos]);
    }
    obj->state = !obj->state;
}


static inline void update_cells_connections(Chunk *chunk, const Pos_Chunk pos)
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
        const Pos_Chunk up_pos = mouv_in_chunk(&up_chunk, pos, up);
        Chunk *down_chunk = chunk;
        const Pos_Chunk down_pos = mouv_in_chunk(&down_chunk, pos, down);
       
        // if ((up_chunk->arr[up_pos] & TYPE_MASK) == bridge)
        // {
        //     assert(0);
        // }
        if ((up_chunk->arr[up_pos] & TYPE_MASK) != empty)
        {
            if ((down_chunk->arr[down_pos] & TYPE_MASK) != empty)
            {
                chunk->arr[pos] |= 1 << up | 1 << down;
                up_chunk->arr[up_pos] |= 1 << down;
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
        const Pos_Chunk left_pos = mouv_in_chunk(&left_chunk, pos, left);
        Chunk *right_chunk = chunk;
        const Pos_Chunk right_pos = mouv_in_chunk(&right_chunk, pos, right);
        if (((right_chunk->arr[right_pos] & TYPE_MASK) != empty))
        {
            if ((left_chunk->arr[left_pos] & TYPE_MASK) != empty)
            {
                chunk->arr[pos] |= 1 << left | 1 << right;
                left_chunk->arr[left_pos]   |= 1 << right;
                right_chunk->arr[right_pos] |= 1 << left;
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

void Chunk_set_cell(Chunk *obj, Pos_Chunk pos, State value)
{
    if (value == obj->arr[pos])
        return ;
    
    // Cell old_value = obj->arr[pos];
    Chunk_delete_cell(obj, pos);
    obj->arr[pos] = value;

    if ((value & TYPE_MASK) == ty_transistor)
    {
        if ((value & VARIANT_MASK) >= var_o)
            da_push(&obj->update, pos);
        if ((value & VARIANT_MASK) == var_on)
            da_push(&obj->light_transistors[obj->parent->state], pos);
    }
    else if ((value & TYPE_MASK) == ty_not_gate)
        da_push(&obj->not_gate, pos);
    else if ((value & TYPE_MASK) == cable_on)
        da_push(&obj->cables, pos);
    
    update_cells_connections(obj, pos /* , old_value */);
}

void World_set_cell(World *obj, Pos pos_world, Pos_Chunk pos_chunk, State value)
{
    Item_chunks *item_chunk = set_Item_chunks_get(&obj->chunks, (Item_chunks){ .pos = pos_world });
    if (item_chunk == NULL && value == empty)
        return ;
    Chunk *chunk = NULL;
    if (item_chunk == NULL)
        chunk = add_Chunk(obj, pos_world);
    else chunk = item_chunk->data;
    assert(chunk);

    Chunk_set_cell(chunk, pos_chunk, value);
}

void Chunk_free(Chunk *obj)
{
    
    da_free(&obj->light_transistors[0]);
    da_free(&obj->light_transistors[1]);
    da_free(&obj->update);
    da_free(&obj->cables);
    da_free(&obj->not_gate);
}
void Item_chunks_free(Item_chunks *obj)
{
    Chunk_free(obj->data);
}

void World_free(World *obj)
{
    set_Item_chunks_free_fun_ptr(&obj->chunks, Item_chunks_free);
    alf_Chunk_free(obj->chunk_allocator);
    free(obj->name);
    free(obj->chunk_allocator);
}
