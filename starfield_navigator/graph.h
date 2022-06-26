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
      
      [[nodiscard]] auto contains(const int node_index) const -> bool;
   };
   auto operator==(const connection& a, const connection& b) -> bool;

   struct graph
   {
      std::vector<node> m_nodes;
      std::vector<connection> m_connections;

      [[nodiscard]] auto get_index(const std::string& name) const -> int;
      auto add_connection(const std::string& name_a, const std::string& name_b, const float weight) -> void;
   };


}