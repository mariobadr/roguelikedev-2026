#include "map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "bsp.h"
#include "rand.h"

/**
 * A connection between a and b.
 */
struct connection
{
  int a;
  int b;
};

/**
 * A graph describing rooms and the connections between them.
 */
struct room_graph
{
  SDL_Rect* rooms;
  int room_count;

  struct connection* connections;
  int connection_count;
};

/**
 * Return a randomly generated rectangle within bounds.
 */
static SDL_Rect
generate_room(SDL_Rect const* bounds, struct rand_state* rng)
{
  int const margin = 1;
  // how much smaller than the leaf a room can be, per axis
  int const min_shave = 1;
  int const max_shave = 3;

  int shave_w = (int)rand_next_between(rng, min_shave, max_shave);
  int shave_h = (int)rand_next_between(rng, min_shave, max_shave);

  int w = bounds->w - 2 * margin - shave_w;
  int h = bounds->h - 2 * margin - shave_h;

  // room can sit anywhere within the space freed up by the shave
  int x = bounds->x + margin + (int)rand_next_between(rng, 0, shave_w);
  int y = bounds->y + margin + (int)rand_next_between(rng, 0, shave_h);

  return (SDL_Rect){
    .x = x,
    .y = y,
    .w = w,
    .h = h,
  };
}

/**
 * Update graph's rooms and establish the connections between those rooms.
 *
 * @param graph       a graph to update.
 * @param tree        a tree to traverse.
 * @param node_index  a node in the tree.
 * @param rng         a random number generator.
 *
 * @return a node in the tree.
 */
static int
generate_room_graph(struct room_graph* graph,
                    struct rl_bsp_tree const* tree,
                    int node_index,
                    struct rand_state* rng)
{
  struct rl_bsp_node const* node = &tree->nodes[node_index];

  if (rl_bsp_node_is_leaf(node)) {
    // base case
    int const room_index = graph->room_count;

    graph->rooms[room_index] = generate_room(&node->rect, rng);
    graph->room_count++;

    return room_index;
  }

  // recurse left subtree
  int left = generate_room_graph(graph, tree, rl_bsp_left_of(node_index), rng);
  // recurse right subtree
  int right =
    generate_room_graph(graph, tree, rl_bsp_right_of(node_index), rng);

  // connect left and right
  graph->connections[graph->connection_count].a = left;
  graph->connections[graph->connection_count].b = right;
  graph->connection_count++;

  return left; // make this random?
}

/**
 * Initialize and generate a room graph.
 */
static bool
init_room_graph(struct room_graph* graph,
                struct rl_bsp_tree const* tree,
                struct rand_state* rng)
{
  int const count = tree->leaf_count;

  graph->rooms = SDL_calloc(count, sizeof(*graph->rooms));
  if (graph->rooms == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  graph->connections = SDL_calloc(count - 1, sizeof(*graph->connections));
  if (graph->connections == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    SDL_free(graph->rooms);
    return false;
  }

  graph->room_count = 0;
  graph->connection_count = 0;
  generate_room_graph(graph, tree, 0, rng);

  return true;
}

/**
 * Update the region of map specified by room.
 */
static void
carve_room(struct rl_map* map, SDL_Rect const* room)
{
  for (int y = room->y; y < room->y + room->h; ++y) {
    for (int x = room->x; x < room->x + room->w; ++x) {
      map->tiles[y * map->width + x].type = RL_TILE_FLOOR;
    }
  }
}

/**
 * Update a map's tiles so that it has all its rooms.
 */
static void
carve_rooms(struct rl_map* map, SDL_Rect const* rooms, int room_count)
{
  for (int i = 0; i < room_count; ++i) {
    carve_room(map, &rooms[i]);
  }
}

static void
carve_hline(struct rl_map* map, int x1, int x2, int y)
{
  // ensure x2 > x1 by swapping their values
  if (x1 > x2) {
    int tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  // now set the floor tiles along the x-axis
  for (int x = x1; x <= x2; ++x) {
    map->tiles[y * map->width + x].type = RL_TILE_FLOOR;
  }
}

static void
carve_vline(struct rl_map* map, int y1, int y2, int x)
{
  // ensure y2 > y1 by swapping their values
  if (y1 > y2) {
    int tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  // now set the floor tiles along the y-axis
  for (int y = y1; y <= y2; ++y) {
    map->tiles[y * map->width + x].type = RL_TILE_FLOOR;
  }
}

/**
 * Update a map's tiles so that a connection between rooms become L-shaped or
 * straight corridors.
 */
static void
carve_corridor(struct rl_map* map, SDL_Rect const* a, SDL_Rect const* b)
{
  int ax = a->x + a->w / 2;
  int ay = a->y + a->h / 2;

  int bx = b->x + b->w / 2;
  int by = b->y + b->h / 2;

  carve_hline(map, ax, bx, ay);
  carve_vline(map, ay, by, bx);
}

/**
 * Update a map's tiles so that there are corridors between all rooms.
 */
static void
carve_corridors(struct rl_map* map, struct room_graph const* graph)
{
  for (int i = 0; i < graph->connection_count; ++i) {
    struct connection const* connection = &graph->connections[i];

    SDL_Rect const* a = &graph->rooms[connection->a];
    SDL_Rect const* b = &graph->rooms[connection->b];

    carve_corridor(map, a, b);
  }
}

/**
 * Randomly generate the tiles in map.
 *
 * @return whether the generation was successful.
 */
static bool
generate_map(struct rl_map* map, struct rand_state* rng)
{
  SDL_Rect rect = { 0 };
  rect.w = map->width;
  rect.h = map->height;

  // generate the rooms
  int const max_depth = 4;
  struct rl_bsp_tree tree;
  if (!rl_bsp_tree_init(&tree, max_depth, rect)) {
    return false;
  }

  struct rl_bsp_policy policy;
  policy.min_width = 10;
  policy.min_height = 8;
  policy.max_wh_ratio = 2.0;
  policy.max_hw_ratio = 2.0;

  rl_bsp_split(&tree, 0, rng, 0, &policy);

  struct room_graph graph;
  if (!init_room_graph(&graph, &tree, rng)) {
    rl_bsp_tree_free(&tree);
    return false;
  }

  carve_rooms(map, graph.rooms, graph.room_count);
  carve_corridors(map, &graph);

  SDL_free(graph.connections);
  rl_bsp_tree_free(&tree);

  // transfer ownership of rooms
  map->rooms = graph.rooms;
  map->room_count = graph.room_count;

  return true;
}

bool
rl_init_map(struct rl_map* map, int width, int height, struct rand_state* rng)
{
  // check whether map has already been initialized
  SDL_assert(map->tiles == NULL);

  map->tiles = SDL_calloc(width * height, sizeof(*map->tiles));
  if (map->tiles == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  map->width = width;
  map->height = height;

  if (!generate_map(map, rng)) {
    rl_free_map(map);
    return false;
  }

  return true;
}

void
rl_free_map(struct rl_map* map)
{
  if (map == NULL) {
    return;
  }

  SDL_free(map->rooms);
  map->rooms = NULL;
  map->room_count = 0;

  SDL_free(map->tiles);
  map->tiles = NULL;
  map->width = 0;
  map->height = 0;
}

bool
rl_map_contains(struct rl_map const* map, int x, int y)
{
  return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

struct rl_tile
rl_get_tile(struct rl_map const* map, int x, int y)
{
  SDL_assert(rl_map_contains(map, x, y));

  return map->tiles[y * map->width + x];
}
