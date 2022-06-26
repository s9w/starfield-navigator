// #include "pch.h"
#include "shader.h"

#include <fstream>
#include <sstream>

#pragma warning(push, 0)    
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)


namespace
{
   using namespace sfn;

   constexpr auto get_opengl_shader_type(const shader_type& type) -> int
   {
      switch (type)
      {
         case shader_type::fragment: return GL_FRAGMENT_SHADER;
         case shader_type::vertex: return GL_VERTEX_SHADER;
         case shader_type::compute: return GL_COMPUTE_SHADER;
      }
      // log::error("bad enum");
      std::terminate();
   }


   [[nodiscard]] auto get_type_from_filename(const std::string& filename) -> shader_type
   {
      if(filename.ends_with(".frag"))
      {
         return shader_type::fragment;
      }
      if (filename.ends_with(".vert"))
      {
         return shader_type::vertex;
      }
      if (filename.ends_with(".compute"))
      {
         return shader_type::compute;
      }
      // log::error("unknown shader extension: {}", filename);
      std::terminate();
   }


   [[nodiscard]] auto get_shader_path(const std::string& filename) -> fs::path
   {
      return fs::path(fs::path{ "shaders" } / filename);
   }


   struct shader_query_result
   {
      std::string name;
      int type;
      int location;
   };


   auto io_from_query_result(const shader_query_result& query_result) -> shader_io
   {
      constexpr auto get_sanitized_type = [](const int type)
      {
         // Samplers get accessed via their texture unit
         if (type == GL_SAMPLER_2D || type == GL_SAMPLER_3D)
            return GL_INT;
         return type;
      };
      return shader_io{
         .m_name = query_result.name,
         .m_data_type = get_data_type_from_ogl(get_sanitized_type(query_result.type)),
         .m_location = query_result.location
      };
   }


   auto get_program_resource(
      const GLuint program,
      const GLenum interface,
      const int i,
      const GLenum property
   ) -> int
   {
      int result{};
      glGetProgramResourceiv(program, interface, i, 1, &property, 1, nullptr, &result);
      return result;
   }


   auto query_program_resource_name(
      const GLuint program,
      const GLenum interface,
      const int i
   ) -> std::string
   {
      const int name_buffer_size = get_program_resource(program, interface, i, GL_NAME_LENGTH);

      // "The name length includes a terminating null character"
      // "The name string assigned to the active resource identified by index is returned as a null-terminated string"
      std::string name(name_buffer_size, ' ');
      glGetProgramResourceName(program, interface, i, name_buffer_size, NULL, name.data());
      name.resize(name.size() - 1);
      return name;
   }


   auto get_program_resources(
      const GLuint program,
      const GLenum interface
   ) -> std::vector<shader_query_result>
   {
      GLint count{};
      glGetProgramInterfaceiv(program, interface, GL_ACTIVE_RESOURCES, &count);

      std::vector<shader_query_result> result;
      result.reserve(count);
      for(int i=0; i<count; ++i)
      {
         result.push_back(
            shader_query_result{
               .name = query_program_resource_name(program, interface, i),
               .type = get_program_resource(program, interface, i, GL_TYPE),
               .location = get_program_resource(program, interface, i, GL_LOCATION)
            }
         );
      }
      return result;
   }

   template<typename T>
   [[nodiscard]] auto get_thing_by_name(const std::vector<T>& vec, const std::string& name) -> const T&
   {
      const auto pred = [&](const T& obj)
      {
         return obj.m_name == name;
      };
      return find_obj(vec, pred, std::format("Thing \"{}\" couldn't be found", name));
   }


   auto assert_binary_io_formats() -> void
   {
      GLint formats = 0;
      glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
      if (formats == 0) {
         // log::error("Driver does not support any binary formats.");
         std::terminate();
      }
   }


   auto replace_all(
      std::string& inout,
      std::string_view what,
      std::string_view with
   ) -> void
   {
      std::size_t count{};
      for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
         inout.replace(pos, what.length(), with.data(), with.length());
      }
   }


   const std::vector<std::string> shader_variable_lookup{
      "shadow_amount", "0.3",
      "ubo_code", get_file_str(fs::path("shaders/ubo.txt"))
   };

   [[nodiscard]] auto get_shader_replaced(
      const std::string& file_content
   ) -> std::string
   {
      std::string result = file_content;
      for(int i=0; i< shader_variable_lookup.size(); i+=2)
      {
         const std::string& key = shader_variable_lookup[i];
         const std::string& value = shader_variable_lookup[i+1];
         replace_all(result, std::format("{{{{{}}}}}", key), value);
      }
      return result;
   }


   auto get_shader_ios(const GLuint program_id) -> shader_ios
   {
      shader_ios result;

      for (const shader_query_result& results : get_program_resources(program_id, GL_PROGRAM_INPUT))
      {
         if (results.type == GL_UNSIGNED_INT_VEC3)
            continue;
         result.m_vertex_inputs.push_back(io_from_query_result(results));
      }

      for (const shader_query_result& results : get_program_resources(program_id, GL_UNIFORM))
      {
         // uniform block
         if (results.location == -1)
         {
            continue;
         }

         result.m_uniforms.push_back(io_from_query_result(results));
      }

      return result;
   }


} // namespace {}

