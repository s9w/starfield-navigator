#include "tools.h"





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
