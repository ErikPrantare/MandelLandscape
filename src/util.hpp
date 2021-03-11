/* Mandelscape
 * Copyright (C) 2020-2021 Erik Präntare
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANDELLANDSCAPE_UTILS_HPP
#define MANDELLANDSCAPE_UTILS_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <future>
#include <cmath>
#include <map>
#include <set>
#include <queue>
#include <optional>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "mandelTypeTraits.hpp"

namespace util {

// CPP20 std::numbers::pi
double constexpr pi = glm::pi<double>();

auto
readFile(std::string const& filePath) -> std::string;

template<
        class... Args,
        class = std::enable_if_t<
                std::conjunction_v<std::is_convertible<Args, std::string>...>>>
auto
concatFiles(Args const&... files) -> std::string
{
    std::string contents;
    return (contents.append(readFile(files)), ...);
}

// CPP20 https://en.cppreference.com/w/cpp/experimental/future/is_ready
template<typename T>
auto
isDone(std::future<T> const& f) -> bool
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<typename T>
class LowPassFilter {
public:
    LowPassFilter(T init, double amount) :
                m_filteredValue(init),
                m_amount(amount){};

    auto
    operator()(T const newValue, double const weight = 1.0f) -> T
    {
        double const factor = std::pow(m_amount, weight);

        m_filteredValue = factor * m_filteredValue + (1.0 - factor) * newValue;
        return m_filteredValue;
    }

private:
    T m_filteredValue;
    double m_amount;
};

// CPP20 https://en.cppreference.com/w/cpp/container/map/contains
template<typename KeyType, typename ValueType>
auto
contains(std::map<KeyType, ValueType> const& map, KeyType key) -> bool
{
    return map.find(key) != map.end();
}

// CPP20 https://en.cppreference.com/w/cpp/container/set/contains
template<typename ValueType>
auto
contains(std::set<ValueType> const& set, ValueType key) -> bool
{
    return set.find(key) != set.end();
}

template<typename... Callables>
struct Overload : Callables... {
    using Callables::operator()...;
};

// CPP20
// https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
template<typename... Callables>
Overload(Callables...) -> Overload<Callables...>;

auto constexpr unaryNOP = [](auto&&) {
};

template<typename T, typename Container>
auto
pop(std::queue<T, Container>& queue) -> std::optional<T>
{
    if(queue.empty()) {
        return std::nullopt;
    }

    auto a = queue.front();
    queue.pop();
    return a;
}

auto constexpr pixelsToAngle(glm::dvec2 nrPixels, double sensitivity = 0.01)
        -> glm::dvec2
{
    //-x, refer to right hand rule with y up and positive pixels rightwards
    return sensitivity * glm::dvec2(-nrPixels.x, nrPixels.y);
}

auto constexpr planePos(glm::dvec3 spacePos) -> glm::dvec2
{
    return {spacePos.x, spacePos.z};
}

// CPP20 Make constexpr
auto
unitVec2(double theta) -> glm::dvec2;

}    // namespace util

#endif    // MANDELLANDSCAPE_UTILS_HPP
