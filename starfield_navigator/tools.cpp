#include "tools.h"


auto sfn::get_split_string(
   std::string source,
   const std::string& delim
) -> std::vector<std::string>
{
   if (delim.empty())
      std::terminate();

   std::vector<std::string> parts;
   for (auto pos = source.find(delim);
        pos != std::string::npos;
        pos = source.find(delim))
   {
      parts.emplace_back(source.substr(0, pos));
      source.erase(0, pos + delim.length());
   }
   parts.emplace_back(source);
   return parts;
}


auto sfn::get_trimmed_str(
   const std::string& str,
   const std::string& to_trim
) -> std::string
{
   const auto lpos = str.find_first_not_of(to_trim);
   if (lpos == std::string::npos)
      return str;
   const auto rpos = str.find_last_not_of(to_trim);
   return str.substr(lpos, rpos - lpos + 1);
}


auto sfn::id::create() -> id
{
   static std::atomic<id::underlying_type> next_id_value = 0;

#ifdef _DEBUG
   sfn_assert(
      next_id_value != std::numeric_limits<id::underlying_type>::max(),
      "id type overflowing. increase size!"
   );
#endif

   return id{ explicit_init{}, next_id_value.fetch_add(1)};
}


auto sfn::get_average(const std::vector<float>& vec) -> float
{
   float sum = 0.0f;
   for (const float elem : vec)
      sum += elem;
   return sum / std::size(vec);
}
