#include "engin.h"

#define FILE_MAGIC { 'W', 'R', 'D', 'B' }
#define FILE_MAGIC_PACKED (*(uint32_t*)(char[]){ 'W', 'R', 'D', 'B' })

const char Gtable[] = {
    ' ',
    'x', 
    'X',
    't','t','t',  //'T', 'O',
    't','t','t',  //'T', 'O',
    't','t','t',  //'T', 'O', 
    't','t','t',  //'T', 'O',

    'b',

    'N', 'N', 'N', 'N',
};

const char Otable[] = {
    ' ',
    ' ', 
    ' ',
    'u', 'u', 'u',
    'r', 'r', 'r',
    'd', 'd', 'd', 
    'l', 'l', 'l',

    'b',

    'u', 'r', 'd', 'l',
};




static inline Cell convert_chars_to_Cell(char code[2])
{
    if (0) printf("chars to cell: '%c%c'\n", code[0], code[1]);

    Direction dir = up;
    switch (code[1])
    {
    case 'u': dir = up;    break;
    case 'l': dir = left;  break;
    case 'd': dir = down;  break;
    case 'r': dir = right; break;
    case ' ': dir = none;  break;
    case 'b': dir = none;  break;
    default:
        return state_trap;
    }

    switch (code[0])
    {
    case ' ': if (dir == none) return empty; break;
    case 'x': if (dir == none) return cable_off; break;                 
    case 'X': if (dir == none) return cable_on; break;
    case 't': if (dir != none) return ty_transistor | var_off | dir; break;
    case 'T': if (dir != none) return ty_transistor | var_o   | dir; break;
    case 'O': if (dir != none) return ty_transistor | var_on  | dir; break;
    case 'n': if (dir != none) return ty_not_gate   | var_off | dir; break;
    case 'N': if (dir != none) return ty_not_gate   | var_on  | dir; break;
    case 'b':
        if (code[1] == 'b' || code[1] == ' ') 
            return bridge;
        break;
    default:;
    }
    return state_trap;
}
static inline uint16_t convert_Cell_to_chars(Cell state, bool tick_state)
{
    static const char dir_to_letter[4] = { 'u', 'r', 'd', 'l' };

    if ((state & TYPE_MASK) == ty_empty)    
        return *(uint16_t*)"  ";
    else if ((state & TYPE_MASK) == ty_cable_off)
        return *(uint16_t*)"x ";
    else if ((state & TYPE_MASK) == ty_cable_on)
        return *(uint16_t*)"X ";
    else if ((state & TYPE_MASK) == ty_transistor)
    {
        if ((state & VARIANT_MASK) == var_off)
            return *(uint16_t*)(char[2]){'t', dir_to_letter[state & DIRECTION_MASK]};
        else if ((state & VARIANT_MASK) == var_o)
            return *(uint16_t*)(char[2]){'T', dir_to_letter[state & DIRECTION_MASK]};
        else if ((state & VARIANT_MASK) == var_on)
            return *(uint16_t*)(char[2]){'O', dir_to_letter[state & DIRECTION_MASK]};
        else UNREACHABLE("unvalide state transistor (mickmak with not gate ?)");
    }
    else if ((state & TYPE_MASK) == ty_not_gate)
        return *(uint16_t*)(char[2]){
            ((state >> (tick_state + 2)) & 1) ? 'n' : 'N', 
            dir_to_letter[state & DIRECTION_MASK]
        };
    else if ((state & TYPE_MASK) == ty_bridge)
        return *(uint16_t*)"b ";
    
    return *(uint16_t*)(char[2]){ '\0', '\0' };
}

