#ifndef WORLD_OF_CUBE_WORLD_MODEL_HPP
#define WORLD_OF_CUBE_WORLD_MODEL_HPP

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include <omp.h>
#include <rlgl.h>

#include "raylib-cpp.hpp"
#include "raylib.h"
#include "raymath.h"
// memcpy
#include <stdio.h>
#include <string.h>

// Cube lib
#include "block.hpp"
#include "chunk.hpp"
#include "math.hpp"

class world_model
{
    public:
        world_model() {}

        ~world_model() {}

        static constexpr int south_face = 0;
        static constexpr int north_face = 1;
        static constexpr int west_face = 2;
        static constexpr int east_face = 3;
        static constexpr int up_face = 4;
        static constexpr int down_face = 5;

        inline void add_vertex(
            Mesh& mesh, size_t& triangle_index, size_t& vert_index, Vector3& vertex, Vector3 offset, Vector3 normal, Vector2 texcoords)
        {
            size_t index = triangle_index * 12 + vert_index * 3;

            index = triangle_index * 6 + vert_index * 2;
            mesh.texcoords[index] = texcoords.x;
            mesh.texcoords[index + 1] = texcoords.y;

            index = triangle_index * 9 + vert_index * 3;
            mesh.normals[index] = normal.x;
            mesh.normals[index + 1] = normal.y;
            mesh.normals[index + 2] = normal.z;

            index = triangle_index * 9 + vert_index * 3;
            mesh.vertices[index] = vertex.x + offset.x;
            mesh.vertices[index + 1] = vertex.y + offset.y;
            mesh.vertices[index + 2] = vertex.z + offset.z;

            vert_index++;
            if (vert_index > 2) {
                triangle_index++;
                vert_index = 0;
            }
        }

