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

#ifndef MANDELLANDSCAPE_TEST_UTILS_HPP
#define MANDELLANDSCAPE_TEST_UTILS_HPP

#include <iostream>

#include <catch2/catch.hpp>
#include <glm/glm.hpp>

#include "player.hpp"

struct Dvec2Approx {
    glm::dvec2 val;
};

bool
operator==(glm::dvec2 a, Dvec2Approx b)
{
    return a.x == Approx(b.val.x) && a.y == Approx(b.val.y);
}

struct Dvec3Approx {
    glm::dvec3 val;
};

bool
operator==(glm::dvec3 a, Dvec3Approx b)
{
    return a.x == Approx(b.val.x) && a.y == Approx(b.val.y)
           && a.z == Approx(b.val.z);
}
// namespace glm needed for ADL
namespace glm {
std::ostream&
operator<<(std::ostream& os, glm::dvec2 const& v)
{
    os << '(' << v.x << ", " << v.y << ')';
    return os;
}
std::ostream&
operator<<(std::ostream& os, glm::dvec3 const& v)
{
    os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return os;
}
}    // namespace glm

std::ostream&
operator<<(std::ostream& os, Player const& player)
{
    os << "Player(" << player.position << ", " << player.offset << ", "
       << player.lookAtOffset << ", " << player.scale << ")";
    return os;
}

struct PlayerApprox {
    Player player;
};

bool
operator==(Player const& a, PlayerApprox const& b)
{
    return a.position == Dvec3Approx{b.player.position}
           && a.offset == Dvec3Approx{b.player.offset}
           && a.lookAtOffset == Dvec2Approx{b.player.lookAtOffset}
           && a.scale == Approx{b.player.scale};
}

struct UniformControllerApprox {
    UniformController uc;
};

bool
operator==(UniformController const& a, UniformControllerApprox const& b)
{
    glm::dvec3 m_colorOffset = {0.0, 1.0, 2.0};
    double m_colorFrequency  = 0.1;
    double m_yScale          = 1.0;
    bool m_fastMode          = false;
    return a.m_colorOffset == Dvec3Approx{b.uc.m_colorOffset}
           && a.m_colorFrequency == Approx{b.uc.m_colorFrequency}
           && a.m_yScale == Approx{b.uc.m_yScale}
           && a.m_fastMode == b.uc.m_fastMode;
}
#endif
