#include "universe_creation.h"

#include <vector>
#include <fstream>

#include "universe.h"
#include "tools.h"

#pragma warning(push, 0)
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <biteopt.h>
#pragma warning(pop)

namespace {
   using namespace sfn;


   [[nodiscard]] auto get_and_delete_cam_info(std::vector<sfn::system>& systems) -> cam_info
   {
      const auto generator = [&](const std::string& target)
      {
         const auto camera_pred = [&](const sfn::system& sys)
         {
            return sys.m_name.starts_with(target);
         };
         const auto it = std::ranges::find_if(systems, camera_pred);
         if (it == std::end(systems))
            std::terminate();
         const glm::vec3 copy = it->m_position;
         systems.erase(it);
         return copy;
      };
      const glm::vec3 cam0 = generator("cam0");
      const glm::vec3 cam1 = generator("cam1");
      const glm::vec3 cam_up = generator("cam_up");
      const glm::vec3 cam_front = generator("cam_front");
      return cam_info{
         .m_cs = cs(glm::normalize(cam_front - cam0), glm::normalize(cam_up - cam0)),
         .m_cam_pos0 = cam0,
         .m_cam_pos1 = cam1
      };
   }


   [[nodiscard]] auto get_catalog_id(const std::string& id_str) -> catalog_id
   {
      if(id_str.starts_with("HIP_"))
      {
         return catalog_id{
            hip_id{
               .m_id = std::stoi(id_str.substr(4))
            }
         };
      }
      else if (id_str.starts_with("GLIESE_"))
      {
         return catalog_id{
            gliese_id{
               .m_id = id_str.substr(7)
            }
         };
      }
      std::terminate();
   }


   auto get_real_entries() -> real_universe
   {
      // std::ifstream input("cc_hip1997.txt");
      // std::ifstream input("cc_hip2007.txt");
      std::ifstream input("cc_hyg.txt");
      real_universe result;
      for (std::string line; getline(input, line); )
      {
         if (line.starts_with("#"))
            continue;
         const auto split = get_split_string(line, ";");
         const std::string catalog_str = get_trimmed_str(split[0]);
         const galactic_coord galactic{
            .m_l = glm::radians(std::stof(split[1])),
            .m_b = glm::radians(std::stof(split[2])),
            .m_dist = std::stof(split[3])
         };
         const float abs_mag = std::stof(split[5]);

         result.m_stars.emplace(
            get_catalog_id(catalog_str),
            real_star{
               .m_position = galactic.get_cartesian(),
               .m_abs_mag = abs_mag
            }
            
         );
      }

      return result;
   }


   [[nodiscard]] auto get_error(
      const universe& fiction,
      const ::real_universe& real,
      const std::string& fictional_name,
      const std::string& hip
   ) -> float
   {
      const glm::vec3 fiction_pos = fiction.get_position_by_name(fictional_name);
      const glm::vec3 real_pos = real.get_star_by_cat_id(hip).m_position;
      return glm::distance(fiction_pos, real_pos);
   };


   [[nodiscard]] auto get_metric(
      const universe& univ,
      const ::real_universe& real
   ) -> float
   {
      std::vector<float> errors;
      errors.reserve(100);

      for (const sfn::system& system : univ.m_systems)
      {
         if (system.m_astronomic_name.empty() || system.m_astronomic_name == "Sol" || system.m_speculative == true)
            continue;
         errors.push_back(get_error(univ, real, system.m_astronomic_name, system.m_catalog_lookup));
      }
      return get_average(errors);
   };


   [[nodiscard]] auto get_generic_trafo(
      const float x_angle,
      const float y_angle,
      const float z_angle,
      const glm::vec3& scale,
      const glm::vec3& shift
   ) -> glm::mat4
   {
      glm::mat4 trafo(1.0f);
      trafo = glm::translate(trafo, shift);
      trafo = glm::scale(trafo, scale);
      trafo = glm::rotate(trafo, x_angle, glm::vec3{ 1, 0, 0 });
      trafo = glm::rotate(trafo, y_angle, glm::vec3{ 0, 1, 0 });
      trafo = glm::rotate(trafo, z_angle, glm::vec3{ 0, 0, 1 });
      return trafo;
   }


