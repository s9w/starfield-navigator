#include "tools.h"





auto sfn::id::create() -> id
{
   static id::underlying_type next_id_value = 0;
   sfn_assert(
      next_id_value != std::numeric_limits<id::underlying_type>::max(),
      "id type overflowing. increase size!"
   );

   id new_id{ explicit_init{}, next_id_value };
   next_id_value++;
   return new_id;
}
