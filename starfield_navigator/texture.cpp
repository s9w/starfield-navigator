#include "texture.h"

#include <execution>

#include <stb_image.h>


namespace
{
   using namespace sfn;

   constexpr texture_target default_texture_target = texture_target::tex2D; // TODO


   [[nodiscard]] constexpr auto get_gl_wrapping(const wrap_setting wrapping)
   {
      switch (wrapping)
      {
      case wrap_setting::clamp_to_edge: return GL_CLAMP_TO_EDGE;
      case wrap_setting::clamp_to_border_zero: return GL_CLAMP_TO_BORDER;
      case wrap_setting::repeat: return  GL_REPEAT;
      case wrap_setting::mirrored_repeat: return GL_MIRRORED_REPEAT;
      }
      // log::error("bad enum");
      std::terminate();
   }

   [[nodiscard]] constexpr auto get_gl_texture_target(const texture_target target)
   {
      switch (target)
      {
      case texture_target::tex2D: return GL_TEXTURE_2D;
      case texture_target::tex2D_rect: return GL_TEXTURE_RECTANGLE;
      case texture_target::tex3D: return GL_TEXTURE_3D;
      }
      // log::error("bad enum");
      std::terminate();
   }


   auto get_image_path(const std::string& filename) -> fs::path
   {
      const fs::path path0 = fs::path("assets") / filename;
      if (exists(path0))
      {
         return path0;
      }

      const fs::path path1 = fs::path("../c4d") / filename;
      if (exists(path1))
      {
         return path1;
      }

      
      // log::error("filename couldn't be found");
      std::terminate();
   }

   enum class init_mode { standard, is_float, depth };

   [[nodiscard]] constexpr auto get_internalformat_from_bpp(const int bpp, const init_mode mode) -> GLenum
   {
      if(mode == init_mode::is_float)
      {
         switch (bpp)
         {
         case 1: return GL_R16F;
         }
      }
      if (mode == init_mode::depth)
      {
         return GL_DEPTH_COMPONENT16;
         //return GL_DEPTH_COMPONENT24;
      }
      else
      {
         switch (bpp)
         {
         case 1: return GL_R8;
         case 2: return GL_RG8;
         case 3: return GL_RGB8;
            // case 4: return GL_SRGB8_ALPHA8;
         case 4: return GL_RGBA8;
         }

      }
      // log::error("invalid bpp");
      std::terminate();
   }

   [[nodiscard]] constexpr auto get_format_from_bpp(const int bpp, const init_mode mode) -> GLenum
   {
      if (mode == init_mode::depth)
      {
         return GL_DEPTH_COMPONENT;
      }
      // linear types because pngs are linearized on load!
      switch (bpp)
      {
      case 1: return GL_RED;
      case 2: return GL_RG;
      case 3: return GL_RGB;
      case 4: return GL_RGBA;
      }
      // log::error("invalid bpp");
      std::terminate();
   }

   [[nodiscard]] auto get_texture_unit_count() -> int
   {
      int texture_unit_count = -1;
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_unit_count);
      return texture_unit_count;
      // 32 on both Intel iGPU and nvidia gpu
   }


   // auto get_empty_pixel_data(const image_metrics& metrics) -> u8_pixel_data
   // {
   //    ZoneScoped;
   //    u8_pixel_data result;
   //    result.m_image_metrics = metrics;
   //    result.pixels.resize(metrics.get_byte_count());
   //    return result;
   // }

   

   auto init_texture(
      const texture_target target,
      const texture_settings& tex_settings,
      const glm::ivec3& dim,
      const int bpp,
      const init_mode init_mode,
      const void* pixel_ptr
   ) -> GLuint
   {
      GLuint m_opengl_id;
      glCreateTextures(get_gl_texture_target(target), 1, &m_opengl_id);

      // TODO make this configurable
      glTextureParameteri(m_opengl_id, GL_TEXTURE_WRAP_S, get_gl_wrapping(tex_settings.m_wrap));
      glTextureParameteri(m_opengl_id, GL_TEXTURE_WRAP_T, get_gl_wrapping(tex_settings.m_wrap));
      if(dim[2] > 1)
         glTextureParameteri(m_opengl_id, GL_TEXTURE_WRAP_R, get_gl_wrapping(tex_settings.m_wrap));
      glTextureParameteri(m_opengl_id, GL_TEXTURE_MIN_FILTER, get_min_filter(tex_settings));
      glTextureParameteri(m_opengl_id, GL_TEXTURE_MAG_FILTER, get_mag_filter(tex_settings));
      // glTextureParameteri(m_opengl_id, GL_TEXTURE_MAX_ANISOTROPY, 16); // TODO

      if (tex_settings.m_wrap == wrap_setting::clamp_to_border_zero)
      {
         constexpr float border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f }; // TODO make this configurable
         glTextureParameterfv(m_opengl_id, GL_TEXTURE_BORDER_COLOR, border_color);
      }

      const int mip_levels = (tex_settings.m_mipmap == mipmap_setting::yes) ? 4 : 1;
      if(target == texture_target::tex3D)
         glTextureStorage3D(m_opengl_id, mip_levels, get_internalformat_from_bpp(bpp, init_mode), dim[0], dim[1], dim[2]);
      else
         glTextureStorage2D(m_opengl_id, mip_levels, get_internalformat_from_bpp(bpp, init_mode), dim[0], dim[1]);

      const GLenum type = (init_mode == init_mode::is_float) ? GL_FLOAT : GL_UNSIGNED_BYTE;
      if (target == texture_target::tex3D)
      {
         glTextureSubImage3D(
            m_opengl_id,
            0,
            0, 0, 0,
            dim[0], dim[1], dim[2],
            get_format_from_bpp(bpp, init_mode), type, pixel_ptr
         );
      }
      else
      {
         glTextureSubImage2D(
            m_opengl_id,
            0,
            0, 0,
            dim[0], dim[1],
            get_format_from_bpp(bpp, init_mode), type, pixel_ptr
         );
      }

      if (tex_settings.m_mipmap == mipmap_setting::yes)
      {
         glGenerateTextureMipmap(m_opengl_id);
      }
      return m_opengl_id;
   }

 } // namespace {}


