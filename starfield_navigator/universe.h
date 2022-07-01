#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>


namespace sfn
{

   enum class factions { uc, freestar, crimson };
   enum class info_quality { confirmed, speculation, unknown };

   struct system {
      std::string m_name;
      glm::vec3 m_position;
      info_quality m_info_quality = info_quality::unknown;

      explicit system(const glm::vec3& pos, const std::string& name = "");
   };

   struct universe {
      std::vector<system> m_systems;

      [[nodiscard]] auto get_position_by_name(const std::string& name) const->glm::vec3;
      [[nodiscard]] auto get_index_by_name(const std::string& name) const -> int;
      [[nodiscard]] auto get_distance(const int a, const int b) const -> float;
      [[nodiscard]] auto get_closest(const int system_index) const->std::vector<int>;
      auto print_info() const -> void;
   };

   struct graph;
   auto get_graph_from_universe(const universe& universe, const float jump_range) -> graph;

   [[nodiscard]] auto get_min_jump_dist(const universe& universe, const int start_index, const int dest_index) -> float;
   [[nodiscard]] auto get_absolute_min_jump_range(const universe& universe) -> float;
   [[nodiscard]] auto get_closest_distances_for_all(const universe& universe)->std::vector<float>;

}
