#pragma once

#include "vertex_data.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace sfn
{

   struct bb_3D
   {
      glm::vec3 m_min{};
      glm::vec3 m_max{};

      auto get_size() const -> glm::vec3
      {
         return m_max - m_min;
      }
   };

   struct complete_obj
   {
      std::vector<complete_obj_vertex_info> m_vertices;
      bb_3D m_vertex_bb;
   };

   [[nodiscard]] auto get_complete_obj_info(
      const fs::path& path,
      const float max_smoothing_angle
   ) -> complete_obj;

   [[nodiscard]] auto get_position_vertex_data(const complete_obj& obj_info) -> std::vector<position_vertex_data>;
}


// template<typename T>
// auto sfn::get_T_vertex_data(const complete_obj& obj_info) -> std::vector<T>
// {
//    std::vector<T> result;
//    for (const complete_obj_vertex_info& vi : obj_info.m_vertices)
//    {
//       result.emplace_back(vi);
//    }
//    return result;
// }