texture::texture(
   explicit_init,
   const texture_target target,
   const texture_settings& tex_settings,
   const glm::ivec3& dim,
   const int bpp,
   const std::vector<float>& pixels
)
   : m_id(id::create())
   , m_image_metrics{ dim }
{
   m_opengl_id = init_texture(target, tex_settings, m_image_metrics, bpp, init_mode::is_float, pixels.data());
}


texture::texture(
   explicit_init,
   const texture_target target,
   const texture_settings& tex_settings,
   const glm::ivec3& dim,
   const int bpp,
   const std::vector<uint8_t>& pixels,
   const std::string& name
)
   : m_id(id::create())
   , m_image_metrics{ dim }
   , m_filename(name)
{
   m_opengl_id = init_texture(target, tex_settings, m_image_metrics, bpp, init_mode::standard, pixels.data());
}


texture::texture(
   depth_tag,
   const texture_target target,
   const glm::ivec2& resolution
)
   : m_id(id::create())
   , m_image_metrics{ resolution, 1 }
{
   constexpr texture_settings depth_tex_settings{
      .m_interpolation = interpolation_filter::linear,
      .m_wrap = wrap_setting::clamp_to_edge,
      .m_mipmap = mipmap_setting::none
   };
   std::vector<uint8_t> pixels(resolution[0] * resolution[1]);
   m_opengl_id = init_texture(target, depth_tex_settings, m_image_metrics, 1, init_mode::depth, pixels.data());
}


auto texture::bind(const int texture_unit) const -> void
{
   glBindTextureUnit(texture_unit, m_opengl_id);
}


texture_manager::texture_manager()
   : m_texture_unit_state(get_texture_unit_count(), std::nullopt)
{

}


auto texture_manager::get_texture_ref(const id tex_id) const -> const texture&
{
   const auto pred = [&](const texture& tex) {return tex.m_id == tex_id; };
   const auto it = std::ranges::find_if(m_textures, pred);
   if(it == std::end(m_textures))
   {
      // log::error("id not found");
      std::terminate();
   }
   return *it;
}



auto texture_manager::create_3D(
   const texture_settings& tex_settings,
   const glm::ivec3& dim,
   const std::vector<float>& pixels
) const -> id
{
   constexpr int bpp = 1;
   const texture& ref = m_textures.emplace_back(explicit_init{}, texture_target::tex3D, tex_settings, dim, bpp, pixels);
   const int texture_unit = get_free_texture_unit();
   ref.bind(texture_unit);
   m_texture_unit_state[texture_unit] = ref.m_id;
   return ref.m_id;
}


auto texture_manager::create_2D(
   const texture_settings& tex_settings,
   const glm::ivec3& dim,
   const int bpp,
   const std::vector<uint8_t>& pixels,
   const fs::path& path
) const -> id
{
   sfn_assert(dim[2] == 1);
   const texture& ref = m_textures.emplace_back(explicit_init{}, texture_target::tex2D, tex_settings, dim, bpp, pixels, path.string());
   const int texture_unit = get_free_texture_unit();
   ref.bind(texture_unit);
   m_texture_unit_state[texture_unit] = ref.m_id;
   return ref.m_id;
}

auto sfn::texture_manager::create_empty_2D(const texture_settings& tex_settings, const glm::ivec3& dim, const int bpp) const -> id
{
   std::vector<uint8_t> pixels(dim[0] * dim[1] * bpp);

   return create_2D(tex_settings, dim, bpp, pixels, "");
}


auto sfn::texture_manager::create_float_2D(const texture_settings& tex_settings, const glm::ivec3& dim) const -> id
{
   constexpr int bpp = 1;
   sfn_assert(dim[2] == 1);
   std::vector<float> pixels(dim[0] * dim[1]);
   const texture& ref = m_textures.emplace_back(explicit_init{}, texture_target::tex2D, tex_settings, dim, bpp, pixels);
   const int texture_unit = get_free_texture_unit();
   ref.bind(texture_unit);
   m_texture_unit_state[texture_unit] = ref.m_id;
   return ref.m_id;
}


