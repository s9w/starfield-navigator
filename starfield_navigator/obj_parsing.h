#pragma once

#include "vertex_data.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace sfn
{
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
