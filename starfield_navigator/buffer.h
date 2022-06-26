#pragma once

#include "shader.h"
#include "tools.h"

#include <variant>
#include <span>


namespace sfn
{
   // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
   // GL_STATIC_DRAW : the data is set only onceand used many times.
   // GL_DYNAMIC_DRAW : the data is changed a lotand used many times.
   enum class usage_pattern { static_draw, dynamic_draw };

   struct ubo_segment
   {
      id m_id;
      int m_sizeof{};
      std::string m_description;

      explicit ubo_segment(const int byte_count, const std::string& description);
      [[nodiscard]] auto get_byte_count() const -> int;
   };

   struct ssbo_segment
   {
      id m_id;
      int m_sizeof{};
      std::string m_description;

      explicit ssbo_segment(const int byte_count, const std::string& description);
      [[nodiscard]] auto get_byte_count() const -> int;
   };

   struct vbo_class_member
   {
      data_type m_type;
      std::string m_attrib_name; // this is used to retrieve the location!
      bool m_normalized = false;

      explicit vbo_class_member(const data_type data_type, const std::string& attrib_name, const bool normalized);
      [[nodiscard]] auto get_byte_count() const -> int;
   };

   struct vbo_class
   {
      std::vector<vbo_class_member> m_attributes;
      int m_element_count = 0;

      explicit vbo_class(const std::vector<vbo_class_member>& attributes, const int element_count);

      [[nodiscard]] auto element_byte_count() const -> int;
      [[nodiscard]] auto get_byte_count() const -> int;
   };

   struct vbo_segment
   {
      id m_id;
      std::vector<vbo_class> m_classes;

      explicit vbo_segment(const std::vector<vbo_class>& classes);
      [[nodiscard]] auto get_byte_count() const -> int;
      [[nodiscard]] auto get_aos_stride() const -> int;

      // this requires adding to offset of vbo segment itself
      [[nodiscard]] auto get_vbo_class_relative_offset(const int class_index) const -> int;
   };

   struct segment_type : std::variant<vbo_segment, ubo_segment, ssbo_segment>
   {
      [[nodiscard]] auto get_byte_count() const -> int;
      [[nodiscard]] auto get_id() const -> id;
   };


   struct buffer
   {
      id m_id;
      GLuint m_buffer_opengl_id = 0;
      usage_pattern m_usage_pattern;
      std::vector<segment_type> m_segments;

      explicit buffer(const usage_pattern usage, std::vector<segment_type>&& segments);
      [[nodiscard]] auto get_byte_count() const -> int;
      [[nodiscard]] auto is_segment_in_buffer(const id target_segment_id) const -> bool;
      [[nodiscard]] auto get_segment_offset(const id segment_id) const -> int;
      [[nodiscard]] auto get_segment_size(const id segment_id) const -> int;
      [[nodiscard]] auto get_opt_segment_ref(const id segment_id) const -> std::optional<std::reference_wrapper<const segment_type>>;
      [[nodiscard]] auto get_opt_vbo_ref(const id segment_id) const -> std::optional<std::reference_wrapper<const vbo_segment>>;
   };

   struct rect_index_buffer
   {
      id m_id;
      GLuint m_opengl_id{};
      int m_rect_count;

      explicit rect_index_buffer(const int rect_count);
      [[nodiscard]] auto get_byte_count() const -> int;
   };

   struct buffers
   {
      rect_index_buffer m_rect_index_buffer;
      std::vector<buffer> m_buffers;

      explicit buffers(const int rect_count);
      auto create_buffer(std::vector<segment_type>&& segments, const usage_pattern usage) -> std::vector<id>;

      [[nodiscard]] auto get_buffer_ref(const id buffer_id) const -> const buffer&;
      [[nodiscard]] auto get_single_buffer_ref() const -> const buffer&;
      [[nodiscard]] auto get_buffer_ref_from_segment_id(const id segment_id) const -> const buffer&;
      [[nodiscard]] auto get_segment_size(const id segment_id) const -> size_t;
      [[nodiscard]] auto get_vbo_ref(const id vbo_segment_id) const -> const vbo_segment&;
      [[nodiscard]] auto get_buffer_opengl_id_from_buffer(const id buffer_id) const -> GLuint;
      
      auto upload_vbo(const id vbo_id, const std::span<const std::byte> data) const -> void;
      auto upload_ubo(const id ubo_id, const std::span<const std::byte> data) const -> void;
      auto upload_ssbo_impl(const id ssbo_id, const std::span<const std::byte> data, const int byte_offset) const -> void;
      auto upload_ssbo(const id ssbo_id, const std::span<const std::byte> data, const int offset) const -> void;

      template<typename T>
      auto upload_ssbo_partial(const id ssbo_id, const std::vector<T>& data, const int count_offset) const -> void
      {
         const int extra_offset = count_offset * sizeof(T);
         upload_ssbo_impl(ssbo_id, as_bytes(data), extra_offset);
      }

      // element count provided because alignment rules make determining it impossible
      //template<typename T>
      //auto download_ssbo(const id ssbo_id, const int element_count) -> std::vector<T>
      //{
      //   const buffer& boof = get_buffer_ref_from_segment_id(ssbo_id);
      //   const GLuint buffer_opengl_id = boof.m_buffer_opengl_id;
      //   const int byte_count = element_count * sizeof(T);
      //   const int offset = boof.get_segment_offset(ssbo_id);
      //   std::vector<T> result(element_count);
      //   void* ptr = result.data();
      //   glGetNamedBufferSubData(buffer_opengl_id, offset, byte_count, ptr);
      //   return result;
      //}

      template<typename T>
      auto download_trash_ssbo(const id ssbo_id) -> T
      {
         const buffer& boof = get_buffer_ref_from_segment_id(ssbo_id);
         const GLuint buffer_opengl_id = boof.m_buffer_opengl_id;
         const int offset = boof.get_segment_offset(ssbo_id);
         T result;
         glGetNamedBufferSubData(buffer_opengl_id, offset, sizeof(T), &result);
         return result;
      }
   };

   // SOA = struct of arrays = ECS
   // AOS = array of structs = std::vector<fat_class>
   [[nodiscard]] auto is_shader_and_vbo_aos_compat(
      const std::vector<shader_io>& shader_inputs,
      const std::vector<vbo_class_member>& vbo_segment_attributes
   ) -> bool;


   struct vao
   {
      GLuint m_vao_id = 0;

      explicit vao(const buffers& buffers, const id vbo_id, const shader_program& shader, const std::optional<GLuint> index_buffer_opengl_id);
      explicit vao(const buffers& buffers, const id vbo_id, const shader_program& shader);
      auto bind() const -> void;
   };

}
