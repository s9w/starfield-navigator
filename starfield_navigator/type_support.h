#pragma once

#include <cstdint>
#include <vector>

#include <glad/gl.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "logging.h"
#include "opengl_stringify.h"


namespace sfn
{

   // Typical shader data types (float, fvec3 etc)
   // This type is used to runtime-represent such types. There's
   // - T -> data_type (get_data_type_v)
   // - data_type -> ogl type (get_ogl_data_type)
   // - ogl type -> data_type (get_data_type_from_ogl)
   enum class data_type { fvec2, fvec3, fvec4, fmat4, f, i, ui };

   // glVertexArrayAttribFormat() needs GL_FLOAT_VEC3 as GL_FLOAT and component count = 3
   struct attrib_format_breakdown {
      int m_count;
      GLenum m_type;
   };
   [[nodiscard]] constexpr auto get_attrib_format_breakdown(const data_type type) -> attrib_format_breakdown;


   template<typename T>
   struct get_data_type_impl {};
   template<> struct get_data_type_impl<uint32_t>   { constexpr static inline data_type value = data_type::ui;    };
   template<> struct get_data_type_impl<int>        { constexpr static inline data_type value = data_type::i;     };
   template<> struct get_data_type_impl<float>      { constexpr static inline data_type value = data_type::f;     };
   template<> struct get_data_type_impl<glm::vec2>  { constexpr static inline data_type value = data_type::fvec2; };
   template<> struct get_data_type_impl<glm::vec3>  { constexpr static inline data_type value = data_type::fvec3; };
   template<> struct get_data_type_impl<std::vector<glm::vec3>>  { constexpr static inline data_type value = data_type::fvec3; };
   template<> struct get_data_type_impl<glm::vec4>  { constexpr static inline data_type value = data_type::fvec4; };
   template<> struct get_data_type_impl<glm::mat4>  { constexpr static inline data_type value = data_type::fmat4; };
   template<typename T>
   constexpr inline data_type get_data_type_v = get_data_type_impl<T>::value;

   [[nodiscard]] constexpr auto get_data_type_from_ogl(const GLint ogl_type) -> data_type;
   [[nodiscard]] constexpr auto get_ogl_data_type(const data_type type) -> GLenum;
   [[nodiscard]] constexpr auto get_data_type_size(const data_type type) -> int;

}


constexpr auto sfn::get_ogl_data_type(const data_type type) -> GLenum
{
   switch (type)
   {
   case data_type::f:     return GL_FLOAT;
   case data_type::fvec2: return GL_FLOAT_VEC2;
   case data_type::fvec3: return GL_FLOAT_VEC3;
   case data_type::fvec4: return GL_FLOAT_VEC4;
   case data_type::fmat4: return GL_FLOAT_MAT4;
   case data_type::i:     return GL_INT;
   case data_type::ui:    return GL_UNSIGNED_INT;
   }
   log::error("enum not yet supported");
   std::terminate();
}


constexpr auto sfn::get_data_type_from_ogl(const GLint ogl_type) -> data_type
{
   switch (ogl_type)
   {
   case GL_FLOAT_VEC2: return data_type::fvec2;
   case GL_FLOAT_VEC3: return data_type::fvec3;
   case GL_FLOAT_VEC4: return data_type::fvec4;
   case GL_FLOAT_MAT4: return data_type::fmat4;
   case GL_FLOAT:      return data_type::f;
   case GL_INT:        return data_type::i;
   case GL_UNSIGNED_INT:return data_type::ui;
   }
   log::error(fmt::format("data type not yet supported: {}", opengl_enum_to_str(ogl_type)));
   std::terminate();
}


constexpr auto sfn::get_attrib_format_breakdown(const data_type type) -> attrib_format_breakdown
{
   switch (type)
   {
   case data_type::f:     return attrib_format_breakdown{ .m_count = 1, .m_type = GL_FLOAT };
   case data_type::i:     return attrib_format_breakdown{ .m_count = 1, .m_type = GL_INT   };
   case data_type::ui:    return attrib_format_breakdown{ .m_count = 1, .m_type = GL_UNSIGNED_INT };
   case data_type::fvec2: return attrib_format_breakdown{ .m_count = 2, .m_type = GL_FLOAT };
   case data_type::fvec3: return attrib_format_breakdown{ .m_count = 3, .m_type = GL_FLOAT };
   case data_type::fvec4: return attrib_format_breakdown{ .m_count = 4, .m_type = GL_FLOAT };
   }
   log::error("data type not yet supported");
   std::terminate();
}


constexpr auto sfn::get_data_type_size(const data_type type) -> int
{
   switch (type)
   {
   case data_type::f:     return sizeof(float);
   case data_type::i:     return sizeof(int);
   case data_type::ui:    return sizeof(uint32_t);
   case data_type::fvec2: return sizeof(float) * 2;
   case data_type::fvec3: return sizeof(float) * 3;
   case data_type::fvec4: return sizeof(float) * 4;
   case data_type::fmat4: return sizeof(float) * 16;
   }
   log::error("data type not yet supported");
   std::terminate();
}