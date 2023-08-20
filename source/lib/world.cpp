#include "world.hpp"

world::world(game_context &game_context_ref, nlohmann::json &_config_json) : _game_context_ref(game_context_ref), config_json(_config_json) {
  logger = std::make_unique<logger_decorator>("world", "world.log");

  render_distance = config_json["world"].value("render_distance", 4);
  view_distance = config_json["world"].value("view_distance", 8);

  generate_world_thread = std::thread(&world::generate_world_thread_func, this);
  generate_world_thread_running = true;
}

world::~world() {
  generate_world_thread_running = false;

  if (generate_world_thread.joinable()) {
    generate_world_thread.join();
  }
  std::lock_guard<std::mutex> lock(world_generator_mutex);

  clear();
}

void world::generate_chunk(const int32_t x, const int32_t y, const int32_t z, bool generate_model) {
  // Generate the chunk
  auto start = std::chrono::high_resolution_clock::now();
  std::unique_ptr<chunk> chunk_new = genv2.generate_chunk(x, y, z, true);
  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  logger->trace("Chunk generation (x: {}, y: {}, z: {}) took {}ms", x, y, z, duration.count());

  // Add the chunk to the world
  if (generate_model) {
    generate_chunk_models(*chunk_new.get());
  }
  chunks.push_back(std::move(chunk_new));
}

void world::generate_chunk_models(chunk &chunk_new) {
  auto start = std::chrono::high_resolution_clock::now();
  std::unique_ptr<Model> chunk_model = world_md.generate_chunk_model(chunk_new);
  chunk_model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _game_context_ref._texture;

  chunk_new.set_model(std::move(chunk_model));

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  logger->trace("Chunk model (x: {}, y: {}, z: {}) generation took {}ms", chunk_new.get_position().x, chunk_new.get_position().y, chunk_new.get_position().z,
                duration.count());
}

bool world::is_chunk_exist(const int32_t x, const int32_t y, const int32_t z) const noexcept {
  auto it = std::find_if(chunks.begin(), chunks.end(), [&](const auto &chunk) {
    auto chunk_pos = chunk->get_position();
    return chunk_pos.x == x && chunk_pos.y == y && chunk_pos.z == z;
  });

  return it != chunks.end();
}

void world::clear() {
  // Clear the chunks
  logger->debug("Clearing {} chunks...", chunks.size());
  chunks.clear();
  logger->debug("All chunks have been cleared");
}

void world::updateGameInput() {
  if (IsKeyPressed(KEY_R)) {
    seed = std::random_device()();
    genv2.reseed(this->seed);
    free_world = true;
    logger->info("seed: {}", seed);
  }

  if (IsKeyPressed(KEY_C)) {
    free_world = true;
  }
}

void world::updateGameLogic() {}

void world::updateOpenglLogic() {
  std::lock_guard<std::mutex> lock(world_generator_mutex);
  if (free_world) {
    clear();
    free_world = false;
    return;
  }

  // Check if each chunk are outsite the unload distance, it yes, free it
  for (auto it = chunks.begin(); it != chunks.end(); ++it) {
    auto current_chunk = (*it).get();

    if (current_chunk == nullptr) {
      logger->warn("Chunk is nullptr");
      continue;
    }

    if (!current_chunk->is_active_chunk()) {
      it = chunks.erase(it);
      continue;
    }

    if (!current_chunk->has_model()) {
      generate_chunk_models(*current_chunk);
      continue;
    }
  }
}