char *save_pattern_file(const World *obj, const char *file, const char *head_msg, Pos pos1, Pos pos2)
{
    FILE *file_handel = fopen(file, "w");

    Strb text = {0};
    char *error = save_pattern(obj, &text, head_msg, pos1, pos2);

    if (fwrite(text.arr, sizeof(char), text.size, file_handel) != (size_t)text.size)
        error = error ? error : "[ERROR] didn't write all data";
    if (fclose(file_handel))
        error = error ? error : "[WARNING] file not closed";
    
    Strb_free(text);
    return error;
}
char *save_pattern(const World *obj, Strb *res, const char *head_msg, Pos pos1, Pos pos2)
{
    Pos start = {
        .x = pos1.x < pos2.x ? pos1.x : pos2.x,
        .y = pos1.y < pos2.y ? pos1.y : pos2.y
    };
    Pos end = {
        .x = pos1.x > pos2.x ? pos1.x : pos2.x,
        .y = pos1.y > pos2.y ? pos1.y : pos2.y
    };
    int width = end.x - start.x + 1;
    int height = end.y - start.y + 1;

    Strb_cat(res, head_msg);
    Strb_ncat(res, " :", 2);
    Strb_catf(res, "%u", width);
    Strb_cat_char(res, ',');
    Strb_catf(res, "%u", height);
    Strb_cat_char(res, '\n');
    
    Pos_Globale gpos = Pos_to_Pos_Globale((Pos){ start.x, end.y });
    Item_chunks *chunk = set_Item_chunks_get(
        &obj->chunks, 
        (Item_chunks){ .pos = gpos.chunk }
    );
    
    for (int cy = end.y; cy >= start.y; cy--)
    {
        for (int cx = start.x; cx <= end.x; cx++)
        {
            gpos = Pos_to_Pos_Globale((Pos){ cx, cy });
            if (chunk->pos.packed != gpos.chunk.packed)
            {
                chunk = set_Item_chunks_get(
                    &obj->chunks, 
                    (Item_chunks){ .pos = gpos.chunk }
                );
            }

            if (chunk->data == NULL)
                Strb_ncat(res, "  ,", 3);
            else
            {
                uint16_t chars = convert_Cell_to_chars(chunk->data->arr[gpos.cell], obj->state);
                if (chars == 0)
                    Strb_ncat(res, "  ,", 3);    
                else
                {
                    Strb_ncat(res, (char*)&chars, 2);
                    Strb_cat_char(res, ',');
                }
            }
        }
        bool is_CRLF = false;
        Strb_ncat(res, &"\r\n"[!is_CRLF], 1 + is_CRLF);
    }

    if (0 && DEBUG_VALUE)
    {
        printf("save pattern: \"");
        Str_print(*res);
        printf("\"\n");
    }
    return NULL;
}


Pattern_header pattern_parser_header(Strv *pattern, bool *is_CRLF)
{
    Strv header = Strv_c_substr(pattern, '\n');

    if (Str_end(header) == '\r')
    {
        *is_CRLF = true;
        header.size--;
    }
    assert(pattern->arr[0] == '\n');
    *pattern = Strv_stride(*pattern, 1);

    Pattern_header res = {
        .msg = Strv_c_substr(&header, ':')
    };
    if (header.size == 0)
    {
        if (DEBUG_VALUE) printf("[ERROR] no ':' after msg\n");
        return (Pattern_header){0};
    }
    // if no description state it bc otherwise it will be interpreted as an error
    if (res.msg.size <= 0) 
        res.msg = Strv_cstr(PATTERN_DEFAULT_DESCRIPTION);

    header = Strv_stride(header, 1);
    
    if (header.size == 0)
    {
        if (DEBUG_VALUE) printf("[ERROR] no ',' between line and coloms numbers\n");
        return (Pattern_header){0};
    }
    
    while (header.size > 0 && isspace(header.arr[0]))
        header = Strv_stride(header, 1);

    res.width = Str_atoll(header, &header);
    if (res.width == INT64_MAX)
    {
        if (DEBUG_VALUE) printf("[ERROR] no fail to parse numbre of colone\n");
        return (Pattern_header){0};
    }

    while (header.size && isspace(header.arr[0]))
        header = Strv_stride(header, 1);
    if (header.arr[0] != ',')
    {
        if (DEBUG_VALUE) printf("[ERROR] missing ',' between width and height\n");
        return (Pattern_header){0};
    }
    do 
        header = Strv_stride(header, 1);
    while (header.size > 0 && isspace(header.arr[0]));
    
    res.height = Str_atoll(header, &header);
    if (res.height == INT64_MAX)
    {
        if (DEBUG_VALUE) printf("[ERROR] no fail to parse numbre of lignes\n");
        return (Pattern_header){0};
    }
    while (header.size > 0 && isspace(header.arr[0]))
        header = Strv_stride(header, 1);
    if (header.size != 0)
    {
        if (DEBUG_VALUE) printf("[ERROR] expected a new ligne\n");
        return (Pattern_header){0};
    }
    return res;
}
    