sfn::shader::shader(
   const fs::path& filename
)
{
   const shader_type type = get_type_from_filename(filename.string());
   const fs::path path = get_shader_path(filename.string());
   if(exists(path) == false)
   {
      // log::error("path doesn't exist: {}", path.string());
      std::terminate();
   }

   m_opengl_id = glCreateShader(get_opengl_shader_type(type));

   constexpr int count = 1;
   const std::string source = get_shader_replaced(get_file_str(path));
   const GLchar* char_ptr = source.data();
   glShaderSource(m_opengl_id, count, &char_ptr, nullptr);
   glCompileShader(m_opengl_id);

   int success;
   glGetShaderiv(m_opengl_id, GL_COMPILE_STATUS, &success);
   if(success != GL_TRUE)
   {
      int msg_length = 0;
      glGetShaderiv(m_opengl_id, GL_INFO_LOG_LENGTH, &msg_length);

      std::string msg;
      msg.resize(msg_length);
      glGetShaderInfoLog(m_opengl_id, msg_length, NULL, msg.data());
      // log::error("shader compilation error: {}", msg);
   }

   //assert_binary_io_formats();
}


shader::~shader()
{
   glDeleteShader(m_opengl_id);
}


shader_program::shader_program(
   const fs::path& base_name
)
   : m_base_name(base_name.string())
{
   int success;
   if(exists(get_shader_path(base_name.string() + ".frag")))
   {
      const shader frag(base_name.string() + ".frag");
      const shader vert(base_name.string() + ".vert");

      m_opengl_id = glCreateProgram();
      glAttachShader(m_opengl_id, frag.m_opengl_id);
      glAttachShader(m_opengl_id, vert.m_opengl_id);
      glLinkProgram(m_opengl_id);
      glGetProgramiv(m_opengl_id, GL_LINK_STATUS, &success);
   }
   else if (exists(get_shader_path(base_name.string() + ".compute")))
   {
      const shader compute(base_name.string() + ".compute");

      m_opengl_id = glCreateProgram();
      glAttachShader(m_opengl_id, compute.m_opengl_id);
      glLinkProgram(m_opengl_id);
      glGetProgramiv(m_opengl_id, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(m_group_size));
      m_is_compute = true;
      glGetProgramiv(m_opengl_id, GL_LINK_STATUS, &success);
   }
   else
   {
      std::terminate();
   }

   if (success != GL_TRUE)
   {
      int msg_length = 0;
      glGetProgramiv(m_opengl_id, GL_INFO_LOG_LENGTH, &msg_length);

      std::string msg;
      msg.resize(msg_length);
      glGetProgramInfoLog(m_opengl_id, msg_length, NULL, msg.data());
      // log::error("shader program link error: {}", msg);
      std::terminate();
   }

   m_ios = get_shader_ios(m_opengl_id);
}


auto shader_program::use() const -> void
{
   glUseProgram(m_opengl_id);
}


auto shader_program::get_uniform(const std::string& name) const -> const shader_io&
{
   return get_thing_by_name(m_ios.m_uniforms, name);
}


auto shader_program::get_vertex_inputs() const -> const std::vector<shader_io>&
{
   return this->m_ios.m_vertex_inputs;
}


auto shader_program::dispatch_compute_1D(const int item_count) const -> void
{
   sfn_assert(m_is_compute == true);
   sfn_assert(m_group_size[1] == 1);
   sfn_assert(m_group_size[2] == 1);
   const int num_groups_x = item_count / m_group_size[0];
   glDispatchCompute(num_groups_x, 1, 1);
}


auto binding_point_man::add(const id id) -> void
{
   m_ids.push_back(id);
}

auto binding_point_man::get_point(const id id) const -> int
{
   for(int i=0; i<m_ids.size(); ++i)
   {
      if (id == m_ids[i])
         return i;
   }
   // log::error( "binding point for id not found");
   std::terminate();
}


auto sfn::get_file_str(const fs::path& path) -> std::string
{
   const std::ifstream t(path.string());
   std::stringstream buffer;
   buffer << t.rdbuf();
   return buffer.str();
}


template<typename T>
auto sfn::shader_program::set_uniform(
   const std::string& name,
   const T& value
) const -> void
{
   // TODO error if shader is not active because that don't work.
   const shader_io& uni = get_uniform(name);
   if (get_data_type_v<T> != uni.m_data_type)
   {
      // log::error("types don't match in set_uniform()");
      std::terminate();
   }

   if constexpr (std::same_as<T, float>)
   {
      glUniform1f(uni.m_location, value);
   }
   else if constexpr (std::same_as<T, glm::vec2>)
   {
      glUniform2fv(uni.m_location, 1, &value[0]);
   }
   else if constexpr (std::same_as<T, glm::vec3>)
   {
      glUniform3fv(uni.m_location, 1, &value[0]);
   }
   else if constexpr (std::same_as<T, glm::vec4>)
   {
      glUniform4fv(uni.m_location, 1, &value[0]);
   }
   else if constexpr (std::same_as<T, int>)
   {
      glUniform1i(uni.m_location, value);
   }
   else if constexpr (std::same_as<T, glm::mat4>)
   {
      glUniformMatrix4fv(uni.m_location, 1, GL_FALSE, glm::value_ptr(value));
   }
   else
   {
      std::terminate();
   }
}

template auto sfn::shader_program::set_uniform(const std::string&, const float&) const -> void;
template auto sfn::shader_program::set_uniform(const std::string&, const int&) const -> void;
template auto sfn::shader_program::set_uniform(const std::string&, const glm::vec2&) const -> void;
template auto sfn::shader_program::set_uniform(const std::string&, const glm::vec3&) const -> void;
template auto sfn::shader_program::set_uniform(const std::string&, const glm::vec4&) const -> void;
template auto sfn::shader_program::set_uniform(const std::string&, const glm::mat4&) const -> void;