        void add_cube(Mesh& mesh, size_t& triangle_index, size_t& vert_index, Vector3&& position, bool faces[6], int block)
        {
            Rectangle texcoords_rect = Rectangle {0.25f, 0, 0.5f, 1};
            // z-
            if (faces[north_face]) {
                Vector3 normal = Vector3 {0, 0, -1};

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                // Issue with this triangle
                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.width});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 0}, normal, {texcoords_rect.x, texcoords_rect.height});

                // Seems to be fixed here
                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 0}, normal, {texcoords_rect.y, texcoords_rect.width});
            }

            // z+
            if (faces[south_face]) {
                Vector3 normal = Vector3 {0, 0, 1};

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 1}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 1}, normal, {texcoords_rect.x, texcoords_rect.height});
            }

            // x+
            if (faces[west_face]) {
                Vector3 normal = Vector3 {1, 0, 0};

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});
            }

            // x-
            if (faces[east_face]) {
                Vector3 normal = Vector3 {-1, 0, 0};

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.y});
            }

            // y+
            if (faces[up_face]) {
                Vector3 normal = Vector3 {0, 1, 0};

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 1, 1}, normal, {texcoords_rect.x, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 1, 1}, normal, {texcoords_rect.width, texcoords_rect.height});
            }

            // y-
            if (faces[down_face]) {
                Vector3 normal = Vector3 {0, -1, 0};

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 0}, normal, {texcoords_rect.width, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 0}, normal, {texcoords_rect.x, texcoords_rect.y});

                add_vertex(mesh, triangle_index, vert_index, position, {1, 0, 1}, normal, {texcoords_rect.width, texcoords_rect.height});

                add_vertex(mesh, triangle_index, vert_index, position, {0, 0, 1}, normal, {texcoords_rect.x, texcoords_rect.height});
            }
        }

        std::vector<Model> generate_world_models(std::vector<chunk>& chunks)
        {
            std::vector<Model> models;
            std::vector<Mesh> meshes;
// Generate meshes on multiple threads
#pragma omp for ordered schedule(static, 1)
            for (size_t i = 0; i < chunks.size(); i++) {
                auto&& mesh = chunk_mesh(chunks, i);
#pragma omp ordered
                meshes.push_back(mesh);
            }

            // Generate models on mono thread (OpenGL)
            for (size_t i = 0; i < meshes.size(); i++) {
                UploadMesh(&meshes[i], false);
                Model world_model = LoadModelFromMesh(meshes[i]);
                models.push_back(world_model);
            }
            return models;
        }

        inline constexpr bool block_is_solid(int x, int y, int z, chunk& _chunk)
        {
            // Check out of bounds
            if (x < 0 || x >= chunk::chunk_size_x) {
                return false;
            }
            if (y < 0 || y >= chunk::chunk_size_y) {
                return false;
            }
            if (z < 0 || z >= chunk::chunk_size_z) {
                return false;
            }

            // Check if block type is not air
            return _chunk.get_block(x, y, z).block_type != block_type::air;
        }

        // Count the number of solid blocks around a block (excluding diagonals and corners)
        inline constexpr int count_neighbours(int x, int y, int z, chunk& _chunk)
        {
            int count = 0;
            if (block_is_solid(x - 1, y, z, _chunk))
                count++;
            if (block_is_solid(x + 1, y, z, _chunk))
                count++;
            if (block_is_solid(x, y - 1, z, _chunk))
                count++;
            if (block_is_solid(x, y + 1, z, _chunk))
                count++;
            if (block_is_solid(x, y, z - 1, _chunk))
                count++;
            if (block_is_solid(x, y, z + 1, _chunk))
                count++;
            return count;
        }

        inline constexpr int count_border(int x, int y, int z, chunk& _chunk)
        {
            int count = 0;
            if (x == 0)
                count++;
            if (x == chunk::chunk_size_x - 1)
                count++;
            if (y == 0)
                count++;
            if (y == chunk::chunk_size_y - 1)
                count++;
            if (z == 0)
                count++;
            if (z == chunk::chunk_size_z - 1)
                count++;
            return count;
        }

        inline int face_count(chunk& _chunk)
        {
            int count = 0;
            auto& blocks = _chunk.get_blocks();

            for (int x = 0; x < chunk::chunk_size_x; x++) {
                for (int y = 0; y < chunk::chunk_size_y; y++) {
                    for (int z = 0; z < chunk::chunk_size_z; z++) {
                        block& current_block = _chunk.get_block(x, y, z);
                        if (current_block.block_type == block_type::air)
                            continue;
                        int border_count = count_border(x, y, z, _chunk);
                        int neighbour_count = count_neighbours(x, y, z, _chunk);

                        if (current_block.block_type == block_type::air || neighbour_count + border_count == 6) {
                            continue;
                        }

                        if (!block_is_solid(x - 1, y, z, _chunk))
                            count++;

                        if (!block_is_solid(x + 1, y, z, _chunk))
                            count++;

                        if (!block_is_solid(x, y - 1, z, _chunk))
                            count++;

                        if (!block_is_solid(x, y + 1, z, _chunk))
                            count++;

                        if (!block_is_solid(x, y, z + 1, _chunk))
                            count++;

                        if (!block_is_solid(x, y, z - 1, _chunk))
                            count++;
                    }
                }
            }
            return count;
        }

        Mesh chunk_mesh(std::vector<chunk>& chunks, size_t chunk_index)
        {
            chunk& _chunk = chunks[chunk_index];
            Mesh mesh = {0};
            int faces_count = face_count(_chunk);
            mesh.vertexCount = faces_count * 6;
            mesh.triangleCount = faces_count * 2;

            mesh.vertices = static_cast<float*>(MemAlloc(sizeof(float) * 3 * mesh.vertexCount));
            mesh.normals = static_cast<float*>(MemAlloc(sizeof(float) * 3 * mesh.vertexCount));
            mesh.texcoords = static_cast<float*>(MemAlloc(sizeof(float) * 2 * mesh.vertexCount));

            size_t triangle_index = 0;
            size_t vert_index = 0;

            auto& blocks = _chunk.get_blocks();

            for (int x = 0; x < chunk::chunk_size_x; x++) {
                for (int y = 0; y < chunk::chunk_size_y; y++) {
                    for (int z = 0; z < chunk::chunk_size_z; z++) {
                        block& current_block = _chunk.get_block(x, y, z);
                        if (current_block.block_type == block_type::air)
                            continue;

                        int border_count = count_border(x, y, z, _chunk);
                        int neighbour_count = count_neighbours(x, y, z, _chunk);

                        current_block.neighbors = neighbour_count;
                        current_block.edges = border_count;

                        if (current_block.x == 48 && current_block.y == 36 && current_block.z == 50) {
                            std::cout << "Block: " << current_block.x << ", " << current_block.y << ", " << current_block.z << std::endl;
                            std::cout << "Neighbours: " << neighbour_count << std::endl;
                            std::cout << "Borders: " << border_count << std::endl;
                        }

                        if (neighbour_count + border_count == 6) {
                            continue;
                        }

                        bool faces[6] = {false, false, false, false, false, false};

                        faces[world_model::east_face] = !block_is_solid(x - 1, y, z, _chunk);

                        faces[world_model::west_face] = !block_is_solid(x + 1, y, z, _chunk);

                        faces[world_model::down_face] = !block_is_solid(x, y - 1, z, _chunk);

                        faces[world_model::up_face] = !block_is_solid(x, y + 1, z, _chunk);

                        faces[world_model::south_face] = !block_is_solid(x, y, z + 1, _chunk);

                        faces[world_model::north_face] = !block_is_solid(x, y, z - 1, _chunk);

                        add_cube(mesh, triangle_index, vert_index, {(float)x, (float)y, (float)z}, faces, 1);
                    }
                }
            }
            return mesh;
        }
};

#endif  // WORLD_OF_CUBE_WORLD_HPP