// == {0} on error
Pattern_header get_pattern_header(Strv pattern_string)
{
    bool dummy = false;
    if (Null_Struct(Strv, pattern_string))
        return (Pattern_header){0};

    return pattern_parser_header(&pattern_string, &dummy);
}

bool add_pattern(World *obj, Pos pos, Strv pattern, Strb *res_msg)
{
    if (0 && DEBUG_VALUE)
    {
        printf("add pattern at %i,%i: \"", pos.x, pos.y);
        Str_print(pattern);
        printf("\"\n");
    }
    if (Null_Struct(Strv, pattern))
    {
        if (DEBUG_VALUE) printf("[ERROR] empty input\n");
        return false;
    }

    bool is_CRLF = false; // \r\n or \n
    bool function_success = true;

    Pattern_header header = pattern_parser_header(&pattern, &is_CRLF);
    if (Null_Struct(Pattern_header, header))
        return false;
    
    if (pattern.size < (header.height - 1) * (3 * header.width + 1 + is_CRLF) + 3 * header.width)
    {
        if (DEBUG_VALUE) printf("[ERROR] body of the pattern too small\n");
        return false;
    }

    // const int line_character_count = nb_colone_patern * 3 + 2;// '\n'
    for (int y = header.height - 1; y >= 0; y--)
    {
        Strv line = Strv_c_substr(&pattern, is_CRLF ? '\r' : '\n');
        pattern = Strv_stride(pattern, 1 + is_CRLF);

        int t = 0;
        for (int x = 0; t < line.size && x < header.width; x++)
        {
            Pos_Globale gpos = Pos_to_Pos_Globale((Pos){x + pos.x, y + pos.y});

            // success if all set_cell success
            function_success &= World_set_cell(
                obj, 
                gpos.chunk, 
                gpos.cell, 
                convert_chars_to_Cell(&line.arr[t])
            );
            t += 3;
        }
    }
    if (res_msg) *res_msg = Strb_Str(header.msg);

    
    return function_success;
}
bool add_pattern_file(World *obj, Pos pos, const char *file, Strb *res_msg)
{
    Strb data_raw_bytes = err_attrib(Strb_read_all(file), 
        err, return false;
    );
    
    if (!add_pattern(obj, pos, data_raw_bytes.self, res_msg)) 
    {
        Strb_free(data_raw_bytes);
        return false;
    }
    
    Strb_free(data_raw_bytes);
    return true;
}

typedef struct Header {
    union {
        char magic[4];
        uint32_t packted_magic;
    };
    int version;
    int chunk_width;
    int chunk_height;
    int chunk_count;
} Header;


typedef struct Cell_data {
    Pos_Chunk pos;
    Cell value;
} Cell_data;
static_assert(sizeof(Cell_data) == 2, "sizeof Cell_data");

typedef struct Chunk_Write {
    Pos pos;
    int size;
    Cell_data array[W*H];
} Chunk_Write;

char *save_World(const World *obj, const char *file_name)
{
    Strb to_write = {0};

    Header head = {
        .magic = FILE_MAGIC,
        .version = 1,
        .chunk_width  = W,
        .chunk_height = H,
        .chunk_count = obj->chunks.item_count
    };
    if (!Strb_ncat(&to_write, (char*)&head, sizeof(head))) return "[ERROR] out of memory";
    
    for_set (Item_chunks, item, &obj->chunks)
    {
        Strb_reserve(&to_write, sizeof(Chunk_Write));
        Chunk_Write *chunk_data = (Chunk_Write*)&to_write.arr[to_write.size];
        
        chunk_data->pos = item->pos;
        chunk_data->size = 0;

        for (int cell_pos = 0; cell_pos < W*H; cell_pos++)
        {
            if ((item->data->arr[cell_pos] & TYPE_MASK) != empty)
            {
                assert(chunk_data->size < W*H);
                chunk_data->array[chunk_data->size++] = (Cell_data){
                    .pos = cell_pos,
                    .value = item->data->arr[cell_pos] 
                };
            }
        }

        // for four bytes alignement
        if (chunk_data->size % 2)
            chunk_data->array[chunk_data->size] = (Cell_data){0};
        
        size_t size_to_write = sizeof(chunk_data->pos)
                             + sizeof(chunk_data->size)
                             + sizeof(chunk_data->array[0])
                             * (chunk_data->size + chunk_data->size % 2);
        assert(to_write.size + (ssize_t)size_to_write < to_write.capacity);
        to_write.size += size_to_write;
    }

    
    if (!Str_write_all(file_name, to_write.self))
    {
        Strb_free(to_write);
        return "[ERROR] save_World: fileio";
    }
    Strb_free(to_write);
    return NULL;
}

