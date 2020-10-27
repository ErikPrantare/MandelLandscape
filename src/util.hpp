#ifndef MANDELLANDSCAPE_UTILS_HPP
#define MANDELLANDSCAPE_UTILS_HPP

#include <string>
#include <fstream>
#include <future>
#include <cmath>
#include <map>
#include <set>
#include <queue>
#include <optional>

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "mandelTypeTraits.hpp"

namespace util {

double constexpr pi = glm::pi<double>();

std::string
readFile(std::string const& filePath);

// CPP20 https://en.cppreference.com/w/cpp/experimental/future/is_ready
template<typename T>
bool
isDone(std::future<T> const& f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<typename T>
class LowPassFilter {
public:
    LowPassFilter(T init, double amount) :
                m_filteredValue(init),
                m_amount(amount){};

    T
    operator()(T const newValue, double const weight = 1.0f)
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
bool
contains(std::map<KeyType, ValueType> const& map, KeyType key)
{
    return map.find(key) != map.end();
}

// CPP20 https://en.cppreference.com/w/cpp/container/set/contains
template<typename ValueType>
bool
contains(std::set<ValueType> const& set, ValueType key)
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
std::optional<T>
pop(std::queue<T, Container>& queue)
{
    if(queue.empty()) {
        return std::nullopt;
    }

    auto a = queue.front();
    queue.pop();
    return a;
}

glm::dvec2 constexpr pixelsToAngle(
        glm::dvec2 nrPixels,
        double sensitivity = 0.01)
{
    //-x, refer to right hand rule with y up and positive pixels rightwards
    return sensitivity * glm::dvec2(-nrPixels.x, nrPixels.y);
}

glm::dvec2 constexpr planePos(glm::dvec3 spacePos)
{
    return {spacePos.x, spacePos.z};
}

// CPP20 Make constexpr
glm::dvec2
unitVec2(double theta);

}    // namespace util

#endif    // MANDELLANDSCAPE_UTILS_HPP