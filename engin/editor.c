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



/* void save_patern(const World *obj, const char *file, const char *head_msg, Pos_Globale pos1, Pos_Globale pos2)
{
    int width  = abs(pos2.world.x - pos1.world.x) 
               + abs((pos2.chunk & POS_X_MASK) - (pos1.chunk & POS_X_MASK));
    int height = abs(pos2.world.y - pos1.world.y)
               + abs(((pos2.chunk & POS_Y_MASK) >> 4) - ((pos1.chunk & POS_Y_MASK) >> 4));
    // Pos_Globale size = {
    //     pos2.l - pos1.l, pos2.c - pos1.c
    // };

    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC);
    char buffer[16];
    write(fd, head_msg, strlen(head_msg));
    write(fd, " :", 2);
    write(fd, buffer, snprintf(buffer, 15, "%i", height));
    write(fd, ",", 1);
    write(fd, buffer, snprintf(buffer, 15, "%i", width));
    write(fd, "\n", 1);
    // file_patern << head_msg << " :" << size.l << "," << size.c << "\n";
    

    Chunk *tmp = set_Chunk_get_(&obj->chunks, (Chunk){ .pos = pos1.world });

    for (int cy = pos1.world.y; cy < pos2.world.y; cy++)
    {
        for (int cx = pos1.world.x; cx < pos2.world.x; cx++)
        {
            if (tmp->pos.packed != ((uint64_t)cy | ((uint64_t)cx << 32l)))
                tmp = set_Chunk_get_(&obj->chunks, (Chunk){ .pos = { .x = cx, .y = cy } });
            int local_pos = 0;    
            for (int ly = 0; ly < H; ly++)
            {
                for (int lx = 0; lx < W; lx++)
                {
                    if (tmp == NULL)
                        write(fd, "  ,", 3);// << "  " << ",";
                    else 
                    {
                        // file_patern << Gtable[tmp->arr[cy][cx]] << Otable[tmp->arr[cy][cx]] << ",";
                        write(fd, &Gtable[tmp->arr[local_pos]], 1);
                        write(fd, &Otable[tmp->arr[local_pos]], 1);
                        write(fd, ",", 1);
                    }
                    local_pos += POS_X_UNIT;
                }
                local_pos &= POS_Y_MASK; // <- zreos x
                local_pos++;
                
                // if last chunk
                if (cx + 1 == pos2.world.x)
                    write(fd, "\n", 1);
                // file_patern << "\n";
            }
        }
    }
    if (close(fd))
        exit(-1);
}
 */


/* Strb read_line(int fd, int *it)
{
    const int block_size = 512;
    Strb buffer = Strb_make(block_size);
    *it = 0;
    do {
        Strb_read_max(&buffer, fd);
        while (*it < buffer.size && buffer.arr[*it] != '\n')
            (*it)++;

        if (*it == buffer.size)
        {
            buffer.capacity += block_size;
            buffer.arr = realloc(buffer.arr, buffer.capacity);
        }
    } while (*it == buffer.size);
    
    return buffer;
}
 */

/* void add_patern(World *obj, Pos_Globale pos, const char *file)
{
    int fd = open(file, O_RDONLY);

    if (fd == -1)
    {
        printf("error opening file, exit\n");
        return ;
    }

    Strb buffer = Strb_make(256);
    Strb_read_max(fd, &buffer);

    int end_header = 0;
    do {
        while (buffer.arr[end_header] != '\n' 
            && buffer.arr[end_header] != '\r'
            && end_header < buffer.size)
            end_header++;

        if (end_header == buffer.size)
            Strb_read_block(fd, &buffer, 256);
        else end_header++;
    } while (end_header == buffer.size);

    int nbLinePatern = 0;
    int nbColonePatern = 0;
    {
        Strv header = Strv_substr(buffer.self, 0, end_header);
        
        int i = 0;
        while (header.arr[i] != ':' && i < header.size)
        i++;
        if (header.size == i)
        {
            printf("[ERROR] no ':' after msg\n");
            Strb_free(buffer);
            return ;
        }
        i++;
        
        nbLinePatern = atoi(&buffer.arr[i]);
        while (buffer.arr[i] != ',' && i < buffer.size)
        i++;
        if (i == buffer.size)
        {
            printf("[ERROR] no ',' between line and coloms numbers\n");
            Strb_free(buffer);
            return ;
        }
        i++;
        nbColonePatern = atoi(&buffer.arr[i]);
    }
    
    const int line_character_count = nbColonePatern * 3 + 2;// '\n'
    for (int y = 0; y < nbLinePatern; y++)
    {
        while (buffer.capacity - end_header + y * line_character_count < line_character_count)
            Strb_read_block(fd, &buffer, 256);
        
        Strv line = Strv_substr(
            buffer.self, 
            end_header + y * line_character_count, 
            line_character_count - 2
        );

        // getline(file_patern, buffer);

        int t = 0;
        for (int x = 0; x < nbColonePatern; x++)
        {
            World_set_cell(
                obj,
                (Pos){0},   // CH_POS(pos.l + y, pos.c + k), 
                (x << 4) | y,  // (pos.l + y)%H, (pos.c + k)%W, 
                convert(&line.arr[t])
            );
            t += 2;
            assert(line.arr[t] == ',');
            t++;
        }
    }
}
 */



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
static_assert(sizeof(Cell_data) == 2);

typedef struct Chunk_Write {
    Pos pos;
    int size;
    Cell_data array[W*H];
} Chunk_Write;

