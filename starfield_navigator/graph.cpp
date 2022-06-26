#include "graph.h"

#include "tools.h"

#include <algorithm>


auto sfn::connection::contains_node_index(const int node_index) const -> bool
{
   return m_node_index0 == node_index || m_node_index1 == node_index;
}


auto sfn::operator==(const connection& a, const connection& b) -> bool
{
   const bool equal = a.m_node_index0 == b.m_node_index0 && a.m_node_index1 == b.m_node_index1;
   const bool equal_reversed = a.m_node_index0 == b.m_node_index1 && a.m_node_index1 == b.m_node_index0;
   return equal || equal_reversed;
}


auto sfn::operator==(const shortest_path& a, const shortest_path& b) -> bool
{
   return a.m_previous_vertex_index == b.m_previous_vertex_index &&
      equal(a.m_shortest_distance, b.m_shortest_distance);
}


sfn::shortest_path_tree::shortest_path_tree(const int source_node_index, const int node_count)
   : m_source_node_index(source_node_index)
   , m_entries(node_count)
{
   m_entries[source_node_index].m_shortest_distance = 0;
}


auto sfn::shortest_path_tree::get_distance_from_source(const int node_index) -> float
{
   return m_entries[node_index].m_shortest_distance;
}


auto sfn::graph::get_node_index_by_name(const std::string& name) const -> int
{
   for(int i=0; i<std::ssize(m_nodes); ++i)
   {
      if(name == m_nodes[i].m_name)
      {
         return i;
      }
   }
   std::terminate();
}


auto sfn::graph::add_connection(const std::string& name_a, const std::string& name_b, const float weight) -> void
{
   const auto node_index_a = this->get_node_index_by_name(name_a);
   const auto node_index_b = this->get_node_index_by_name(name_b);
   const connection new_connection{
         .m_node_index0 = node_index_a,
         .m_node_index1 = node_index_b,
         .m_weight = weight
   };
   const auto it = std::ranges::find(m_connections, new_connection);
   if (it != std::end(m_connections))
      std::terminate();

   m_connections.push_back(new_connection);
   const int latest_index = static_cast<int>(std::ssize(m_connections)) - 1;

   for(int i=0; i<std::ssize(m_nodes); ++i)
   {
      if (new_connection.contains_node_index(i))
         m_nodes[i].m_connections.push_back(latest_index);
   }
}


auto sfn::graph::get_dijkstra(const int source_node_index) const -> shortest_path_tree
{
   shortest_path_tree tree(source_node_index, static_cast<int>(std::ssize(m_nodes)));

   // init
   std::vector<int> visited;
   std::vector<int> unvisited;
   unvisited.reserve(std::ssize(m_nodes));
   for (int i = 0; i < std::ssize(m_nodes); ++i)
      unvisited.push_back(i);

   int current_vertex = source_node_index;

   while(unvisited.empty() == false)
   {
      std::vector<int> current_vertex_neighbors;
      for (int i = 0; i < std::ssize(m_nodes); ++i)
      {
         if(this->are_neighbors(current_vertex, i) == false)
            continue;
         if(std::ranges::find(visited, i) != visited.cend())
            continue;
         current_vertex_neighbors.push_back(i);
      }

      for(const int neighbor : current_vertex_neighbors)
      {
         const auto info = this->get_neighbor_info(current_vertex, neighbor);
         const float distance = tree.get_distance_from_source(info.m_other_index) + info.m_distance;
         if (distance < tree.m_entries[neighbor].m_shortest_distance)
         {
            tree.m_entries[neighbor].m_shortest_distance = distance;
            tree.m_entries[neighbor].m_previous_vertex_index = current_vertex;
         }
      }

      visited.push_back(current_vertex);
      std::erase(unvisited, current_vertex);

      if (unvisited.empty())
         break;

      // sort unvisited
      const auto pred = [&](const int a, const int b){
         return tree.get_distance_from_source(a) < tree.get_distance_from_source(b);
      };
      std::ranges::sort(unvisited, pred);

      current_vertex = unvisited.front();
   }

   return tree;
}


auto sfn::graph::are_neighbors(const int node_index_0, const int node_index_1) const -> bool
{
   if (node_index_0 == node_index_1)
      return false;
   for(const auto a_connect_index : m_nodes[node_index_0].m_connections)
   {
      const connection& connection = m_connections[a_connect_index];
      if (connection.contains_node_index(node_index_1))
         return true;
   }
   return false;
}


auto sfn::graph::get_neighbor_info(const int node_index_0, const int node_index_1) const -> neighbor_info
{
   if (node_index_0 == node_index_1)
      std::terminate();
   for (const auto a_connect_index : m_nodes[node_index_0].m_connections)
   {
      const connection& connection = m_connections[a_connect_index];
      if (connection.contains_node_index(node_index_1))
      {
         const int other_index = (connection.m_node_index0 == node_index_1) ? connection.m_node_index1 : connection.m_node_index0;
         return neighbor_info{
               .m_distance = connection.m_weight,
               .m_other_index = other_index
         };
      }
   }
   std::terminate();
}
