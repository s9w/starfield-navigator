#include "obj_parsing.h"

#include <random>
#include <fstream>
#include <map>

#pragma warning(push, 0)
#include <fmt/format.h>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtx/vector_angle.hpp>
#pragma warning(pop)


namespace
{
   using namespace sfn;

   auto get_bb(
      const std::vector<complete_obj_vertex_info>& vertices
   ) -> bb_3D
   {
      glm::vec3 min = vertices.front().m_position;
      glm::vec3 max = vertices.front().m_position;
      for (const complete_obj_vertex_info& vertex : vertices)
      {
         min = glm::min(min, vertex.m_position);
         max = glm::max(max, vertex.m_position);
      }
      return bb_3D{ .m_min = min, .m_max = max };
   }

   


   template<typename vec_type>
   auto get_vec_from_line(const std::string& line) -> vec_type
   {
      const std::vector<std::string> splitted = get_split_string(line, " ");
      vec_type result{};
      for(int i=0; i<vec_type::length(); ++i)
      {
         result[i] = std::stof(splitted[i + 1]);
      }
      return result;
   }

   struct index_triple
   {
      size_t m_vertex_index;
      size_t m_uv_index;
      size_t m_normal_index;
   };

   struct face_indices {
      std::vector<size_t> m_position_indices;
      std::vector<size_t> m_final_normal_indices;
      std::string m_mat_name;

      [[nodiscard]] auto get_position_index_index(const size_t index) const -> std::optional<size_t>
      {
         for(size_t i=0; i< m_position_indices.size(); ++i)
         {
            if(m_position_indices[i] == index)
            {
               return i;
            }
         }
         return std::nullopt;
      }
   };

   auto get_face_index_triple(const std::string& triple_str) -> index_triple
   {
      constexpr auto get_index = [](const std::string& str)
      {
         return static_cast<size_t>(std::stoi(str) - 1);
      };

      const std::vector<std::string> splitted = get_split_string(triple_str, "/");
      return index_triple{
         .m_vertex_index = get_index(splitted[0]),
         .m_uv_index = get_index(splitted[1]),
         .m_normal_index = get_index(splitted[2])
      };
   }


   [[nodiscard]] auto get_face_indices(
      const std::string& line,
      std::vector<glm::vec3>& normals,
      const std::string& mat_name
   ) -> face_indices
   {
      const std::vector<std::string> splitted = get_split_string(line, " ");
      std::vector<size_t> positions;
      std::vector<size_t> uvs;
      glm::vec3 normal{};
      for(int i=1; i< splitted.size(); ++i)
      {
         const auto triple = get_face_index_triple(splitted[i]);
         positions.push_back(triple.m_vertex_index);
         uvs.push_back(triple.m_uv_index);
         normal += normals[triple.m_normal_index];
      }
      normal = glm::normalize(normal);
      normals.push_back(normal);
      const size_t normal_index = normals.size() - 1;
      return face_indices{
         .m_position_indices = positions,
         .m_final_normal_indices = std::vector<size_t>(positions.size(), normal_index),
         .m_mat_name = mat_name
      };
   }


   [[nodiscard]] auto get_quad_center(
      const std::vector<glm::vec3>& positions,
      const face_indices& indices
   ) -> glm::vec3
   {
      sfn_assert(indices.m_position_indices.size() == 4);
      glm::vec3 position{};
      for(const size_t& i : indices.m_position_indices)
      {
         position += positions[i];
      }
      return position /= 4.0f;
   }
   
} // namespace {}




