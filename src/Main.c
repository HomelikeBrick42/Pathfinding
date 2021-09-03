#include "Typedefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

typedef enum Direction {
    Direction_Up,
    Direction_Down,
    Direction_Left,
    Direction_Right,
    Direction_Count,
} Direction;

typedef struct Node Node;
typedef struct Node {
    int x;
    int y;
    Node* Neighbours[Direction_Count];
    Node* Parent;
    b8 Walkable;
    b8 Start;
    b8 End;
    b8 Open;
    b8 Closed;
    b8 OnPath;
    int GCost;
    int HCost;
    int FCost;
} Node;

#define WIDTH  800
#define HEIGHT 600

#define GRID_SCALE  10
#define GRID_WIDTH  (WIDTH / GRID_SCALE)
#define GRID_HEIGHT (HEIGHT / GRID_SCALE)

struct Grid {
    Node Nodes[GRID_WIDTH * GRID_HEIGHT];
    Node* Open[GRID_WIDTH * GRID_HEIGHT];
    u64 OpenCount;
    Node* Closed[GRID_WIDTH * GRID_HEIGHT];
    u64 ClosedCount;
    Node* StartNode;
    Node* EndNode;
} Grid;

void GridAddOpen(Node* node) {
    if (!node->Open) {
        node->Open                  = TRUE;
        Grid.Open[Grid.OpenCount++] = node;
    }
}

void GridRemoveOpen(Node* node) {
    for (u64 i = 0; i < Grid.OpenCount; i++) {
        if (Grid.Open[i] == node) {
            node->Open = FALSE;
            memmove(&Grid.Open[i], &Grid.Open[i + 1], (Grid.OpenCount - i - 1) * sizeof(Node*));
            Grid.OpenCount--;
            break;
        }
    }
}

void GridAddClosed(Node* node) {
    if (!node->Closed) {
        node->Closed                    = TRUE;
        Grid.Closed[Grid.ClosedCount++] = node;
    }
}

void GridRemoveClosed(Node* node) {
    for (u64 i = 0; i < Grid.ClosedCount; i++) {
        if (Grid.Closed[i] == node) {
            node->Closed = FALSE;
            memmove(&Grid.Closed[i], &Grid.Closed[i + 1], (Grid.ClosedCount - i - 1) * sizeof(Node*));
            Grid.ClosedCount--;
            break;
        }
    }
}

void GridReset() {
    Grid.OpenCount   = 0;
    Grid.ClosedCount = 0;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Node* node   = &Grid.Nodes[x + y * GRID_WIDTH];
            node->Open   = FALSE;
            node->Closed = FALSE;
            node->OnPath = FALSE;
            node->GCost  = 0;
            node->HCost  = 0;
            node->FCost  = 0;
            node->x      = x;
            node->y      = y;
            if (y == 0) {
                node->Neighbours[Direction_Down]  = &Grid.Nodes[x + (y + 1) * GRID_WIDTH];
                node->Neighbours[Direction_Left]  = &Grid.Nodes[(x - 1) + y * GRID_WIDTH];
                node->Neighbours[Direction_Right] = &Grid.Nodes[(x + 1) + y * GRID_WIDTH];
                node->Walkable                    = FALSE;
            } else if (y == GRID_HEIGHT - 1) {
                node->Neighbours[Direction_Up]    = &Grid.Nodes[x + (y - 1) * GRID_WIDTH];
                node->Neighbours[Direction_Left]  = &Grid.Nodes[(x - 1) + y * GRID_WIDTH];
                node->Neighbours[Direction_Right] = &Grid.Nodes[(x + 1) + y * GRID_WIDTH];
                node->Walkable                    = FALSE;
            } else if (x == 0) {
                node->Neighbours[Direction_Up]    = &Grid.Nodes[x + (y - 1) * GRID_WIDTH];
                node->Neighbours[Direction_Down]  = &Grid.Nodes[x + (y + 1) * GRID_WIDTH];
                node->Neighbours[Direction_Right] = &Grid.Nodes[(x + 1) + y * GRID_WIDTH];
                node->Walkable                    = FALSE;
            } else if (x == GRID_WIDTH - 1) {
                node->Neighbours[Direction_Up]   = &Grid.Nodes[x + (y - 1) * GRID_WIDTH];
                node->Neighbours[Direction_Down] = &Grid.Nodes[x + (y + 1) * GRID_WIDTH];
                node->Neighbours[Direction_Left] = &Grid.Nodes[(x - 1) + y * GRID_WIDTH];
                node->Walkable                   = FALSE;
            } else {
                node->Neighbours[Direction_Up]    = &Grid.Nodes[x + (y - 1) * GRID_WIDTH];
                node->Neighbours[Direction_Down]  = &Grid.Nodes[x + (y + 1) * GRID_WIDTH];
                node->Neighbours[Direction_Left]  = &Grid.Nodes[(x - 1) + y * GRID_WIDTH];
                node->Neighbours[Direction_Right] = &Grid.Nodes[(x + 1) + y * GRID_WIDTH];
                node->Walkable                    = TRUE;
            }
        }
    }
}

void SetGridStart(int x, int y) {
    Node* node = &Grid.Nodes[x + y * GRID_WIDTH];
    if (!node->End) {
        Grid.StartNode->Start = FALSE;
        node->Start           = TRUE;
        Grid.StartNode        = &Grid.Nodes[x + y * GRID_WIDTH];
    }
}