   [[nodiscard]] auto get_system_size(const std::string& size_str) -> system_size
   {
      if (size_str == "big")
         return system_size::big;
      if (size_str == "small")
         return system_size::small;
      std::terminate();
   }


   [[nodiscard]] auto get_unexplored_bb(
      const bb_3D& visible_bb,
      const glm::vec3& sol_position
   ) -> bb_3D
   {
      bb_3D result = visible_bb;

      // Align left to Sol
      result.m_min[0] = sol_position[0];

      // Shift to the left
      const float width = result.get_size()[0];
      const glm::vec3 shift{ -width, 0, 0 };
      result.m_min += shift;
      result.m_max += shift;

      return result;
   }

} // namespace {}


catalog_id::catalog_id(const std::variant<hip_id, gliese_id>& var)
{
   if (std::holds_alternative<hip_id>(var))
   {
      m_string_cache = fmt::format("HIP {}", std::get<hip_id>(var).m_id);
   }
   if (std::holds_alternative<gliese_id>(var))
   {
      m_string_cache = fmt::format("GLIESE {}", std::get<gliese_id>(var).m_id);
   }
}

catalog_id::catalog_id(const std::string& cache)
   : m_string_cache(cache)
{

}

auto sfn::catalog_id::get_user_str() const -> const std::string&
{
   return m_string_cache;
   
}

auto real_universe::get_star_by_cat_id(const std::string& cat_id) const -> const real_star&
{
   return m_stars.at(catalog_id(cat_id));
}


sfn::CTestOpt::CTestOpt()
{
   updateDims(9);
}

void sfn::CTestOpt::getMinValues(double* const p) const
{
   p[0] = 0.0; p[1] = 0.0; p[2] = 0.0;
   p[3] = 0.01; p[4] = 0.01; p[5] = 0.01;
   p[6] = -100.0; p[7] = -100.0; p[8] = -100.0;
}

void sfn::CTestOpt::getMaxValues(double* const p) const
{
   p[0] = 2.0 * std::numbers::pi_v<double>; p[1] = 2.0 * std::numbers::pi_v<double>; p[2] = 2.0 * std::numbers::pi_v<double>;
   p[3] = 10.0; p[4] = 10.0; p[5] = 10.0;
   p[6] = 100.0; p[7] = 100.0; p[8] = 100.0;
}

double sfn::CTestOpt::optcost(const double* const p)
{
   universe transformed_universe = *fiction_ref;
   const glm::mat4 trafo = get_trafo_from_vector(p);
   for (sfn::system& elem : transformed_universe.m_systems)
   {
      elem.m_position = apply_trafo(trafo, elem.m_position);
   }
   return static_cast<double>(get_metric(transformed_universe, *real_ref));
}

auto sfn::CTestOpt::get_trafo_from_vector(const double* const p) -> glm::mat4
{
   return get_generic_trafo(p[0], p[1], p[2], glm::vec3{ p[3], p[4], p[5] }, glm::vec3{ p[6], p[7], p[8] });
}


