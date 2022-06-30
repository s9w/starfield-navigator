#pragma once

#include "buffer.h"


namespace sfn
{
   // struct complete_obj_vertex_info {
   //    glm::vec3 m_position;
   //    glm::vec3 m_normal;
   //    glm::vec2 m_uv;
   //    glm::vec4 m_color;
   //    friend constexpr auto operator<=>(const complete_obj_vertex_info&, const complete_obj_vertex_info&) = default;
   // };


   // Advantage of a visitor is that the vertex types itself don't need to have templated functions
   struct vertex_data_visitor
   {
      std::vector<vbo_class_member> m_vbo_classes;
      template<typename T>
      auto visit(const char* vertex_attrib_name) -> void
      {
         // constexpr bool default_normalized = false; // TODO
         m_vbo_classes.emplace_back(get_data_type_v<T>, vertex_attrib_name, false);
      }
   };


   template<typename T>
   auto get_vbo_class(const int count) -> vbo_class
   {
      vertex_data_visitor visitor;
      T::for_each(visitor);

      return vbo_class(visitor.m_vbo_classes, count);
   }

   template<typename T>
   auto get_soa_vbo_segment() -> vbo_segment
   {
      return vbo_segment({ get_vbo_class<T>(T::static_count)});
   }

   template<typename T>
   auto get_soa_vbo_segment(const int max_count) -> vbo_segment
   {
      return vbo_segment({ get_vbo_class<T>(max_count) });
   }

   template<typename T>
   auto get_soa_vbo_segment(const std::vector<T>& vec) -> vbo_segment
   {
      const int count = static_cast<int>(vec.size());
      return vbo_segment({ get_vbo_class<T>(count) });
   }





   struct position_vertex_data
   {
      glm::vec3 m_position;
      static auto for_each(vertex_data_visitor& visitor) -> void;
   };

   

}
