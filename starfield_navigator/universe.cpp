#include "universe.h"

#include <format>
#include <execution>

#include "graph.h"

#pragma warning(push, 0)    
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
   for (int i = 0; i < std::ssize(m_systems); ++i)
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


auto sfn::universe::print_info() const -> void
{
   glm::vec3 min = m_systems[0].m_position;
   glm::vec3 max = m_systems[0].m_position;
   for (const system& system : m_systems)
   {
      min = glm::min(min, system.m_position);
      max = glm::max(max, system.m_position);
   }
   printf(std::format("BB min: {:.1f} {:.1f}, {:.1f}\n", min[0], min[1], min[2]).c_str());
   printf(std::format("BB max: {:.1f} {:.1f}, {:.1f}\n", max[0], max[1], max[2]).c_str());
}


auto sfn::universe::get_closest(const int system_index) const -> std::vector<int>
{
   const glm::vec3 source_pos = m_systems[system_index].m_position;

   std::vector<int> closest;
   closest.reserve(m_systems.size());

   for (int i = 0; i < m_systems.size(); ++i)
   {
      closest.push_back(i);
   }

   const auto pred = [&](const int a, const int b)
   {
      const glm::vec3 a_pos = m_systems[a].m_position;
      const glm::vec3 b_pos = m_systems[b].m_position;
      const float a_dist2 = glm::distance2(source_pos, a_pos);
      const float b_dist2 = glm::distance2(source_pos, b_pos);
      return a_dist2 < b_dist2;
   };
   std::ranges::sort(closest, pred);

   return closest;
}


auto sfn::get_graph_from_universe(const universe& universe, const float jump_range) -> graph
{
   graph result;
   result.m_jump_range = jump_range;
   result.m_nodes.reserve(universe.m_systems.size());
   result.m_connections.reserve(universe.m_systems.size() * universe.m_systems.size());
   result.m_sorted_connections.reserve(universe.m_systems.size() * universe.m_systems.size());

   const float jump_range2 = jump_range * jump_range;
   for (int i = 0; i < std::ssize(universe.m_systems); ++i)
   {
      result.m_nodes.push_back(
         node{
            .m_index = i
         }
      );
   }

   for (int i = 0; i < universe.m_systems.size(); ++i)
   {
      for (int j = i + 1; j < universe.m_systems.size(); ++j)
      {
         const float distance2 = glm::distance2(universe.m_systems[i].m_position, universe.m_systems[j].m_position);
         if (distance2 > jump_range2)
            continue;

         const id connection_id = id::create();
         result.m_connections.emplace(
            connection_id,
            connection{
               .m_node_index0 = i,
               .m_node_index1 = j,
               .m_weight = std::sqrt(distance2)
            }
         );
         result.m_sorted_connections.emplace_back(connection_id);

         result.m_nodes[i].m_neighbor_nodes.push_back(j);
         result.m_nodes[j].m_neighbor_nodes.push_back(i);
      }
   }

   const auto pred = [&](const id& a, const id& b) {
      return result.m_connections.at(a).m_weight < result.m_connections.at(b).m_weight;
   };
   std::ranges::sort(result.m_sorted_connections, pred);

   return result;
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
   graph minimum_graph = get_graph_from_universe(universe, total_dist + 0.001f);

   float necessary_jumprange = std::numeric_limits<float>::max();
   while (true)
   {
      // Plot a course through that graph
      // If no jump is possible, the previously calculated longest jump is the minimum required range
      const auto distance_getter = [&](const int i, const int j) {return universe.get_distance(i, j); };
      const std::optional<jump_path> plot = minimum_graph.get_jump_path(start_index, dest_index, distance_getter);
      if (plot.has_value() == false || plot->m_stops.size() == 1)
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
         const auto& con = minimum_graph.m_connections.at(longest_connection_id);
         const bool was_relevant = plot->contains_connection(con);

         {
            // const auto& con = minimum_graph.m_connections.at(longest_connection_id);
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


auto sfn::get_absolute_min_jump_range(const universe& universe) -> float
{
   timer t;
   std::vector<std::pair<int, int>> connections;
   connections.reserve(universe.m_systems.size() * universe.m_systems.size());
   for (int i = 0; i < universe.m_systems.size(); ++i)
   {
      for (int j = i + 1; j < universe.m_systems.size(); ++j)
      {
         connections.emplace_back(i, j);
      }
   }
   std::vector<float> results;
   results.resize(connections.size());

   std::transform(
      std::execution::par_unseq,
      std::cbegin(connections),
      std::cend(connections),
      std::begin(results),
      [&](const std::pair<int, int>& ppp)
      {
         return get_min_jump_dist(universe, ppp.first, ppp.second);
      }
   );

   return *std::ranges::max_element(results);
}


auto sfn::get_closest_distances_for_all(const universe& universe) -> std::vector<float>
{
   std::vector<float> result;
   result.reserve(universe.m_systems.size());

   for (int i = 0; i < std::size(universe.m_systems); ++i)
   {
      float closest = std::numeric_limits<float>::max();
      for (int j = 0; j < std::size(universe.m_systems); ++j)
      {
         if (i == j)
            continue;

         const float dist = glm::distance(
            universe.m_systems[i].m_position,
            universe.m_systems[j].m_position
         );
         closest = std::min(closest, dist);
      }
      result.push_back(closest);
   }

   return result;
}
