#pragma once

#include <concepts>
#include <filesystem>
namespace fs = std::filesystem;

#include <s9w_colors.h>
#include <glm/vec2.hpp>

namespace sg
{

   template<typename T, typename ... types>
   concept t_in_types = (std::same_as<T, types> || ...);

   template<typename T>
   concept valid_tp = t_in_types<T, s9w::srgb_gray_u, s9w::srgba_gray_u, s9w::srgb_u, s9w::srgba_u, s9w::linear_r_f>;

   // TODO constraint read/save with this
   template<typename T>
   concept io_canvas_types = t_in_types<T, s9w::srgb_gray_u, s9w::srgba_gray_u, s9w::srgb_u, s9w::srgba_u>;

   // min/max {column, row}
   struct zone
   {
      glm::ivec2 m_min{};
      glm::ivec2 m_max{};
   };

   
   template<sg::valid_tp color_type>
   struct typed_canvas
   {
      constexpr static inline int bpp = color_type::components;

      glm::ivec2 m_dimensions; // m_width, height
      std::vector<color_type> m_pixels;

      // All components zero for default ctors
      explicit typed_canvas(const int width, const int height, const color_type default_color = color_type{});
      explicit typed_canvas(const glm::ivec2 dimensions, const color_type default_color = color_type{});

      [[nodiscard]] auto get_byte_count() const -> int;
      [[nodiscard]] auto get_index(const glm::ivec2 coordinate) const -> size_t;
      [[nodiscard]] auto get_size() const -> size_t;
      [[nodiscard]] auto get_color(const glm::ivec2 coordinate) const -> const color_type&;
      [[nodiscard]] auto get_color(const glm::ivec2 coordinate)       ->       color_type&;
      [[nodiscard]] auto get_width() const -> int;
      [[nodiscard]] auto get_height() const -> int;
      auto copy_into(const typed_canvas<color_type>& other, const glm::ivec2 insert_pos) -> void;

      [[nodiscard]] auto operator[](const size_t index) const -> const color_type&;
      [[nodiscard]] auto operator[](const size_t index) -> color_type&;

      template<typename pred>
      [[nodiscard]] auto get_zone(const pred& predicate) const -> zone;

      [[nodiscard]] auto begin() const -> typename std::vector<color_type>::const_iterator
      {
         return std::cbegin(m_pixels);
      }
      [[nodiscard]] auto end() const -> typename std::vector<color_type>::const_iterator
      {
         return std::cend(m_pixels);
      }
      [[nodiscard]] auto begin() -> typename std::vector<color_type>::iterator
      {
         return std::begin(m_pixels);
      }
      [[nodiscard]] auto end() -> typename std::vector<color_type>::iterator
      {
         return std::end(m_pixels);
      }

   private:
      template<typename pred>
      [[nodiscard]] auto does_column_satisfy_pred(const int column, const pred& predicate) const -> bool;
      template<typename pred>
      [[nodiscard]] auto does_row_satisfy_pred(const int row, const pred& predicate) const -> bool;
   };


   // Offset means (x, y)
   template<sg::valid_tp color_type>
   auto blit_canvas(typed_canvas<color_type>& target, const typed_canvas<color_type>& source, const glm::ivec2& offset) -> void;

   template<sg::valid_tp color_type>
   [[nodiscard]] auto load_canvas_from_file(const fs::path& path) -> typed_canvas<color_type>;

   template<sg::valid_tp color_type>
   auto save_canvas_to_file(const typed_canvas<color_type>& canvas, const fs::path& path) -> void;
}



template <sg::valid_tp color_type>
template <typename pred>
auto sg::typed_canvas<color_type>::does_column_satisfy_pred(const int column, const pred& predicate) const -> bool
{
   for (int row = 0; row < this->get_height(); ++row)
   {
      const glm::ivec2 coord{ column, row };
      if (predicate(this->get_color(coord)))
      {
         return true;
      }
   }
   return false;
}


template <sg::valid_tp color_type>
template <typename pred>
auto sg::typed_canvas<color_type>::does_row_satisfy_pred(const int row, const pred& predicate) const -> bool
{
   for (int column = 0; column < this->get_height(); ++column)
   {
      const glm::ivec2 coord{ column, row };
      if (predicate(this->get_color(coord)))
      {
         return true;
      }
   }
   return false;
}


template <sg::valid_tp color_type>
template <typename pred>
auto sg::typed_canvas<color_type>::get_zone(const pred& predicate) const -> zone
{
   // init with invalid range
   zone result{
      .m_min = {this->get_width() - 1, this->get_height() - 1},
      .m_max = {0, 0}
   };
   for (int column = 0; column < this->get_width(); ++column)
   {
      if (this->does_column_satisfy_pred(column, predicate))
      {
         result.m_min[0] = column;
         break;
      }
   }
   for (int column = this->get_width() - 1; column >= 0; --column)
   {
      if (this->does_column_satisfy_pred(column, predicate))
      {
         result.m_max[0] = column;
         break;
      }
   }
   for (int row = 0; row < this->get_height(); ++row)
   {
      if (this->does_row_satisfy_pred(row, predicate))
      {
         result.m_min[1] = row;
         break;
      }
   }
   for (int row = this->get_height() - 1; row >= 0; --row)
   {
      if (this->does_row_satisfy_pred(row, predicate))
      {
         result.m_max[1] = row;
         break;
      }
   }

   return result;
}


template <sg::valid_tp color_type>
auto sg::blit_canvas(
   typed_canvas<color_type>& target,
   const typed_canvas<color_type>& source,
   const glm::ivec2& offset
) -> void
{
   if(offset[0]+source.get_width() > target.get_width())
   {
      std::terminate();
   }
   if (offset[1] + source.get_height() > target.get_height())
   {
      std::terminate();
   }
   for(int row=0; row<source.get_height(); ++row)
   {
      for(int column=0; column<source.get_width(); ++column)
      {
         const int source_index = row * source.get_width() + column;
         const int target_index = (row + offset[1]) * target.get_width() + column + offset[0];
         target[target_index] = source[source_index];
      }
   }
}