auto texture_manager::create_from_rgb_file(
   const texture_settings& tex_settings,
   const fs::path& path,
   const bool linearize,
   const bool flip_load
) const -> id
{
   glm::ivec3 dim{ 1, 1, 1 };
   int bpp = 3;
   unsigned char* data = nullptr;
   {
      stbi_set_flip_vertically_on_load(flip_load ? 1 : 0);
      data = stbi_load(path.string().c_str(), &dim[0], &dim[1], &bpp, 0);
   }
   const int byte_size = dim[0] * dim[1] * bpp;
   std::vector<uint8_t> pixels(byte_size);
   {
      std::memcpy(pixels.data(), data, byte_size);
   }
   if (linearize)
   {
      std::transform(
         std::execution::par_unseq,
         std::cbegin(pixels),
         std::cend(pixels),
         std::begin(pixels),
         [](const uint8_t rgb){
            return srgb_to_linear_ui8(rgb);
         }
      );
   }

   sfn_assert(data != nullptr);
   sfn_assert(bpp == 3);
   return create_2D(tex_settings, dim, bpp, pixels, path);
}


auto texture_manager::create_from_rgba_file(
   const texture_settings& tex_settings,
   const fs::path& path,
   const bool linearize,
   const bool flip_load
) const -> id
{
   glm::ivec3 dim{ 1, 1, 1 };
   int bpp = 4;
   unsigned char* data = nullptr;
   {
      stbi_set_flip_vertically_on_load(flip_load ? 1 : 0);
      data = stbi_load(path.string().c_str(), &dim[0], &dim[1], &bpp, 0);
   }
   const int byte_size = dim[0] * dim[1] * bpp;
   std::vector<uint8_t> pixels(byte_size);
   {
      std::memcpy(pixels.data(), data, byte_size);
   }
   if (linearize)
   {
      for (int i = 0; i < pixels.size(); ++i)
      {
         if (i % 4 == 3) // skip alpha
            continue;
         pixels[i] = srgb_to_linear_ui8(pixels[i]);
      }
   }

   sfn_assert(data != nullptr);
   sfn_assert(bpp == 4);
   return create_2D(tex_settings, dim, bpp, pixels, path);
}


auto texture_manager::create_from_r8_file(const texture_settings& tex_settings, const fs::path& path) const -> id
{
   glm::ivec3 dim{ 1, 1, 1 };
   int bpp = 1;
   unsigned char* data = stbi_load(path.string().c_str(), &dim[0], &dim[1], &bpp, 0);
   const int byte_size = dim[0] * dim[1] * bpp;
   std::vector<uint8_t> pixels(byte_size);
   std::memcpy(pixels.data(), data, byte_size);

   sfn_assert(data != nullptr);
   sfn_assert(bpp == 1);
   return create_2D(tex_settings, dim, bpp, pixels, path);
}


auto texture_manager::get_texture_id(const std::string& filename) const -> id
{
   const auto pred = [&](const texture& tex)
   {
      return filename == tex.m_filename;
   };
   const auto it = std::ranges::find_if(m_textures, pred);
   if (it == std::end(m_textures))
   {
      // log::error("texture not found");
      std::terminate();
   }
   return it->m_id;
}


// auto texture_manager::create_new(
//    const image_metrics& image_metrics,
//    const texture_settings& tex_settings
// ) const -> id
// {
//    const texture& ref = m_textures.emplace_back(explicit_init{}, "not_from_file", default_texture_target, get_empty_pixel_data(image_metrics), tex_settings);
//    const int texture_unit = get_free_texture_unit();
//    ref.bind(texture_unit);
//    m_texture_unit_state[texture_unit] = ref.m_id;
//    return ref.m_id;
// }


auto texture_manager::create_new_depth(const glm::ivec2& resolution) const -> id
{
   const texture& ref = m_textures.emplace_back(depth_tag{}, default_texture_target, resolution);
   const int texture_unit = get_free_texture_unit();
   ref.bind(texture_unit);
   m_texture_unit_state[texture_unit] = ref.m_id;
   return ref.m_id;
}


auto texture_manager::get_texture_unit(const id tex_id) const -> int
{
   for (int i = 0; i < m_texture_unit_state.size(); ++i)
   {
      if (m_texture_unit_state[i].has_value() == true && m_texture_unit_state[i].value() == tex_id)
      {
         return i;
      }
   }
   // log::error("couldn't find texture unit of texture");
   std::terminate();
}

auto texture_manager::get_texture_unit(const std::string& name) const -> int
{
   return this->get_texture_unit(this->get_texture_id(name));
}


auto texture_manager::get_free_texture_unit() const -> int
{
   for(int i=0; i<m_texture_unit_state.size(); ++i)
   {
      if(m_texture_unit_state[i].has_value() == false)
      {
         return i;
      }
   }
   // log::error("couldn't find any free texture unit!");
   std::terminate();
}
