#pragma once

#include <string>
#include <vector>


namespace sfn {

   struct node{
      std::string m_name;
      std::vector<int> m_connections;
   };

   struct connection{
      int m_node_index0;
      int m_node_index1;
      float m_weight;
      
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

   struct graph;

   struct shortest_path_tree{
      int m_source_node_index;
      std::vector<shortest_path> m_entries;

      explicit shortest_path_tree(const int source_node_index, const int node_count);
      [[nodiscard]] auto get_distance_from_source(const int node_index) const -> float;
      friend auto operator<=>(const shortest_path_tree&, const shortest_path_tree&) = default;
   };

   struct neighbor_info{
      float m_distance;
      int m_other_index;
   };

   struct graph
   {
      std::vector<node> m_nodes;
      std::vector<connection> m_connections;

      [[nodiscard]] auto get_node_index_by_name(const std::string& name) const -> int;
      auto add_connection(const std::string& name_a, const std::string& name_b, const float weight) -> void;
      [[nodiscard]] auto get_dijkstra(const int source_node_index) const -> shortest_path_tree;
      [[nodiscard]] auto are_neighbors(const int node_index_0, const int node_index_1) const -> bool;
      [[nodiscard]] auto get_neighbor_info(const int node_index_0, const int node_index_1) const -> neighbor_info;
   };


}