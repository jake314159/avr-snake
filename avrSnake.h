#define BLOCK_SIZE 17
#define GRID_SIZE_X 14
#define GRID_SIZE_Y 18
#define SNAKE_LENGTH_MAX (GRID_SIZE_X * GRID_SIZE_Y)

#define BACKGROUND_COLOR BLACK
#define SNAKE_COLOR WHITE
#define SNAKE_HEAD_COLOR RED

typedef struct Point {
    uint8_t x, y;
} Point;

void init_snake(void);
void pushed(char type);
void button_task(void);
void set_gameOver(void);
void updateSnake(void);
void checkSelfCollide(void);
void draw_task(void);
