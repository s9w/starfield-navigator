#include "graph.h"


auto sfn::connection::contains(const int node_index) const -> bool
{
   return m_node_index0 == node_index || m_node_index1 == node_index;
}


auto sfn::operator==(const connection& a, const connection& b) -> bool
{
   const bool equal = a.m_node_index0 == b.m_node_index0 && a.m_node_index1 == b.m_node_index1;
   const bool equal_reversed = a.m_node_index0 == b.m_node_index1 && a.m_node_index1 == b.m_node_index0;
   return equal || equal_reversed;
}


auto sfn::graph::get_index(const std::string& name) const -> int
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
   const auto index_a = this->get_index(name_a);
   const auto index_b = this->get_index(name_b);
   const connection new_connection{
         .m_node_index0 = index_a,
         .m_node_index1 = index_b,
         .m_weight = weight
   };
   const auto it = std::ranges::find(m_connections, new_connection);
   if (it != std::end(m_connections))
      std::terminate();

   m_connections.push_back(new_connection);
   const int latest_index = static_cast<int>(std::ssize(m_connections)) - 1;

   for(int i=0; i<std::ssize(m_nodes); ++i)
   {
      if (new_connection.contains(i))
         m_nodes[i].m_connections.push_back(latest_index);
   }
}
