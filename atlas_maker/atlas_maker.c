#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../engin/set_defines.h"

enum Colors {
    red = 0,
    green = 1,
    blue = 2,
    alpha = 3
};

typedef struct Img
{
    uint8_t *data;    
    int w;
    int h;
    int channels;
} Img;

Img Img_make(int channels, int w, int h)
{
    if (0) printf("%d channels (new)\n", channels);
    return (Img){
        .data = calloc(channels * w * h, sizeof(uint8_t)),
        .channels = channels,
        .w = w,
        .h = h
    };
}
Img Img_stbi_load(const char *filename, int channels)
{
    Img res = {
        .data = NULL,
        .w = 0,
        .h = 0,
        .channels = 4,
    };
    res.data = stbi_load(filename, &res.w, &res.h, &res.channels, channels);
    if (0) printf("%d channels (load)\n", res.channels);
    return res;
}
int Img_stbi_write_png(const Img img, const char *filename)
{
    return stbi_write_png(
        filename, 
        img.w, img.h, 
        img.channels, img.data, 
        img.w * img.channels
    );
}
// ret: uint8_t[channels]
uint8_t *Img_get_px_pt(Img img, int x, int y)
{
    if (0 <= x && x < img.w
     && 0 <= y && y < img.h)
        return &img.data[(y * img.w + x) * img.channels];
    
    raise(SIGINT);
    return NULL;
}
void Img_cp_px(Img dst, Img src, int dst_x, int dst_y, int src_x, int src_y)
{
    assert(dst.channels == src.channels);
    for (int i = 0; i < dst.channels; i++)
        Img_get_px_pt(dst, dst_x, dst_y)[i] = Img_get_px_pt(src, src_x, src_y)[i];
}
void Img_print_metadata(Img img)
{
    printf("w%d h%d ch%d at %p\n", img.w, img.h, img.channels, img.data);
}

// Copy a rectangular region from src to dst image at given position.
void copy_rect_to_image(Img src, Img dst,
    int src_x, int src_y,     // Source top-left corner
    int w, int h,       // Width/height of region to copy
    int dst_x, int dst_y      // Destination top-left corner
) {
    assert(src.channels == dst.channels);

    printf("cp (%d:%d,%d:%d) at (%d:,%d:)\n", 
        src_x, src_x+w,
        src_y, src_y+h,
        dst_x, dst_y
    );
    printf("src w%d h%d dst w%d h%d\n", src.w, src.h, dst.w, dst.h);
    
    {
        if (src_x < 0)          raise(SIGINT);
        if (src_y < 0)          raise(SIGINT);
        if (src_y + h > src.h)  raise(SIGINT);
        if (src_x + w > src.w)  raise(SIGINT);
    
        if (dst_x < 0)          raise(SIGINT);
        if (dst_y < 0)          raise(SIGINT);
        if (dst_x + w > dst.w)  raise(SIGINT);
        if (dst_y + h > dst.h)  raise(SIGINT);

        if (w <= 0 || h <= 0)   raise(SIGINT);
    }
    

    for (int row = 0; row < h; row++)
        memcpy(
            Img_get_px_pt(dst, dst_x, dst_y + row), 
            Img_get_px_pt(src, src_x, src_y + row), 
            sizeof(uint8_t) * w * src.channels
        );
}


void add_rect_to_image(Img src, Img dst,
    int src_x, int src_y,
    int w, int h,
    int dst_x, int dst_y
) {
    assert(src.channels == dst.channels);
    
    printf("add (%d:%d,%d:%d) at (%d:,%d:)\n", 
        src_x, src_x+w,
        src_y, src_y+h,
        dst_x, dst_y
    );
    printf("src w%d h%d dst w%d h%d\n", src.w, src.h, dst.w, dst.h);
    
    {
        if (src_x < 0)          raise(SIGINT);
        if (src_y < 0)          raise(SIGINT);
        if (src_y + h > src.h)  raise(SIGINT);
        if (src_x + w > src.w)  raise(SIGINT);
    
        if (dst_x < 0)          raise(SIGINT);
        if (dst_y < 0)          raise(SIGINT);
        if (dst_x + w > dst.w)  raise(SIGINT);
        if (dst_y + h > dst.h)  raise(SIGINT);

        if (w <= 0 || h <= 0)   raise(SIGINT);
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
            Img_cp_px(
                dst, src,
                dst_x + x, dst_y + y,
                src_x + x, dst_y + y
            );
    }
}

void rotate_rect_90(Img img,
    int x, int y,
    int w, int h,       
    Direction rotation
) {
    printf("rot (%d:%d,%d:%d) of %d°\n", 
        x, x+w,
        y, y+h,
        90 * rotation
    );
    Img_print_metadata(img);

    if (x < 0)
        raise(SIGINT);
    if (y < 0)
        raise(SIGINT);
    if (x + w > img.w)
        raise(SIGINT);
    if (y + h > img.h)
        raise(SIGINT);
    if (w <= 0 || h <= 0)
        raise(SIGINT);

    Img tmp = Img_make(4, w, h);

    for (int yy = 0; yy < h; yy++)
    {
        for (int xx = 0; xx < w; xx++)
        {
            int src_idx = ((yy + y) * img.w + (xx + x)) * img.channels;
            
            int dx, dy;
            // Rotation logic
            switch (rotation)
            {
                case up: // 0°
                    dx = xx;
                    dy = yy;
                    break;
                case right: // 90°
                    dx = h - 1 - yy;
                    dy = xx;
                    break;
                case down: // 180°
                    dx = w - 1 - xx;
                    dy = h - 1 - yy; 
                    break;
                case left: // 270°
                    dx = yy;
                    dy = w - 1 - xx;
                    break;
            }

            int dst_idx = (dy * tmp.w + dx) * tmp.channels;
            
            Img_cp_px(tmp, img, dx, dy, xx + x, yy + y);
        }
    }
    copy_rect_to_image(tmp, img, 0, 0, tmp.w, tmp.h, x, y);
    stbi_image_free(tmp.data);   
}




