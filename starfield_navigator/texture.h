#pragma once

#include <starfield_navigator/core/canvas.h>

#include <glad/gl.h>
#include <glm/vec3.hpp>

#include "tools.h"

namespace sfn
{

   enum class texture_target { tex2D, tex2D_rect, tex3D };

   enum class interpolation_filter { nearest, linear };
   enum class wrap_setting { clamp_to_edge, clamp_to_border_zero, repeat, mirrored_repeat };
   enum class mipmap_setting{none, yes};

   struct texture_settings{
      interpolation_filter m_interpolation = interpolation_filter::linear;
      wrap_setting m_wrap = wrap_setting::clamp_to_edge;
      mipmap_setting m_mipmap = mipmap_setting::none;
   };

   [[nodiscard]] constexpr auto get_mag_filter(const interpolation_filter interpolation)
   {
      switch (interpolation)
      {
      case interpolation_filter::linear: return GL_LINEAR;
      case interpolation_filter::nearest: return GL_NEAREST;
      }
      // log::error("bad enum");
      std::terminate();
   }


   [[nodiscard]] constexpr auto get_mag_filter(const texture_settings& tex_settings)
   {
      return get_mag_filter(tex_settings.m_interpolation);
   }

   [[nodiscard]] constexpr auto get_min_filter(const texture_settings& tex_settings)
   {
      if (tex_settings.m_interpolation == interpolation_filter::linear && tex_settings.m_mipmap == mipmap_setting::yes)
      {
         return GL_LINEAR_MIPMAP_LINEAR;
      }
      return get_mag_filter(tex_settings);
   }


   // struct u8_pixel_data
   // {
   //    image_metrics m_image_metrics;
   //    std::vector<uint8_t> pixels;
   // };

   struct depth_tag{};

   // struct texture_dimensions{
   //    int m_width = 1, m_height = 1, m_depth = 1;
   // };

   struct texture
   {
      id m_id;
      GLuint m_opengl_id{};
      glm::ivec3 m_image_metrics;
      std::string m_filename;

      // explicit texture(explicit_init, const std::string& filename, const texture_target target, const u8_pixel_data& pixels, const texture_settings& tex_settings);
      explicit texture(
         explicit_init,
         const texture_target target,
         const texture_settings& tex_settings,
         const glm::ivec3& dim,
         const int bpp,
         const std::vector<float>& pixels
      );
      explicit texture(
         explicit_init,
         const texture_target target,
         const texture_settings& tex_settings,
         const glm::ivec3& dim,
         const int bpp,
         const std::vector<uint8_t>& pixels,
         const std::string& name
      );
      explicit texture(depth_tag, const texture_target target, const glm::ivec2& resolution);
      auto bind(const int texture_unit) const -> void;
   };


   struct texture_manager
   {
   private:
      mutable std::vector<texture> m_textures;
      mutable std::vector<std::optional<id>> m_texture_unit_state;

   public:
      explicit texture_manager();

      [[nodiscard]] auto create_3D(const texture_settings& tex_settings, const glm::ivec3& dim, const std::vector<float>& pixels) const -> id;
      [[nodiscard]] auto create_2D(const texture_settings& tex_settings, const glm::ivec3& dim, const int bpp, const std::vector<uint8_t>& pixels, const fs::path& path) const -> id;
      [[nodiscard]] auto create_empty_2D(const texture_settings& tex_settings, const glm::ivec3& dim, const int bpp) const -> id;
      [[nodiscard]] auto create_float_2D(const texture_settings& tex_settings, const glm::ivec3& dim) const -> id;
                    auto create_from_rgb_file(const texture_settings& tex_settings, const fs::path& path, const bool linearize, const bool flip_load) const -> id;
                    auto create_from_rgba_file(const texture_settings& tex_settings, const fs::path& path, const bool linearize, const bool flip_load) const -> id;
      [[nodiscard]] auto create_from_r8_file(const texture_settings& tex_settings, const fs::path& path) const -> id;

      [[nodiscard]] auto get_texture_id(const std::string& filename) const -> id;
      //[[nodiscard]] auto create_new(const image_metrics& image_metrics, const texture_settings& tex_settings) const -> id;
      [[nodiscard]] auto create_new_depth(const glm::ivec2& resolution) const->id;
      [[nodiscard]] auto get_texture_unit(const id tex_id) const -> int;
      [[nodiscard]] auto get_texture_unit(const std::string& name) const -> int;
      [[nodiscard]] auto get_texture_ref(const id tex_id) const -> const texture&;

   private:
      [[nodiscard]] auto get_free_texture_unit() const -> int;
      
   };

}
