#include <variant>
#include <vector>
#include <string>
// #include <numbers>

#include "universe.h"

#pragma warning(push, 0)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <biteopt.h>
#pragma warning(pop)


namespace sfn
{
   struct real {
      std::string m_hip;
      glm::vec3 m_coordinates;
   };

   struct real_universe {
      std::vector<real> m_stars;

      [[nodiscard]] auto get_pos_by_hip(const std::string& hip) const -> glm::vec3;
      [[nodiscard]] auto get_name_by_index(const int i) const -> std::string;
      [[nodiscard]] auto get_index_by_name(const std::string& hip) const -> int;
   };

   struct CTestOpt : public CBiteOpt
   {
      const universe* fiction_ref;
      const real_universe* real_ref;
      // 0, 1, 2: rotation angles
      // 3, 4, 5: scale factors
      // 6, 7, 8: translation
      CTestOpt();

      auto getMinValues(double* const p) const -> void override;
      auto getMaxValues(double* const p) const -> void override;
      [[nodiscard]] static auto get_trafo_from_vector(const double* const p) -> glm::mat4;
      auto optcost(const double* const p) -> double override;
   };

   using creator_result = std::variant<float, universe>;
   struct universe_creator{
      real_universe m_real_universe;
      universe m_starfield_universe;
      CTestOpt opt;
      CBiteRnd rnd;
      int i = 0;

      explicit universe_creator();
      [[nodiscard]] auto get() -> creator_result;
   };

}
