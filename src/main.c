#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define SLEEP 90000
#define POP_FACTOR 0.35

int DEAD = 0;
int ALIVE = 1;
char *sprite = "+x";
int len = 2;

typedef struct
{
    size_t rows;
    size_t cols;
} Window;

Window *NewWindow(size_t rows, size_t cols)
{
    Window *window = (Window *)malloc(sizeof(Window));
    assert(window != NULL);

    window->rows = rows;
    window->cols = cols;

    return window;
};

typedef struct
{
    int **state;
    size_t population;
    size_t generation;
    Window *window;
} GOL;

GOL *NewGame(size_t rows, size_t cols)
{
    GOL *game = (GOL *)malloc(sizeof(GOL));
    assert(game != NULL);

    Window *window = NewWindow(rows, cols);
    assert(window != NULL);

    game->window = window;

    game->state = (int **)malloc(sizeof(int *) * rows);
    for (int r = 0; r < rows; r++)
    {
        game->state[r] = (int *)malloc(sizeof(int) * cols);
    }

    game->population = 0;
    game->generation = 0;

    return game;
}

void InitialGameState(GOL *game)
{
    for (int r = 0; r < game->window->rows; r++)
    {
        for (int c = 0; c < game->window->cols; c++)
        {
            game->state[r][c] = DEAD;
        }
    }
};

void PopulateGameState(GOL *game)
{
    int pop = (game->window->rows * game->window->cols) * POP_FACTOR;
    for (int i = 0; i < pop; i++)
    {
        int row = rand() % (game->window->rows - 1 + 1 - 1) + 1;
        int col = rand() % (game->window->cols - 1 + 1 - 1) + 1;
        game->state[row][col] = ALIVE;
    }

    game->population = pop;
}

int GetNeighborCount(int **state, int row, int col)
{
    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
                continue;
            if (state[row + i][col + j] != DEAD)
                count++;
        }
    }

    return count;
}

void RenderGameState(GOL *game)
{
    printf("[POPULATION : %zu] [GENERATION : %zu]\n", game->population, game->generation);

    for (int r = 0; r < game->window->rows; r++)
    {
        for (int c = 0; c < game->window->cols; c++)
        {
            if (game->state[r][c] == ALIVE)
            {
                printf("%c", sprite[(game->generation) % len]);
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
};

void UpdateGameState(GOL *game)
{
    int **next_state = (int **)malloc(sizeof(int *) * game->window->rows);
    for (int r = 0; r < game->window->rows; r++)
    {
        next_state[r] = (int *)malloc(sizeof(int) * game->window->cols);
    }

    size_t next_pop = 0;

    for (int r = 0; r < game->window->rows; r++)
    {
        for (int c = 0; c < game->window->cols; c++)
        {
            if (r == 0 || c == 0 || r == game->window->rows - 1 || c == game->window->cols - 1)
            {
                next_state[r][c] = DEAD;
                continue;
            }

            int neighbors = GetNeighborCount(game->state, r, c);
            char current_state = game->state[r][c];

            if (current_state != DEAD && (neighbors == 2 || neighbors == 3))
            {
                next_state[r][c] = ALIVE;
                next_pop++;
            }
            else if (current_state == DEAD && neighbors == 3)
            {
                next_state[r][c] = ALIVE;
                next_pop++;
            }
            else
            {
                next_state[r][c] = DEAD;
            }
        }
    }

    for (int r = 0; r < game->window->rows; r++)
    {
        for (int c = 0; c < game->window->cols; c++)
        {
            game->state[r][c] = next_state[r][c];
        }
        free(next_state[r]);
    }
    free(next_state);

    game->population = next_pop;
    game->generation++;
};

void Run(GOL *game)
{
    InitialGameState(game);
    PopulateGameState(game);

    while (1)
    {
        UpdateGameState(game);
        RenderGameState(game);
        usleep(SLEEP);
        system("clear");
    }
};

int main()
{
    struct winsize win;
    ioctl(0, TIOCGWINSZ, &win);
    srand(time(NULL));

    GOL *game = NewGame(win.ws_row - 2, win.ws_col);
    Run(game);

    return 0;
}
