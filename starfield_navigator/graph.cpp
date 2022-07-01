#include "graph.h"

#include "tools.h"

#include <algorithm>
#include <execution>
#include <ranges>



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


auto sfn::shortest_path_tree::get_distance_from_source(const int node_index) const -> float
{
   return m_entries[node_index].m_shortest_distance;
}


auto sfn::jump_path::contains_connection(const connection& con) const -> bool
{
   for(int i=0; i<std::ssize(m_stops)-1; ++i)
   {
      const connection temp{
         .m_node_index0 = m_stops[i],
         .m_node_index1 = m_stops[i+1],
         .m_weight = 0.0f
      };
      if (temp == con)
         return true;
   }
   return false;
}


auto sfn::graph::are_neighbors(const int node_index_0, const int node_index_1) const -> bool
{
   if (node_index_0 == node_index_1)
      return false;

   const auto pred = [&](const int node_index)
   {
      return node_index == node_index_1;
   };
   return std::ranges::any_of(m_nodes[node_index_0].m_neighbor_nodes, pred);
}

