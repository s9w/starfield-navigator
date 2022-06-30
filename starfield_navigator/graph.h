#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include <glm/vec3.hpp>

#include "tools.h"


namespace sfn {

   enum class factions{uc, freestar, crimson};
   enum class info_quality{confirmed, speculation, unknown};

   struct system{
      std::string m_name;
      glm::vec3 m_position;
      info_quality m_info_quality = info_quality::unknown;

      explicit system(const glm::vec3& pos, const std::string& name = "");
   };

   struct universe{
      std::vector<system> m_systems;

      [[nodiscard]] auto get_position_by_name(const std::string& name) const -> glm::vec3;
      [[nodiscard]] auto get_index_by_name(const std::string& name) const -> int;
      [[nodiscard]] auto get_distance(const int a, const int b) const -> float;
      [[nodiscard]] auto get_closest(const int system_index) const -> std::vector<int>;
      auto print_info() const -> void;
   };




   struct node{
      int m_index;
      std::vector<int> m_neighbor_nodes;
   };

   struct connection{
      int m_node_index0;
      int m_node_index1;
      float m_distance;
      
      [[nodiscard]] auto contains_node_index(const int node_index) const -> bool;
   };
   auto operator==(const connection& a, const connection& b) -> bool;

   struct shortest_path{
      // int m_target_index;
      constexpr static inline float no_distance = std::numeric_limits<float>::max();
      float m_shortest_distance = no_distance;
      int m_previous_vertex_index = -1;
   };
   [[nodiscard]] auto operator==(const shortest_path& a, const shortest_path& b) -> bool;
   

   struct shortest_path_tree{
      int m_source_node_index;
      std::vector<shortest_path> m_entries;

      explicit shortest_path_tree(const int source_node_index, const int node_count);
      [[nodiscard]] auto get_distance_from_source(const int node_index) const -> float;
      friend auto operator<=>(const shortest_path_tree&, const shortest_path_tree&) = default;
   };

   struct jump_path{
      std::vector<int> m_stops;

      [[nodiscard]] auto contains_connection(const connection& con) const -> bool;
   };

   struct MyHashFunction {
      constexpr auto operator()(const id& p) const -> size_t{
         return p.m_id;
      }
   };

   struct graph
   {
      float m_jump_range = 0.0f;
      std::vector<node> m_nodes;
      std::unordered_map<id, connection, MyHashFunction> m_connections;
      std::vector<id> m_sorted_connections;

      explicit graph() = default;
      explicit graph(const universe& universe, const float jump_range);

      [[nodiscard]] auto get_dijkstra(const int source_node_index, const universe& universe) const -> shortest_path_tree;
      [[nodiscard]] auto are_neighbors(const int node_index_0, const int node_index_1) const -> bool;
      [[nodiscard]] auto get_jump_path(const int start_index, const int destination_index, const universe& universe) const -> std::optional<jump_path>;
   };

   [[nodiscard]] auto get_min_jump_dist(const universe& universe, const int start_index, const int dest_index) -> float;
   [[nodiscard]] auto get_absolute_min_jump_range(const universe& universe) -> float;
   [[nodiscard]] auto get_closest_distances_for_all(const universe& universe) -> std::vector<float>;

}