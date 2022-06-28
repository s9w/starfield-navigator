#include "graph.h"

#include "tools.h"

#include <algorithm>

#pragma warning(push, 0)    
#include <ranges>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#pragma warning(pop)

namespace
{
   using namespace sfn;

   auto get_name_iq(const std::string& name) -> info_quality
   {
      const static std::vector<std::string> known_systems{
         "SOL", "NARION", "ALPHA CENTAURI", "CHEYENNE", "PORRIMA", "VOLII", "JAFFA"
      };
      if (name.starts_with("User"))
         return info_quality::unknown;
      if (std::ranges::find(known_systems, name) != std::end(known_systems))
         return info_quality::confirmed;
      return info_quality::speculation;
   }

}

sfn::system::system(const glm::vec3& pos, const std::string& name)
   : m_name(name)
   , m_position(pos)
   , m_info_quality(get_name_iq(name))
{
   static int unnamed_count = 0;
   if (name.empty())
      m_name = std::format("UNNAMED {}", unnamed_count++);
}


auto sfn::universe::get_position_by_name(const std::string& name) const -> glm::vec3
{
   return m_systems[get_index_by_name(name)].m_position;
}


auto sfn::universe::get_index_by_name(const std::string& name) const -> int
{
   for(int i=0; i<std::ssize(m_systems); ++i)
   {
      if (m_systems[i].m_name == name)
         return i;
   }
   std::terminate();
}


auto universe::get_distance(const int a, const int b) const -> float
{
   return glm::distance(
      m_systems[a].m_position,
      m_systems[b].m_position
   );
}


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


auto jump_path::contains_connection(const connection& con) const -> bool
{
   for(int i=0; i<std::ssize(m_stops)-1; ++i)
   {
      const connection temp{
         .m_node_index0 = m_stops[i],
         .m_node_index1 = m_stops[i+1],
         .m_distance = 0.0f
      };
      if (temp == con)
         return true;
   }
   return false;
}