[[nodiscard]] auto sfn::get_complete_obj_info(
   const fs::path& path,
   const float max_smoothing_angle
) -> complete_obj
{
   sfn_assert(fs::exists(path), "obj file doesn't exist");

   std::vector<glm::vec3> positions;
   std::vector<glm::vec3> normals;
   std::vector<face_indices> indexed_faces;
   

   std::ifstream infile(path.string());
   std::string line;

   // collect indixed data + indices
   while (std::getline(infile, line))
   {
      if (line.starts_with("v "))
      {
         positions.push_back(c4d_convert(0.01f * get_vec_from_line<glm::vec3>(line)));
         positions.back()[1] *= -1.0f; // TODO
      }
      else if (line.starts_with("vn "))
      {
         normals.push_back(c4d_convert(get_vec_from_line<glm::vec3>(line)));
      }
   }

   complete_obj result;

   std::string current_mat;
   std::string current_obj;
   infile = std::ifstream(path.string());
   while (std::getline(infile, line))
   {
      if (line.starts_with("f "))
      {
         const face_indices face_indices = get_face_indices(line, normals, current_mat);
         indexed_faces.push_back(face_indices);
      }
      else if (line.starts_with("o "))
      {
         current_obj = get_split_string(line, " ")[1];
      }
      else if (line.starts_with("usemtl "))
      {
         current_mat = get_split_string(line, " ")[1];
      }
   }

   // post-process indexed data (normal smoothing)
   if (max_smoothing_angle > 0.0f)
   {
      for (size_t position_index = 0; position_index < positions.size(); ++position_index)
      {
         // calculate smoothed normal
         glm::vec3 smoothed_normal{};
         std::vector<std::reference_wrapper<face_indices>> relevant_faces;
         for (face_indices& indexed_face : indexed_faces)
         {
            const std::optional<size_t> position_index_index = indexed_face.get_position_index_index(position_index);

            if (position_index_index.has_value())
            {
               smoothed_normal += normals[indexed_face.m_final_normal_indices[position_index_index.value()]];
               relevant_faces.push_back(indexed_face);
            }
         }
         if (glm::isNull(smoothed_normal, 0.001f))
         {
            continue;
         }

         smoothed_normal = glm::normalize(smoothed_normal);
         normals.push_back(smoothed_normal);
         const size_t smoothed_normal_index = normals.size() - 1;

         // Apply it to faces that satisfy angle predicate
         for (std::reference_wrapper<face_indices>& indexed_face : relevant_faces)
         {
            const std::optional<size_t> position_index_index = indexed_face.get().get_position_index_index(position_index);
            const glm::vec3& normal = normals[indexed_face.get().m_final_normal_indices[position_index_index.value()]];
            const float angle = glm::angle(smoothed_normal, normal);
            if (angle <= max_smoothing_angle)
            {
               indexed_face.get().m_final_normal_indices[position_index_index.value()] = smoothed_normal_index;
            }
         }
      }
   }

   // build non-indexed data
   
   auto build_complete_vertex_info = [&](const face_indices& face, const int i)
   {
      return complete_obj_vertex_info{
         .m_position = positions[face.m_position_indices[i]],
         .m_normal = normals[face.m_final_normal_indices[i]]
      };
   };
   for (const face_indices& face : indexed_faces)
   {
      if (face.m_position_indices.size() == 3)
      {
         result.m_vertices.push_back(build_complete_vertex_info(face, 0));
         result.m_vertices.push_back(build_complete_vertex_info(face, 1));
         result.m_vertices.push_back(build_complete_vertex_info(face, 2));
      }
      else if (face.m_position_indices.size() == 4)
      {
         result.m_vertices.push_back(build_complete_vertex_info(face, 0));
         result.m_vertices.push_back(build_complete_vertex_info(face, 1));
         result.m_vertices.push_back(build_complete_vertex_info(face, 2));

         result.m_vertices.push_back(build_complete_vertex_info(face, 0));
         result.m_vertices.push_back(build_complete_vertex_info(face, 2));
         result.m_vertices.push_back(build_complete_vertex_info(face, 3));
      }
      else
      {
         std::terminate();
      }
   }

   result.m_vertex_bb = get_bb(result.m_vertices);
   {
      const glm::vec3 size = result.m_vertex_bb.get_size();
      fmt::print(
         "loaded {}. dimensions: {:.2f}, {:.2f}, {:.2f}",
         path.string(), size[0], size[1], size[2]
      );
   }

   return result;
}


auto sfn::get_position_vertex_data(
   const complete_obj& obj_info
) -> std::vector<position_vertex_data>
{
   std::vector<position_vertex_data> result;
   for (const complete_obj_vertex_info& vi : obj_info.m_vertices)
   {
      result.emplace_back(vi.m_position);
   }
   return result;
}
