
#define SET_IMPLEMENTATION
#define STRING_IMPLEMENTATION
#define STRING_FILEIO
#include "cell_auto.h"
#include "editor.h"


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("no args (1: nb_tick)\n");
        return (0);
    }


    int nb_tick = atoi(argv[1]);
    World test = World_make("test");

    printf("end make\n");
    
  
    // for (int y = 7; y < 9; y++)
    // {
    //     for (int x = 7; x < 10; x++)
    // }

    for (int x = 13; x < 20; x++)
        World_set_cell(&test,
            (Pos){ x, 0 },
            cable_off
        );
        
    World_set_cell(&test, (Pos){ 13, 2 }, cable_off);
    World_set_cell(&test, (Pos){ 14, 2 }, d_transistor_on);
    World_set_cell(&test, (Pos){ 15, 2 }, cable_off);


    
    printf("transsitors[0].size = %d\n", test.chunks.items.arr[0].light_transistors[0].size);
    printf("transsitors[1].size = %d\n", test.chunks.items.arr[0].light_transistors[1].size);
    foreach_ptr (Chunk, item, &test.chunks.items)
    {
        printf("close chunks (%p):\n", item);
        printf("  %d\n%d . %d\n  %d\n", 
            !!item->close_chunks[up], 
            !!item->close_chunks[left], !!item->close_chunks[right],
            !!item->close_chunks[down]
        );
    }


    printf("end init paterns\n");
    
    int i = 0;
    while (i < nb_tick)
    {
        printf("tick : %d\n", i);
        World_basic_print(&test, true);

        printf("start tick...\n"); fflush(stdout);
        World_tick(&test);
        printf("end tick\n");
                
        i++;
    }
    printf("tick : %d\n", i);
    World_basic_print(&test, true); 

    World_free(&test);
}