void world::updateDraw3d() {
  std::lock_guard<std::mutex> lock(world_generator_mutex);
  /*
  // TODO: optimize to check only the blocks around the player
  for (size_t ci = 0; ci < world_new->chunks.size(); ci++) {
      auto& current_chunk = world_new->chunks[ci];
      auto& blocks = current_chdebug_menu1Box box = block_utils::get_bounding_box(current_block, 1.0f);

              RayCollision box_hit_info = GetRayCollisionBox(ray, box);
              if (box_hit_info.hit) {
#pragma omp critical
                  collisions.push_back({&current_block, box_hit_info});
              }
          }
      }
  }

  if (!collisions.empty()) {
      // sort by distance and get the closest collision
      std::sort(collisions.begin(), collisions.end(), [](const auto& a, const auto& b) { return a.second.distance < b.second.distance; });
      closest_collision = collisions[0].second;
      closest_block = collisions[0].first;

      block_info_pos = closest_block->get_position();
      // block_info_index = closest_block->x + closest_block->z * 16 + closest_block->y * 16 * 16;
      block_info_index = 0;
  } else {
      block_info_pos = {0, 0, 0};
      block_info_index = 0;
  }
  */

  /*
if (closest_collision.hit) {
   DrawCube(closest_collision.point, 0.3f, 0.3f, 0.3f, YELLOW);
   DrawCubeWires(closest_collision.point, 0.3f, 0.3f, 0.3f, BLACK);

   Vector3 normalEnd;
   normalEnd.x = closest_collision.point.x + closest_collision.normal.x;
   normalEnd.y = closest_collision.point.y + closest_collision.normal.y;
   normalEnd.z = closest_collision.point.z + closest_collision.normal.z;

   DrawLine3D(closest_collision.point, normalEnd, BLUE);
}
*/
  for (auto const &_chunk : chunks) {
    if (_chunk.get() == nullptr) {
      logger->warn("Chunk is nullptr");
      continue;
    }

    if (!_chunk->is_visible_chunk() || !_chunk->is_active_chunk()) {
      continue;
    }

    chunk &current_chunk = *_chunk.get();
    auto chunk_coor = current_chunk.get_position();
    [[maybe_unused]] auto &blocks = current_chunk.get_blocks();

    auto &player1_pose = _game_context_ref.player_chunk_pos;

    if (!current_chunk.has_model()) {
      continue;
    }

    Model &current_model = *current_chunk.get_model();

    auto chunk_pos = chunk::get_real_position(current_chunk);

    Vector3 chunk_pos_center = {static_cast<float>(chunk_coor.x * chunk::chunk_size_x + chunk::chunk_size_x / 2.0f),
                                static_cast<float>(chunk_coor.y * chunk::chunk_size_y + chunk::chunk_size_y / 2.0f),
                                static_cast<float>(chunk_coor.z * chunk::chunk_size_z + chunk::chunk_size_z / 2.0f)};

    // For debug menu
    if (*_game_context_ref.display_debug_menu) {
      _game_context_ref.vectices_on_world_count += static_cast<size_t>(current_model.meshes->vertexCount);
      _game_context_ref.triangles_on_world_count += static_cast<size_t>(current_model.meshes->triangleCount);
      _game_context_ref.display_block_count += chunk::chunk_size_x * chunk::chunk_size_y * chunk::chunk_size_z;
      _game_context_ref.display_chunk_count++;
    }

    /*
    const Vector2&& chunk_screen_pos = player1->camera.GetWorldToScreen(chunk_pos_center, player1->camera);

    if (chunk_screen_pos.x < -1000.0 || chunk_screen_pos.x > static_cast<float>(GetScreenWidth()) + 1000.0 || chunk_screen_pos.y < -1000.0
        || chunk_screen_pos.y > static_cast<float>(GetScreenHeight()) + 1000.0)
    {
        continue;
    }
    */

    DrawModelEx(current_model, chunk_pos, {0, 0, 0}, 1.0f, {1, 1, 1}, WHITE);

    if (*_game_context_ref.display_debug_menu) {
      _game_context_ref.chunks_on_screen_count++;
      _game_context_ref.vectices_on_screen_count += current_model.meshes->vertexCount;
      _game_context_ref.triangles_on_screen_count += current_model.meshes->triangleCount;

      chunk_pos.x += static_cast<float>(chunk::chunk_size_x / 2.0f);
      chunk_pos.y += static_cast<float>(chunk::chunk_size_y / 2.0f);
      chunk_pos.z += static_cast<float>(chunk::chunk_size_z / 2.0f);

      DrawCubeWires(chunk_pos, static_cast<float>(chunk::chunk_size_x), static_cast<float>(chunk::chunk_size_y), static_cast<float>(chunk::chunk_size_z), RED);
    }
  }
}

void world::updateDraw2d() {}

void world::updateDrawInterface() {}

void world::generate_world_thread_func() {
  while (generate_world_thread_running) {
    for (int32_t x = -render_distance; x <= render_distance; x++) {
      for (int32_t y = -render_distance; y <= render_distance; y++) {
        for (int32_t z = -render_distance; z <= render_distance; z++) {
          int32_t chunk_x = _game_context_ref.player_chunk_pos.x + x;
          int32_t chunk_y = _game_context_ref.player_chunk_pos.y + y;
          int32_t chunk_z = _game_context_ref.player_chunk_pos.z + z;

          if (is_chunk_exist(chunk_x, chunk_y, chunk_z)) {
            continue;
          }

          std::lock_guard<std::mutex> lock(world_generator_mutex);
          generate_chunk(chunk_x, chunk_y, chunk_z, false);
        }
      }
    }

    // Check if each chunk are outsite the unload distance, it yes, free it
    for (auto it = chunks.begin(); it != chunks.end(); ++it) {
      auto current_chunk = (*it).get();
      if (current_chunk == nullptr) {
        logger->warn("Chunk is nullptr");
        continue;
      }
      auto chunk_coor = current_chunk->get_position();
      auto player_chunk_pos = _game_context_ref.player_chunk_pos;

      // If chunk is too far away, free it
      if (std::abs(chunk_coor.x - player_chunk_pos.x) > unload_distance || std::abs(chunk_coor.y - player_chunk_pos.y) > unload_distance ||
          std::abs(chunk_coor.z - player_chunk_pos.z) > unload_distance) {
        current_chunk->set_active_chunk(false);
        continue;
      }

      // If chunk is too far away, don't render it
      if (std::abs(chunk_coor.x - player_chunk_pos.x) > view_distance || std::abs(chunk_coor.y - player_chunk_pos.y) > view_distance ||
          std::abs(chunk_coor.z - player_chunk_pos.z) > view_distance) {
        current_chunk->set_visible_chunk(false);
        continue;
      }
      current_chunk->set_visible_chunk(true);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  logger->info("World generation thread stopped");
}