int main()
{
    int tile_w = 16, tile_h = 16, channels = 4;

    Img output = Img_make(channels, tile_w * 256, tile_h);

    
    Img img = Img_stbi_load("/home/augustin/C/cables_raylib/assets/atlas.png", 4);
    printf("load: "); Img_print_metadata(img);
    

    state index = 0;
    int cursor = 0;
    
    if (1)
    {
        /* empty space */
        for (; index < cable_on;  index++, cursor += tile_w)
            ;
        /* off cables  */
        for (; index < cable_off; index++, cursor += tile_w)
        {
            // printf("cp img (cable_on) at index %d (offset %d)\n", index, cursor);
            copy_rect_to_image(
                img, output,
                0, 1 * tile_h,
                tile_w, tile_h,
                cursor, 0
            );
            
            if (1) 
            {
                if (index & 0b0001)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 6, 1 * tile_h,
                        4, 6,
                        cursor + 6, 0
                    );
                if (index & 0b0010)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 10, 1 * tile_h + 6,
                        6, 4,
                        cursor + 10, 6
                    );
                if (index & 0b0100)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 6, 1 * tile_h + 10,
                        4, 6,
                        cursor + 6, 10
                    );
                if (index & 0b1000)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w, 1 * tile_h + 6,
                        6, 4,
                        cursor, 6
                    );
            }
        }
        /* on calbes   */
        for (; index < bridge; index++, cursor += tile_w)
        {
            copy_rect_to_image(
                img, output,
                0, 2 * tile_h,
                tile_w, tile_h, 
                cursor, 0
            );

            if (1) 
            {
                if (index & 0b0001)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 6, 2 * tile_h,
                        4, 6,
                        cursor + 6, 0
                    );
                if (index & 0b0010)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 10, 2 * tile_h + 6,
                        6, 4,
                        cursor + 10, 6
                    );
                if (index & 0b0100)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w + 6, 2 * tile_h + 10,
                        4, 6,
                        cursor + 6, 10
                    );
                if (index & 0b1000)
                    copy_rect_to_image(
                        img, output,
                        1 * tile_w, 2 * tile_h + 6,
                        6, 4,
                        cursor, 6
                    );
            }
        }
        /* bridges */
        for (; index < u_transistor_off; index++, cursor += tile_w)
        {
            copy_rect_to_image(
                img, output,
                0, 3 * tile_h,
                tile_w, tile_h, 
                cursor, 0
            );
        }
        
        /* transistors */
        {
            for (int x_src_cursor = 0; 
                x_src_cursor < 3 * tile_w; 
                x_src_cursor += tile_w
            ) {
                for (Direction i = up; 
                        i <= left; 
                        i++, cursor += tile_w, index++
                ) {
                    copy_rect_to_image(
                        img, output,
                        x_src_cursor, 4 * tile_h,
                        tile_w, tile_h, 
                        cursor, 0
                    );
                    rotate_rect_90(
                        output, 
                        cursor, 0, 
                        tile_w, tile_h, 
                        i
                    );
                }
            }
        }
        for (; index < u_not_gate; index++, cursor += tile_w)
            ;

        /* not gate */
        {
            for (Direction i = up;
                           i <= left;
                           i++, cursor += tile_w, index++
            ) {
                copy_rect_to_image(
                    img, output,
                    0, 5 * tile_h,
                    tile_w, tile_h,
                    cursor, 0
                );
                rotate_rect_90(
                    output,
                    cursor, 0,
                    tile_w, tile_h,
                    i
                );
            }
            
            for (int _ = 0; _ < 3; _++) 
            for (Direction i = up;
                           i <= left;
                           i++, cursor += tile_w, index++
            ) {
                copy_rect_to_image(
                    img, output,
                    tile_w, 5 * tile_h,
                    tile_w, tile_h,
                    cursor, 0
                );
                rotate_rect_90(
                    output,
                    cursor, 0,
                    tile_w, tile_h,
                    i
                );
            }
        }
        
    }
    else if (0)
    {
        for (int i = 0; i < output.w; i++)
        {
            Img_get_px_pt(output, i, 1)[red] = 100;
            Img_get_px_pt(output, i, 2)[red] = 100;
            Img_get_px_pt(output, i, 3)[red] = 100;
            Img_get_px_pt(output, i, 1)[alpha] = UINT8_MAX;
            Img_get_px_pt(output, i, 2)[alpha] = UINT8_MAX;
            Img_get_px_pt(output, i, 3)[alpha] = UINT8_MAX;
        }
    }
    else ;

    stbi_image_free(img.data);
    
    printf("whta\n");
    if (!Img_stbi_write_png(output, "./assets/new_atlas.png"))//, final_w, final_h, channels, output, final_w * channels);
        printf("not SAVED !\n");
    else printf("img saved\n");
    stbi_image_free(output.data);
    return 0;
}