void SetGridEnd(int x, int y) {
    Node* node = &Grid.Nodes[x + y * GRID_WIDTH];
    if (!node->Start) {
        Grid.EndNode->End = FALSE;
        node->End         = TRUE;
        Grid.EndNode      = &Grid.Nodes[x + y * GRID_WIDTH];
    }
}

int GetDistance(Node* a, Node* b) {
    int distX = abs(a->x - b->x);
    int distY = abs(a->y - b->y);

    if (distX > distY) {
        return (14 * distY) + (10 * (distX - distY));
    } else {
        return (14 * distX) + (10 * (distY - distX));
    }
}

void GridPathfind() {
    Grid.OpenCount   = 0;
    Grid.ClosedCount = 0;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Node* node   = &Grid.Nodes[x + y * GRID_WIDTH];
            node->Parent = nil;
            node->Open   = FALSE;
            node->Closed = FALSE;
            node->OnPath = FALSE;
            node->GCost  = 0;
            node->HCost  = 0;
            node->FCost  = 0;
        }
    }

    GridAddOpen(Grid.StartNode);

    while (Grid.OpenCount > 0) {
        Node* current = Grid.Open[0];
        for (int i = 1; i < Grid.OpenCount; i++) {
            if (Grid.Open[i]->FCost < current->FCost ||
                (Grid.Open[i]->FCost == current->FCost && Grid.Open[i]->HCost < current->HCost)) {
                current = Grid.Open[i];
            }
        }

        GridRemoveOpen(current);
        GridAddClosed(current);

        if (current == Grid.EndNode) {
            break;
        }

        for (u64 i = 0; i < Direction_Count; i++) {
            Node* neighbour = current->Neighbours[i];

            if (neighbour == nil) {
                continue;
            }

            if (!neighbour->Walkable || neighbour->Closed) {
                continue;
            }

            int newMoveCostToNeighbour = current->GCost + GetDistance(current, neighbour);
            if (newMoveCostToNeighbour < neighbour->GCost || !neighbour->Open) {
                neighbour->GCost  = newMoveCostToNeighbour;
                neighbour->HCost  = GetDistance(neighbour, Grid.EndNode);
                neighbour->FCost  = neighbour->GCost + neighbour->HCost;
                neighbour->Parent = current;

                if (!neighbour->Open) {
                    GridAddOpen(neighbour);
                }
            }
        }
    }

    if (Grid.EndNode->Parent) {
        Node* current = Grid.EndNode->Parent;
        while (current != Grid.StartNode) {
            current->OnPath = TRUE;
            current         = current->Parent;
        }
    }
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window =
        SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nil) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nil) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    Grid.StartNode        = &Grid.Nodes[1 + 1 * GRID_WIDTH];
    Grid.StartNode->Start = TRUE;
    Grid.EndNode          = &Grid.Nodes[(GRID_WIDTH - 2) + (GRID_HEIGHT - 2) * GRID_WIDTH];
    Grid.EndNode->End     = TRUE;
    GridReset();

    b8 leftButtonPressed  = FALSE;
    b8 rightButtonPressed = FALSE;
    b8 showTriedPaths     = FALSE;
    int mouseXPos         = 0;
    int mouseYPos         = 0;
    while (TRUE) {
        {
            b8 close = FALSE;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT: {
                        close = TRUE;
                    } break;

                    case SDL_KEYDOWN: {
                        if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                            GridReset();
                        } else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                            SetGridStart(mouseXPos / GRID_SCALE, mouseYPos / GRID_SCALE);
                        } else if (event.key.keysym.scancode == SDL_SCANCODE_E) {
                            SetGridEnd(mouseXPos / GRID_SCALE, mouseYPos / GRID_SCALE);
                        } else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                            showTriedPaths = !showTriedPaths;
                        }
                    } break;

                    case SDL_MOUSEMOTION: {
                        mouseXPos = event.motion.x;
                        mouseYPos = event.motion.y;

                        int x = event.motion.x / GRID_SCALE;
                        int y = event.motion.y / GRID_SCALE;

                        Node* node = &Grid.Nodes[x + y * GRID_WIDTH];

                        if (leftButtonPressed) {
                            node->Walkable = FALSE;
                        } else if (rightButtonPressed) {
                            node->Walkable = TRUE;
                        }
                    } break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP: {
                        int x = event.button.x / GRID_SCALE;
                        int y = event.button.y / GRID_SCALE;

                        Node* node = &Grid.Nodes[x + y * GRID_WIDTH];

                        if (event.button.button == 1) {
                            node->Walkable    = FALSE;
                            leftButtonPressed = event.type == SDL_MOUSEBUTTONDOWN;
                        } else if (event.button.button == 3) {
                            node->Walkable     = TRUE;
                            rightButtonPressed = event.type == SDL_MOUSEBUTTONDOWN;
                        }
                    } break;
                }
            }
            if (close) {
                break;
            }
        }

        GridPathfind();

        SDL_SetRenderDrawColor(renderer, 51, 51, 51, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                Node* node = &Grid.Nodes[x + y * GRID_WIDTH];

                SDL_Rect rect;
                rect.x = (x * GRID_SCALE) + 1;
                rect.y = (y * GRID_SCALE) + 1;
                rect.w = GRID_SCALE - 2;
                rect.h = GRID_SCALE - 2;

                if (node->Start) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                } else if (node->End) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                } else if (node->OnPath) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
#if 0
                } else if (node->Open) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
#endif
                } else if (node->Closed && showTriedPaths) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                } else if (node->Walkable) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                }

                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