universe_creator::universe_creator()
   : m_real_universe(get_real_entries())
{
   std::ifstream input("system_data.txt");

   enum class read_mode{tracking, naming, speculative};
   read_mode mode = read_mode::tracking;
   std::unordered_map<std::string, glm::vec3> read_data;
   for (std::string line; getline(input, line); )
   {
      {
         const size_t comment_begin = line.find('#');
         if (comment_begin != std::string::npos)
         {
            line = line.substr(0, comment_begin);

            // Comment-only lines shouldn't trigger anything
            if (get_trimmed_str(line).empty())
               continue;
         }
      }
      line = get_trimmed_str(line);

      if (mode == read_mode::tracking && line.empty())
      {
         mode = read_mode::naming;
         continue;
      }
      if (mode == read_mode::naming && line.empty())
      {
         mode = read_mode::speculative;
         continue;
      }
      if (mode == read_mode::tracking)
      {
         const std::vector<std::string> split = get_split_string(line, ";");
         const std::string name = split[0];
         const float x = static_cast<float>(std::stod(get_trimmed_str(split[1])));
         const float y = static_cast<float>(std::stod(get_trimmed_str(split[2])));
         const float z = static_cast<float>(std::stod(get_trimmed_str(split[3])));
         read_data.emplace(name, c4d_convert(glm::vec3{ x, y, z }));
      }
      else if (mode == read_mode::naming)
      {
         const std::vector<std::string> key_value = get_split_string(line, ":");

         const std::vector<std::string> values = get_split_string(key_value[1], ";");
         std::string astronomical_name;
         std::string catalog_entry;
         if (values[2].empty() == false)
         {
            const auto astro_split = get_split_string(values[2], "_");
            if (astro_split.size() == 1)
               std::terminate();
            astronomical_name = astro_split[0];
            catalog_entry = astro_split[1];
         }
         if (values[1] == "SOL")
            astronomical_name = "Sol";

         const std::string starfield_name = values[1];

         std::string name = key_value[0];
         const glm::vec3 pos = read_data.at(name);
         if (starfield_name.empty() == false)
            name = starfield_name;
         float mag = 999.0f;
         float abs_mag = 999.0f;
         if (catalog_entry.empty() == false)
         {
            abs_mag = m_real_universe.get_star_by_cat_id(catalog_entry).m_abs_mag;
         }
         constexpr bool speculative = false;
         m_starfield_universe.m_systems.emplace_back(pos, name, astronomical_name, catalog_entry, get_system_size(values[0]), abs_mag, speculative);
      }
      else if(mode == read_mode::speculative)
      {
         const std::vector<std::string> split = get_split_string(line, ";");
         const std::string name = split[0];
         const std::string catalog_entry = split[1];
         const glm::vec3 pos = m_real_universe.get_star_by_cat_id(catalog_entry).m_position;
         const float abs_mag = m_real_universe.get_star_by_cat_id(catalog_entry).m_abs_mag;
         constexpr bool speculative = true;
         m_starfield_universe.m_systems.emplace_back(pos, name, name, catalog_entry, system_size::small, abs_mag, speculative);
      }
   }
   // Sort from left to right before transformation is applied
   const auto pred = [](const sfn::system& a, const sfn::system& b) {
      return a.m_position.x < b.m_position.x;
   };
   std::ranges::sort(m_starfield_universe.m_systems, pred);


   rnd.init(1); // Needs to be seeded with different values on each run.
   opt.fiction_ref = &m_starfield_universe;
   opt.real_ref = &m_real_universe;
   opt.init(rnd);
}


auto sfn::universe_creator::get() -> creator_result
{
   constexpr int n = 20000;
   constexpr int increment = n / 100;
   for (int run = 0; run<increment; ++i, ++run)
   {
      opt.optimize(rnd);
      constexpr double tol = 0.001;
      if (opt.getBestCost() < tol)
      {
         break;
      }
   }

   if(i<n)
   {
      return static_cast<float>(i) / static_cast<float>(n);
   }

   else
   {
      return this->get_finished_result();
   }
}