char *save_World(World *obj, const char *file_name)
{
    Strb to_write = {0};

    Header head = {
        .magic = FILE_MAGIC,
        .version = 1,
        .chunk_width  = W,
        .chunk_height = H,
        .chunk_count = obj->chunks.item_count
    };
    Strb_ncat(&to_write, (char*)&head, sizeof(head));
    
    for_set (Item_chunks, item, &obj->chunks)
    {
        Strb_reserve(&to_write, sizeof(Chunk_Write));
        Chunk_Write *chunk_data = (Chunk_Write*)&to_write.arr[to_write.size];
        
        chunk_data->pos = item->pos;
        chunk_data->size = 0;

        for (int cell_pos = 0; cell_pos < W*H; cell_pos++)
        {
            if (item->data->arr[cell_pos] != empty)
            {
                assert(chunk_data->size < W*H);
                chunk_data->array[chunk_data->size++] = (Cell_data){
                    .pos = cell_pos,
                    .value = item->data->arr[cell_pos] 
                };
            }
        }

        // for 4 bytes alignement
        if (chunk_data->size % 2)
            chunk_data->array[chunk_data->size] = (Cell_data){0};
        
        size_t size_to_write = sizeof(chunk_data->pos)
                             + sizeof(chunk_data->size)
                             + sizeof(chunk_data->array[0]) * chunk_data->size
                             + sizeof(chunk_data->array[0]) * (chunk_data->size % 2);
        assert(to_write.size + (ssize_t)size_to_write < to_write.capacity);
        to_write.size += size_to_write;
    }

    
    if (Str_write_all(Strv_cstr((char*)file_name), to_write.self))
    {
        Strb_free(to_write);
        return "[ERROR] closing the file";
    }
    Strb_free(to_write);
    return NULL;
}



ERR_TYPEDEF(Header, char);
err_Header_char read_Header(Strv *raw_bytes)
{
    err_Header_char res = {0};

    if (raw_bytes->size < (ssize_t)sizeof(res.data))
        return (err_Header_char){ .error = "[ERROR] failed to read header" };
    
    res.data = *(Header*)raw_bytes->arr;
    *raw_bytes = Strv_stride(*raw_bytes, sizeof(res.data));

    if (res.data.packted_magic != FILE_MAGIC_PACKED)
        return (err_Header_char){ .error = "[ERROR] wrong magic" };
    if (res.data.version != 1)
        return (err_Header_char){ .error = "[ERROR] version unsupported" };
    if (res.data.chunk_count < 0)
        return (err_Header_char){ .error = "[ERROR] corrupt file \"negative chunk count\"" };
    if (res.data.chunk_width != W || res.data.chunk_height != H)
        return (err_Header_char){ .error = "[ERROR] corrupt file \"unvalid width or/and height\"" };

    return res;
}


err_World_char load_World(const char *file)
{
    Strb data_raw_bytes = Strb_read_all(Strv_cstr((char*)file));

    Strv raw_bytes = Strv_stride(data_raw_bytes, 0);

    if (!memcmp(&raw_bytes, &(Strb){0}, (ssize_t)sizeof(raw_bytes)))
        return (err_World_char){ .error = "[ERROR] reading the file" };
    
    Header head = err_follow(read_Header(&raw_bytes), err_World_char);
    
    World res = World_make(file);


    for (int i = 0; i < head.chunk_count; i++)
    {
        if (raw_bytes.size < (ssize_t)(sizeof(Pos) + sizeof(int)))
        {
            Strb_free(data_raw_bytes);
            return (err_World_char){ 
                .data = res,
                .error = "[WARNING] corrupt file \"can't read a chunk (blocked at .pos .cell_count)\""
            };
        }
        Pos tmp_pos = *(Pos*)raw_bytes.arr;
        raw_bytes = Strv_stride(raw_bytes, sizeof(Pos));
        
        int cell_count = *(int*)raw_bytes.arr;
        raw_bytes = Strv_stride(raw_bytes, sizeof(int));

        if (cell_count > 0)
        {
            Chunk *chunk = add_Chunk(&res, tmp_pos);
            
            for (int i = 0; i < cell_count; i++)
            {
                if (raw_bytes.size < (ssize_t)sizeof(Cell_data))
                {
                    Strb_free(data_raw_bytes);
                    if (!erase_Chunk(chunk))
                    {
                        World_free(&res);
                        return (err_World_char){ .error = "[ERROR] corrupt file \"can't read a chunk (blocked at ?/? cell)\" while failing, fail to erase '...' chunk" };
                    }
                    return (err_World_char){
                        .error = "[WARNING] corrupt file \"can't read a chunk (blocked at ?/? cell)\"",
                        .data = res
                    };
                }
                Cell_data cell_data = *(Cell_data*)raw_bytes.arr;
                raw_bytes = Strv_stride(raw_bytes, sizeof(Cell_data));
                
                Chunk_set_cell(chunk, cell_data.pos, cell_data.value);
            }
            // if padding
            if (cell_count % 2)
            {
                if (raw_bytes.size < (ssize_t)sizeof(Cell_data))
                {
                    Strb_free(data_raw_bytes);
                    return (err_World_char){
                        .error = "[WARNING] corrupt file \"missing padding\"",
                        .data = res
                    };
                }
    
                raw_bytes = Strv_stride(raw_bytes, sizeof(Cell_data) * (cell_count % 2));
            }
        }
    
    }

    Strb_free(data_raw_bytes);

    return (err_World_char){ .data = res };
}
