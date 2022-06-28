#pragma once

#include <vector>

#include "tools.h"

namespace sfn
{

   template<typename T>
   struct vector_map
   {
   private:
      struct element
      {
         id key;
         T value;
      };

      std::vector<element> m_container;

   public:
      auto emplace(const T& value) -> id;
      [[nodiscard]] auto at(const id key) const -> const T&;

      template<typename pred_type>
      [[nodiscard]] auto get_max_element_id(const pred_type& pred) const -> id;

      auto delete_elem(const id key) -> void;
   };
   
}


template <typename T>
auto sfn::vector_map<T>::emplace(const T& value) -> id
{
   const id new_id = id::create();
   m_container.emplace_back(new_id, value);
   return new_id;
}


template <typename T>
auto sfn::vector_map<T>::at(const id key) const -> const T&
{
   const auto pred = [&](const element& elem)
   {
      return elem.key == key;
   };
   const auto it = std::ranges::find_if(m_container, pred);
   return it->value;
}


template <typename T>
template <typename pred_type>
auto sfn::vector_map<T>::get_max_element_id(const pred_type& pred) const -> id
{
   const auto outer_pred = [&](const element& elem_a, const element& elem_b)
   {
      return pred(elem_a.value, elem_b.value);
   };
   const auto it = std::ranges::max_element(m_container, outer_pred);
   return it->key;
}


template <typename T>
auto sfn::vector_map<T>::delete_elem(const id key) -> void
{
   const auto pred = [&](const element& elem)
   {
      return elem.key == key;
   };
   std::erase_if(m_container, pred);
}
