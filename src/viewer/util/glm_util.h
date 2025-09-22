#pragma once

#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

template<glm::length_t L, typename T, glm::qualifier Q>
nlohmann::json to_json_array(const glm::vec<L, T, Q>& v)
{
    auto a = nlohmann::json::array();
    for (glm::length_t i : vi::iota(0, v.length())) {
        a.push_back(v[i]);
    }
    return a;
}
