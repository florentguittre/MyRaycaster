/*
** EPITECH PROJECT, 2023
** MyRayCaster
** File description:
** cast_rays.c
*/

#include <math.h>
#include "../include/structs.h"
#include "../include/prototypes.h"

#define P2 M_PI / 2
#define P3 3 * M_PI / 2

static void get_vertical_ray_pos(sfVector2f *ray_pos, float ray_angle, raycaster_t *raycaster, sfVector2f *next)
{
    float tan_value = tan(deg_to_rad(ray_angle));

    raycaster->tries = 0;
    if (cos(deg_to_rad(ray_angle)) > 0.001) {
        ray_pos->x = (((int)raycaster->player->pos.x >> raycaster->power_2.y) << raycaster->power_2.y) + raycaster->block_size.y; // 64
        ray_pos->y = (raycaster->player->pos.x - ray_pos->x) * tan_value + raycaster->player->pos.y;
        next->x = raycaster->block_size.y; // 64
        next->y = -next->x * tan_value;
    } else if (cos(deg_to_rad(ray_angle)) < -0.001) {
        ray_pos->x = (((int)raycaster->player->pos.x >> raycaster->power_2.y) << raycaster->power_2.y) - 0.0001;
        ray_pos->y = (raycaster->player->pos.x - ray_pos->x) * tan_value + raycaster->player->pos.y;
        next->x = -raycaster->block_size.y; // 64
        next->y = -next->x * tan_value;
    } else {
        *ray_pos = raycaster->player->pos;
        raycaster->tries = raycaster->max_tries.y;
    }
}

static void get_horizontal_ray_pos(sfVector2f *ray_pos, float ray_angle, raycaster_t *raycaster, sfVector2f *next)
{
    float tan_value = 1.0 / tan(deg_to_rad(ray_angle));

    raycaster->tries = 0;
    if (sin(deg_to_rad(ray_angle)) > 0.001) {
        ray_pos->y = (((int)raycaster->player->pos.y >> raycaster->power_2.x) << raycaster->power_2.x) - 0.0001;
        ray_pos->x = (raycaster->player->pos.y - ray_pos->y) * tan_value + raycaster->player->pos.x;
        next->y = -raycaster->block_size.x; // 64
        next->x = -next->y * tan_value;
    } else if (sin(deg_to_rad(ray_angle)) < -0.001) {
        ray_pos->y = (((int)raycaster->player->pos.y >> raycaster->power_2.x) << raycaster->power_2.x) + raycaster->block_size.x; // 64
        ray_pos->x = (raycaster->player->pos.y - ray_pos->y) * tan_value + raycaster->player->pos.x;
        next->y = raycaster->block_size.x; // 64
        next->x = -next->y * tan_value;
    } else {
        *ray_pos = raycaster->player->pos;
        raycaster->tries = raycaster->max_tries.x;
    }
}

static float check_horizontal_line(raycaster_t *raycaster, sfVector2f *ray_pos, float ray_angle)
{
    sfVector2f next = {0};
    sfVector2i m = {0};
    float distance = 1000000;
    int mp = 0;

    get_horizontal_ray_pos(ray_pos, ray_angle, raycaster, &next);
    while (raycaster->tries < raycaster->max_tries.x) {
        m.x = (int)(ray_pos->x) >> raycaster->power_2.x;
        m.y = (int)(ray_pos->y) >> raycaster->power_2.y;
        mp = m.y * raycaster->map_size.x + m.x;
        if (mp > 0 && mp < raycaster->map_surface && map[mp] == 1) {
            distance = calc_distance(&raycaster->player->pos, ray_pos, ray_angle);
            raycaster->tries = raycaster->max_tries.x;
        } else {
            ray_pos->x += next.x;
            ray_pos->y += next.y;
            raycaster->tries++;
        }
    }
    return (distance);
}

static float check_vertical_line(raycaster_t *raycaster, sfVector2f *ray_pos, float ray_angle)
{
    sfVector2f next = {0};
    sfVector2i m = {0};
    float distance = 1000000;
    int mp = 0;

    get_vertical_ray_pos(ray_pos, ray_angle, raycaster, &next);
    while (raycaster->tries < raycaster->max_tries.y) {
        m.x = (int)(ray_pos->x) >> raycaster->power_2.x;
        m.y = (int)(ray_pos->y) >> raycaster->power_2.y;
        mp = m.y * raycaster->map_size.y + m.x;
        if (mp > 0 && mp < raycaster->map_surface && map[mp] == 1) {
            distance = calc_distance(&raycaster->player->pos, ray_pos, ray_angle);
            raycaster->tries = raycaster->max_tries.y;
        } else {
            ray_pos->x += next.x;
            ray_pos->y += next.y;
            raycaster->tries++;
        }
    }
    return (distance);
}

static void create_3d_wall(float distance, int i, raycaster_t *raycaster, sfColor *color)
{
    sfVertex vertex = {0};
    float height = (raycaster->map_surface * 540) / fabs(distance);

    vertex.color = *color;
    vertex.position.x = raycaster->wall_size.x * i;
    vertex.position.y = ((double)raycaster->mode.height / 2.0) - (height / 2.0);
    sfVertexArray_append(raycaster->walls_3d, vertex);
    vertex.color = *color;
    vertex.position.x = (raycaster->wall_size.x * i) + raycaster->wall_size.x;
    vertex.position.y = ((double)raycaster->mode.height / 2.0) - (height / 2.0);
    sfVertexArray_append(raycaster->walls_3d, vertex);
    vertex.color = *color;
    vertex.position.x = (raycaster->wall_size.x * i) + raycaster->wall_size.x;
    vertex.position.y = ((double)raycaster->mode.height / 2.0) + (height / 2.0);
    sfVertexArray_append(raycaster->walls_3d, vertex);
    vertex.color = *color;
    vertex.position.x = (raycaster->wall_size.x * i);
    vertex.position.y = ((double)raycaster->mode.height / 2.0) + (height / 2.0);
    sfVertexArray_append(raycaster->walls_3d, vertex);
}

void cast_rays(raycaster_t *raycaster)
{
    float h_distance = 0;
    float v_distance = 0;
    sfVertex vertex = {0};
    sfVector2f h_pos = {0};
    sfVector2f v_pos = {0};
    sfVector2f ray_pos = {0};
    float ray_dist = 0.01;
    float ray_angle = (raycaster->player->angle - ((double)raycaster->rays_nb / 2.0) * ray_dist);
    float shortest_distance = 0;
    sfColor color = {0};

    sfVertexArray_clear(raycaster->rays_2d);
    sfVertexArray_setPrimitiveType(raycaster->rays_2d, sfLines);
    sfVertexArray_clear(raycaster->walls_3d);
    sfVertexArray_setPrimitiveType(raycaster->walls_3d, sfQuads);
    for (int i = 0; i < raycaster->rays_nb; i++) {
        h_distance = check_horizontal_line(raycaster, &h_pos, ray_angle);
        v_distance = check_vertical_line(raycaster, &v_pos, ray_angle);
        ray_pos = (h_distance < v_distance) ? h_pos : v_pos;
        vertex.position = raycaster->player->pos;
        vertex.color = sfGreen;
        sfVertexArray_append(raycaster->rays_2d, vertex);
        vertex.position = ray_pos;
        vertex.color = sfGreen;
        sfVertexArray_append(raycaster->rays_2d, vertex);
        ray_angle += 0.125;
        shortest_distance = (h_distance < v_distance) ? h_distance : v_distance;
        color = (h_distance < v_distance) ? sfGreen : sfRed;
        create_3d_wall(shortest_distance, i, raycaster, &color);
    }
}
