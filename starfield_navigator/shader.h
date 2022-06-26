#pragma once

// #include "imgui_tools.h"

#include "tools.h"
#include "type_support.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;


namespace sfn
{
   enum class shader_type{vertex, fragment, compute};

   struct binding_point_man
   {
      std::vector<id> m_ids;

      explicit constexpr binding_point_man() = default;
      auto add(const id id) -> void;
      [[nodiscard]] auto get_point(const id id) const -> int;
   };

   struct shader_io
   {
      std::string m_name;
      data_type m_data_type;
      int m_location;
   };

   struct shader_ios
   {
      std::vector<shader_io> m_vertex_inputs;
      std::vector<shader_io> m_uniforms;
   };

   auto get_file_str(const fs::path& path) -> std::string;

   struct shader
   {
      GLuint m_opengl_id{};  

      explicit shader(const fs::path& filename);
      ~shader();
   };

   struct shader_program
   {
      GLuint m_opengl_id{};
      shader_ios m_ios;
      bool m_is_compute = false;
      glm::ivec3 m_group_size{};
      std::string m_base_name;

      explicit shader_program(const fs::path& base_name);
      auto use() const -> void;
      [[nodiscard]] auto get_uniform(const std::string& name) const -> const shader_io&;
      [[nodiscard]] auto get_vertex_inputs() const -> const std::vector<shader_io>&;
      auto dispatch_compute_1D(const int item_count) const -> void;

      template<typename T>
      auto set_uniform(const std::string& name, const T& value) const -> void;
   };
}