sfn::graph::graph(const universe& universe, const float jump_range)
   : m_jump_range(jump_range)
{
   m_nodes.reserve(universe.m_systems.size());
   m_connections.reserve(universe.m_systems.size() * universe.m_systems.size());
   m_sorted_connections.reserve(universe.m_systems.size() * universe.m_systems.size());

   const float jump_range2 = jump_range * jump_range;
   for(const system& system : universe.m_systems)
   {
      m_nodes.push_back(
         node{
            .m_name = system.m_name,
            .m_position = system.m_position
         }
      );
   }

   for(int i=0; i<universe.m_systems.size(); ++i)
   {
      for (int j = i+1; j < universe.m_systems.size(); ++j)
      {
         const float distance2 = glm::distance2(universe.m_systems[i].m_position, universe.m_systems[j].m_position);
         if(distance2 > jump_range2)
            continue;

         const id connection_id = id::create();
         m_connections.emplace(
            connection_id,
            connection{
               .m_node_index0 = i,
               .m_node_index1 = j,
               .m_distance = std::sqrt(distance2)
            }
         );
         m_sorted_connections.emplace_back(connection_id);

         m_nodes[i].m_neighbor_nodes.push_back(j);
         m_nodes[j].m_neighbor_nodes.push_back(i);
      }
   }

   const auto pred = [&](const id& a, const id& b) {
      return m_connections.at(a).m_distance < m_connections.at(b).m_distance;
   };
   std::ranges::sort(m_sorted_connections, pred);
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


auto sfn::graph::get_dijkstra(const int source_node_index) const -> shortest_path_tree
{
   shortest_path_tree tree(source_node_index, static_cast<int>(std::ssize(m_nodes)));

   std::vector<int> visited;
   visited.reserve(std::ssize(m_nodes));
   std::vector<int> unvisited;
   unvisited.reserve(std::ssize(m_nodes));
   for (int i = 0; i < std::ssize(m_nodes); ++i)
      unvisited.push_back(i);

   int current_vertex = source_node_index;

   while(unvisited.empty() == false)
   {
      std::vector<int> current_vertex_neighbors;
      current_vertex_neighbors.reserve(10);
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
         const float distance = tree.get_distance_from_source(current_vertex) + glm::distance(
            m_nodes[current_vertex].m_position,
            m_nodes[neighbor].m_position
         );
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
   for(const int& node_index : m_nodes[node_index_0].m_neighbor_nodes)
   {
      if (node_index == node_index_1)
         return true;
   }
   return false;
}


auto sfn::graph::get_jump_path(const int start_index, const int destination_index) const -> std::optional<jump_path>
{
   const shortest_path_tree tree = this->get_dijkstra(start_index);

   jump_path result;
   int position = destination_index;

   while(position != start_index)
   {
      result.m_stops.push_back(position);
      position = tree.m_entries[position].m_previous_vertex_index;
      if(position == -1)
      {
         return std::nullopt;
      }
   }
   result.m_stops.push_back(start_index);
   std::ranges::reverse(result.m_stops);

   return result;
}


auto sfn::graph::print_path(const jump_path& path) const -> void
{
   printf(std::format(
      "Calculating jump from {} to {} with jump_range of {} LY. Total distance: {:.1f} LY\n",
      m_nodes[path.m_stops.front()].m_name,
      m_nodes[path.m_stops.back()].m_name,
      m_jump_range,
      glm::distance(m_nodes[path.m_stops.front()].m_position, m_nodes[path.m_stops.back()].m_position)
   ).c_str());

   float travelled_distance = 0.0f;
   for(int i=0; i<path.m_stops.size()-1; ++i)
   {
      const int this_stop_system = path.m_stops[i];
      const int next_stop_system = path.m_stops[i+1];
      const float dist = glm::distance(m_nodes[this_stop_system].m_position, m_nodes[next_stop_system].m_position);
      travelled_distance += dist;
      printf(std::format(
         "Jump {}: {} to {}. Distance: {:.1f} LY\n",
         i,
         m_nodes[this_stop_system].m_name,
         m_nodes[next_stop_system].m_name,
         dist
      ).c_str());
   }
   printf(std::format("Travelled {:.1f} LY\n", travelled_distance).c_str());
}


auto sfn::graph::get_closest(const std::string& system) const -> std::vector<int>
{
   const int source_index = this->get_node_index_by_name(system);
   const glm::vec3 source_pos = m_nodes[source_index].m_position;

   std::vector<int> closest;
   closest.reserve(m_nodes.size());

   for(int i=0; i<m_nodes.size(); ++i)
   {
      closest.push_back(i);
   }

   const auto pred = [&](const int a, const int b)
   {
      const glm::vec3 a_pos = m_nodes[a].m_position;
      const glm::vec3 b_pos = m_nodes[b].m_position;
      const float a_dist2 = glm::distance2(source_pos, a_pos);
      const float b_dist2 = glm::distance2(source_pos, b_pos);
      return a_dist2 < b_dist2;
   };
   std::ranges::sort(closest, pred);
   closest.resize(20);

   return closest;
}


auto sfn::get_min_jump_dist(
   const universe& universe,
   const int start_index,
   const int dest_index
) -> float
{
   const float total_dist = glm::distance(
      universe.m_systems[start_index].m_position,
      universe.m_systems[dest_index].m_position
   );

   // Initialize the graph with the total distance. That is guaranteed to work
   // graph minimum_graph(universe, 26.0f);
   graph minimum_graph(universe, total_dist+0.001f);

   float necessary_jumprange = std::numeric_limits<float>::max();
   while (true)
   {
      // Plot a course through that graph
      // If no jump is possible, the previously calculated longest jump is the minimum required range
      const std::optional<jump_path> plot = minimum_graph.get_jump_path(start_index, dest_index);
      if (plot.has_value() == false)
      {
         return necessary_jumprange;
      }

      float longest_jump = 0.0f;
      for (int i = 0; i < plot->m_stops.size() - 1; ++i)
      {
         const float dist = glm::distance(
            universe.m_systems[plot->m_stops[i]].m_position,
            universe.m_systems[plot->m_stops[i + 1]].m_position
         );
         longest_jump = std::max(longest_jump, dist);
      }

      necessary_jumprange = longest_jump;

      // delete longest connections until one relevant was found
      while (true)
      {
         const id longest_connection_id = minimum_graph.m_sorted_connections.back();
         const bool was_relevant = plot->contains_connection(minimum_graph.m_connections.at(longest_connection_id));

         {
            const auto& con = minimum_graph.m_connections.at(longest_connection_id);
            std::erase(minimum_graph.m_nodes[con.m_node_index0].m_neighbor_nodes, con.m_node_index1);
            std::erase(minimum_graph.m_nodes[con.m_node_index1].m_neighbor_nodes, con.m_node_index0);
         }

         minimum_graph.m_connections.erase(longest_connection_id);
         minimum_graph.m_sorted_connections.pop_back();

         if (was_relevant)
            break;
      }
   }
}
