#include "canvas.h"

#include <stb_image.h>
#include <stb_image_write.h>


namespace
{

   auto save_to_file(
      const fs::path& path,
      const int width,
      const int height,
      const int bpp,
      const void* data
   ) -> void
   {
      stbi_flip_vertically_on_write(1);
      const int stride = width * bpp;
      stbi_write_png(path.string().c_str(), width, height, bpp, data, stride);
   }
    
} // namespace {}


template<sg::valid_tp color_type>
sg::typed_canvas<color_type>::typed_canvas(const int width, const int height, const color_type default_color)
   : m_dimensions{width, height}
   , m_pixels(m_dimensions[0] * m_dimensions[1], default_color)
{ }


template<sg::valid_tp color_type>
sg::typed_canvas<color_type>::typed_canvas(const glm::ivec2 dimensions, const color_type default_color)
   : typed_canvas(dimensions[0], dimensions[1], default_color)
{ }


template<sg::valid_tp color_type>
auto sg::load_canvas_from_file(const fs::path& path) -> sg::typed_canvas<color_type>
{
   if(fs::exists(path) == false)
   {
      std::terminate();
   }

   glm::ivec2 dimensions;
   int bpp;
   stbi_set_flip_vertically_on_load(1);
   stbi_uc* data = stbi_load(path.string().c_str(), &dimensions[0], &dimensions[1], &bpp, 0);
   if(data == nullptr)
   {
      std::terminate();
   }
   typed_canvas<color_type> result(dimensions);
   std::memcpy(result.m_pixels.data(), data, result.get_byte_count());
   stbi_image_free(data);
   return result;
}
template auto sg::load_canvas_from_file(const fs::path& path) -> sg::typed_canvas<s9w::srgb_gray_u>;
template auto sg::load_canvas_from_file(const fs::path& path) -> sg::typed_canvas<s9w::srgba_gray_u>;
template auto sg::load_canvas_from_file(const fs::path& path) -> sg::typed_canvas<s9w::srgb_u>;
template auto sg::load_canvas_from_file(const fs::path& path) -> sg::typed_canvas<s9w::srgba_u>;


template <sg::valid_tp color_type>
auto sg::save_canvas_to_file(
   const typed_canvas<color_type>& canvas,
   const fs::path& path
) -> void
{
   save_to_file(path, canvas.m_dimensions[0], canvas.m_dimensions[1], canvas.bpp, canvas.m_pixels.data());
}
template auto sg::save_canvas_to_file(const sg::typed_canvas<s9w::srgb_gray_u>&, const fs::path& path) -> void;
template auto sg::save_canvas_to_file(const sg::typed_canvas<s9w::srgba_gray_u>&, const fs::path& path) -> void;
template auto sg::save_canvas_to_file(const sg::typed_canvas<s9w::srgb_u>&, const fs::path& path) -> void;
template auto sg::save_canvas_to_file(const sg::typed_canvas<s9w::srgba_u>&, const fs::path& path) -> void;


template<sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_byte_count() const -> int
{
   return m_dimensions[0] * m_dimensions[1] * bpp;
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_index(const glm::ivec2 coordinate) const -> size_t
{
   return coordinate[1] * m_dimensions[0] + coordinate[0];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_size() const -> size_t
{
   return m_dimensions[0] * m_dimensions[1];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_color(const glm::ivec2 coordinate) const -> const color_type&
{
   return m_pixels[this->get_index(coordinate)];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_color(const glm::ivec2 coordinate) -> color_type&
{
   return m_pixels[this->get_index(coordinate)];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_width() const -> int
{
   return m_dimensions[0];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::get_height() const -> int
{
   return m_dimensions[1];
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::copy_into(
   const typed_canvas<color_type>& other,
   const glm::ivec2 insert_pos
) -> void
{
   for(int row=0; row<other.get_height(); ++row)
   {
      for(int column=0; column<other.get_width(); ++column)
      {
         const glm::ivec2 other_coord{column, row};
         const glm::ivec2 this_coord = other_coord + insert_pos;
         this->get_color(this_coord) = other.get_color(other_coord);
      }
   }
}


template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::operator[](const size_t index) const -> const color_type&
{
   return m_pixels[index];
}

template <sg::valid_tp color_type>
auto sg::typed_canvas<color_type>::operator[](const size_t index) -> color_type&
{
   const auto& const_ref = std::as_const(*this);
   return const_cast<color_type&>(const_ref[index]);
}


template sg::typed_canvas<s9w::srgb_gray_u>;
template sg::typed_canvas<s9w::srgba_gray_u>;
template sg::typed_canvas<s9w::srgb_u>;
template sg::typed_canvas<s9w::srgba_u>;
template sg::typed_canvas<s9w::linear_r_f>;

template auto sg::load_canvas_from_file<s9w::srgb_gray_u>(const fs::path&) -> typed_canvas<s9w::srgb_gray_u>;
template auto sg::load_canvas_from_file<s9w::srgba_gray_u>(const fs::path&) -> typed_canvas<s9w::srgba_gray_u>;
template auto sg::load_canvas_from_file<s9w::srgb_u>(const fs::path&) -> typed_canvas<s9w::srgb_u>;
template auto sg::load_canvas_from_file<s9w::srgba_u>(const fs::path&) -> typed_canvas<s9w::srgba_u>;

