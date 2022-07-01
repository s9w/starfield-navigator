#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include "tools.h"


namespace sfn {

   struct node{
      int m_index;
      std::vector<int> m_neighbor_nodes;
   };

   struct connection{
      int m_node_index0;
      int m_node_index1;
      float m_weight;
      
      [[nodiscard]] auto contains_node_index(const int node_index) const -> bool;
   };
   auto operator==(const connection& a, const connection& b) -> bool;

   struct shortest_path{
      constexpr static inline float no_distance = std::numeric_limits<float>::max();
      float m_shortest_distance = no_distance;
      std::optional<int> m_previous_vertex_index;
   };
   [[nodiscard]] auto operator==(const shortest_path& a, const shortest_path& b) -> bool;
   

   struct shortest_path_tree{
      int m_source_node_index;
      std::vector<shortest_path> m_entries;

      explicit shortest_path_tree(const int source_node_index, const int node_count);
      [[nodiscard]] auto get_distance_from_source(const int node_index) const -> float;
   };

   struct jump_path{
      std::vector<int> m_stops;

      [[nodiscard]] auto contains_connection(const connection& con) const -> bool;
   };

   struct graph
   {
      float m_jump_range = 0.0f;
      std::vector<node> m_nodes;
      std::unordered_map<id, connection, id_hash_callable> m_connections;
      std::vector<id> m_sorted_connections;

      explicit graph() = default;

      template<typename T>
      [[nodiscard]] auto get_dijkstra(const int source_node_index, const T& weight_getter) const -> shortest_path_tree;

      template<typename T>
      [[nodiscard]] auto get_jump_path(const int start_index, const int destination_index, const T& weight_getter) const -> std::optional<jump_path>;

   private:
      [[nodiscard]] auto are_neighbors(const int node_index_0, const int node_index_1) const -> bool;
   };
}


template<typename T>
[[nodiscard]] auto sfn::graph::get_dijkstra(
   const int source_node_index,
   const T& weight_getter
) const -> shortest_path_tree
{
   shortest_path_tree tree(source_node_index, static_cast<int>(std::ssize(m_nodes)));

   std::vector<int> visited;
   visited.reserve(std::ssize(m_nodes));
   std::vector<int> unvisited;
   unvisited.reserve(std::ssize(m_nodes));
   for (int i = 0; i < std::ssize(m_nodes); ++i)
      unvisited.push_back(i);

   int current_vertex = source_node_index;

   while (unvisited.empty() == false)
   {
      std::vector<int> current_vertex_neighbors;
      current_vertex_neighbors.reserve(10);
      for (int i = 0; i < std::ssize(m_nodes); ++i)
      {
         if (this->are_neighbors(current_vertex, i) == false)
            continue;
         if (std::ranges::find(visited, i) != visited.cend())
            continue;
         current_vertex_neighbors.push_back(i);
      }

      for (const int neighbor : current_vertex_neighbors)
      {
         const float weight = tree.get_distance_from_source(current_vertex) + weight_getter(current_vertex, neighbor);
         if (weight < tree.m_entries[neighbor].m_shortest_distance)
         {
            tree.m_entries[neighbor].m_shortest_distance = weight;
            tree.m_entries[neighbor].m_previous_vertex_index = current_vertex;
         }
      }

      visited.push_back(current_vertex);
      std::erase(unvisited, current_vertex);

      if (unvisited.empty())
         break;

      // sort unvisited
      const auto pred = [&](const int a, const int b) {
         return tree.get_distance_from_source(a) < tree.get_distance_from_source(b);
      };
      std::ranges::sort(unvisited, pred);

      current_vertex = unvisited.front();
   }

   return tree;
}


template<typename T>
[[nodiscard]] auto sfn::graph::get_jump_path(
   const int start_index,
   const int destination_index,
   const T& weight_getter
) const -> std::optional<jump_path>
{
   const shortest_path_tree tree = this->get_dijkstra(start_index, weight_getter);

   jump_path result;
   result.m_stops.reserve(10);
   std::optional<int> position = destination_index;

   while (*position != start_index)
   {
      result.m_stops.push_back(*position);
      position = tree.m_entries[*position].m_previous_vertex_index;
      if (position.has_value() == false)
      {
         return std::nullopt;
      }
   }
   result.m_stops.push_back(start_index);
   std::ranges::reverse(result.m_stops);

   return result;
}
