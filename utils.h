
// from nob.h (thanks rexim)
#define UNUSED(value) (void)(value)
#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)


#define Null_Struct(T, value) (0 == memcmp(&value, &(T){0}, sizeof(value)))
#define STRUCT_OFFSET(struct_T, component_name) ((void*)&(((struct_T*)NULL)->component_name))


#define SWAP(a, b) do { __auto_type SWAP_tmp = a; a = b; b = SWAP_tmp; } while (0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define StartBodyEnd(start, end)\
    for ((_LATCH = 0, start); _LATCH < 1; _LATCH = 1, end)