char *read_Header(Strv *raw_bytes, Header *res)
{
    if (raw_bytes->size < (ssize_t)sizeof(*res))
        return "[ERROR] file shorter than the header";
    
    *res = *(Header*)raw_bytes->arr;
    *raw_bytes = Strv_stride(*raw_bytes, sizeof(*res));

    if (res->packted_magic != FILE_MAGIC_PACKED)
        return "[ERROR] wrong magic";
    if (res->version != 1)
        return "[ERROR] version unsupported";
    if (res->chunk_count < 0)
        return "[ERROR] corrupt file \"negative chunk count\"";
    if (res->chunk_width != W || res->chunk_height != H)
        return "[ERROR] corrupt file \"unvalid width or/and height\"";

    return NULL;
}

char *load_World(const char *file, World *res)
{
    Strb data_raw_bytes = err_attrib(Strb_read_all(file), 
        err, return "[ERROR] can't read file";
    );

    Strv raw_bytes = data_raw_bytes.self;
    
    Header head = {0};
    char *err_head = read_Header(&raw_bytes, &head);
    if (err_head)
    {
        Strb_free(data_raw_bytes);
        return err_head;
    }
    
    *res = World_make(file);
    for (int i = 0; i < head.chunk_count; i++)
    {
        if (raw_bytes.size < (ssize_t)(sizeof(Pos) + sizeof(int)))
        {
            Strb_free(data_raw_bytes);
            return "[WARNING] corrupt file \"can't read a chunk (blocked at .pos .cell_count)\"";
        }
        Pos tmp_pos = {0};
        tmp_pos.x = *(int*)raw_bytes.arr;
        raw_bytes = Strv_stride(raw_bytes, sizeof(tmp_pos.x));
        tmp_pos.y = *(int*)raw_bytes.arr;
        raw_bytes = Strv_stride(raw_bytes, sizeof(tmp_pos.y));
        
        int cell_count = *(int*)raw_bytes.arr;
        raw_bytes = Strv_stride(raw_bytes, sizeof(int));

        if (cell_count > 0)
        {
            Chunk *chunk = add_Chunk(res, tmp_pos);
            
            for (int i = 0; i < cell_count; i++)
            {
                if (raw_bytes.size < (ssize_t)sizeof(Cell_data))
                {
                    Strb_free(data_raw_bytes);
                    if (!erase_Chunk(chunk))
                    {
                        World_free(res);
                        return "[ERROR] corrupt file \"can't read a chunk (blocked at ?/? cell)\" while failing, fail to erase '...' chunk";
                    }
                    return "[WARNING] corrupt file \"can't read a chunk (blocked at ?/? cell)\"";
                }
                Cell_data cell_data = *(Cell_data*)raw_bytes.arr;
                raw_bytes = Strv_stride(raw_bytes, sizeof(Cell_data));
                
                Chunk_set_cell(chunk, cell_data.pos, cell_data.value);
            }
            // padding
            if (cell_count % 2)
            {
                if (raw_bytes.size < (ssize_t)sizeof(Cell_data))
                {
                    Strb_free(data_raw_bytes);
                    return "[WARNING] corrupt file \"missing padding\"";
                }
                raw_bytes = Strv_stride(raw_bytes, sizeof(Cell_data) * (cell_count % 2));
            }
        }
    }
    Strb_free(data_raw_bytes);
    return NULL;
}
