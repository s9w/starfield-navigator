#pragma once

#include <string>
#include <vector>
#include <optional>

#include "tools.h"

#include <glm/vec3.hpp>


namespace sfn
{

   enum class factions { uc, freestar, crimson };
   enum class system_size{big, small};
   enum class position_mode{reconstructed, from_catalog};

   struct system {
      std::string m_name;
      std::string m_astronomic_name;
      std::string m_catalog_lookup;
      glm::vec3 m_reconstructed_position;
      glm::vec3 m_catalog_position;
      system_size m_size = system_size::big;
      float m_abs_mag;
      bool m_speculative = false;

      [[nodiscard]] auto get_useful_name() const -> std::optional<std::string>;
      [[nodiscard]] auto get_name() const -> std::string;
      [[nodiscard]] auto get_starfield_name() const -> std::optional<std::string>;
      [[nodiscard]] auto get_position(const position_mode mode) const -> glm::vec3;

      explicit system(const glm::vec3& pos, const std::string& name, const std::string& astronomic_name, const std::string& catalog, const system_size size, const float abs_mag, const bool speculative);
   };

   struct cs
   {
      glm::vec3 m_front{};
      glm::vec3 m_up{};
      glm::vec3 m_right{};
      cs() = default;
      explicit cs(const glm::vec3& front, const glm::vec3& up); // TODO: constexpr
   };

   struct cam_info
   {
      cs m_cs;
      glm::vec3 m_cam_pos0;
      glm::vec3 m_cam_pos1;
   };

   struct universe {
      std::vector<system> m_systems;
      cam_info m_cam_info;
      float m_min_abs_mag;
      float m_max_abs_mag;
      glm::mat4 m_trafo;
      bb_3D m_map_bb;
      bb_3D m_left_bb;

      auto init() -> void;
      [[nodiscard]] auto get_position_by_name(const std::string& name, const position_mode mode) const -> glm::vec3;
      [[nodiscard]] auto get_index_by_name(const std::string& name) const -> int;
      [[nodiscard]] auto get_distance(const int a, const int b, const position_mode mode) const -> float;
   };

   struct graph;
   auto get_graph_from_universe(const universe& universe, const float jump_range) -> graph;

   [[nodiscard]] auto get_min_jump_dist(const universe& universe, const int start_index, const int dest_index, const position_mode mode) -> float;
   [[nodiscard]] auto get_absolute_min_jump_range(const universe& universe, const position_mode mode) -> float;

}