auto universe_creator::get_finished_result() -> universe
{
   printf("IterCount: %i\n", i);
   printf("BestCost: %f\n", opt.getBestCost());
   for (int p = 0; p < 9; ++p)
      fmt::print("best params {}: {:.2f}\n", p, opt.getBestParams()[p]);

   const auto no_speculative_or_cam = [](const auto& vertex)
   {
      return vertex.m_speculative == false && vertex.m_name.starts_with("cam") == false;
   };
   const bb_3D old_coord_bb = get_bb(m_starfield_universe.m_systems, no_speculative_or_cam);

   // Apply trafo
   const glm::mat4 final_transformation = CTestOpt::get_trafo_from_vector(opt.getBestParams());
   for (sfn::system& sys : m_starfield_universe.m_systems)
   {
      if (sys.m_speculative == true)
         continue;
      sys.m_position = apply_trafo(final_transformation, sys.m_position);
   }
   fmt::print("metric with optimized trafo: {:.2f} LY\n", get_metric(m_starfield_universe, m_real_universe));


   const auto candidates_for_fictional = [&](const std::string& fictional_name)
   {
      const glm::vec3 pos0 = m_starfield_universe.get_position_by_name(fictional_name);
      std::vector<catalog_id> real_closest;
      for (const auto& [key, value] : m_real_universe.m_stars)
      {
         real_closest.push_back(key);
      }
      const auto pred = [&](const catalog_id& i, const catalog_id& j)
      {
         const float dist_i = glm::distance(pos0, m_real_universe.m_stars.at(i).m_position);
         const float dist_j = glm::distance(pos0, m_real_universe.m_stars.at(j).m_position);
         return dist_i < dist_j;
      };
      std::ranges::sort(real_closest, pred);
      fmt::print("\n");
      for (int i = 0; i < 3; ++i)
      {
         const float dist = glm::distance(pos0, m_real_universe.m_stars.at(real_closest[i]).m_position);
         fmt::print(
            "{}: {}, dist: {:.2f}\n",
            i, real_closest[i].get_user_str(), dist
         );
      }
   };
   const auto candidates_for_real = [&](const std::string& cat_id)
   {
      const glm::vec3 target_pos = m_real_universe.get_star_by_cat_id(cat_id).m_position;
      std::vector<int> real_closest;
      for (int i = 0; i < std::ssize(m_starfield_universe.m_systems); ++i)
         real_closest.push_back(i);
      const auto pred = [&](const int i, const int j)
      {
         const float dist_i = glm::distance(target_pos, m_starfield_universe.m_systems[i].m_position);
         const float dist_j = glm::distance(target_pos, m_starfield_universe.m_systems[j].m_position);
         return dist_i < dist_j;
      };
      std::ranges::sort(real_closest, pred);
      // fmt::print("\n");
      for (int i = 0; i < 3; ++i)
      {
         const float dist = glm::distance(target_pos, m_starfield_universe.m_systems[real_closest[i]].m_position);
         // fmt::print("{}: {:.2f} {}\n", i, dist, m_starfield_universe.m_systems[i].get_name());
      }
   };
   // candidates_for_real("HIP 91262"); // vega
   candidates_for_fictional("ADP");

   const auto error_report = [&](const std::string& fictional_name, const std::string& hip)
   {
      const glm::vec3 fiction_pos = m_starfield_universe.get_position_by_name(fictional_name);
      const glm::vec3 real_pos = m_real_universe.get_star_by_cat_id(hip).m_position;
      const float dist = glm::distance(fiction_pos, real_pos);
      // fmt::print("{:<16} deviation: {:>5.2f} LY\n", fictional_name, dist);
   };

   for (const sfn::system& system : m_starfield_universe.m_systems)
   {
      if (system.m_astronomic_name.empty() || system.m_astronomic_name == "Sol")
         continue;
      error_report(system.m_astronomic_name, system.m_catalog_lookup);
   }


   m_starfield_universe.m_cam_info = get_and_delete_cam_info(m_starfield_universe.m_systems);
   m_starfield_universe.m_trafo = final_transformation;
   m_starfield_universe.m_map_bb = old_coord_bb;
   m_starfield_universe.m_left_bb = get_unexplored_bb(old_coord_bb, m_starfield_universe.get_position_by_name("SOL"));;
   m_starfield_universe.init();

   // {
   //    const float sufficient_jump_range = get_absolute_min_jump_range(starfield_universe);
   //    fmt::print("sufficient_jump_range: {:.1f}\n", sufficient_jump_range);
   // }

   // {
   //    const auto closest = get_closest_distances_for_all(starfield_universe);
   //    fmt::print("avg of closest: {:.2f}\n", get_average(closest));
   //    fmt::print("max of closest: {:.2f}\n", *std::ranges::max_element(closest));
   // }

   m_starfield_universe.print_info();
   return m_starfield_universe;
}


auto std::hash<catalog_id>::operator()(
   sfn::catalog_id const& cat_id
   ) const noexcept -> std::size_t
{
   return std::hash<std::string>()(cat_id.m_string_cache);
}
