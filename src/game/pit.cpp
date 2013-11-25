// This file is part of Dust Racing 2D.
// Copyright (C) 2013 Jussi Lind <jussi.lind@iki.fi>
//
// Dust Racing 2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// Dust Racing 2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dust Racing 2D. If not, see <http://www.gnu.org/licenses/>.

#include "pit.hpp"
#include "car.hpp"

#include <MCCollisionEvent>
#include <MCSurface>

Pit::Pit(MCSurface & surface)
: MCObject(surface, "Pit")
, m_tag(0)
{
}

void Pit::collisionEvent(MCCollisionEvent & event)
{
    // Cache type id integers.
    static MCUint carType = MCObject::typeID("Car");

    // Check if the car is colliding with another car.
    if (event.collidingObject().typeID() == carType)
    {
        Car & car = static_cast<Car &>(event.collidingObject());
        if (car.isHuman() && car.speedInKmh() < 25)
        {
            if (m_pittingCars.find(&car) == m_pittingCars.end())
            {
                emit pitStop(car);
            }

            m_pittingCars[&car] = m_tag;
        }
    }
}

void Pit::stepTime(MCFloat)
{
    auto i = m_pittingCars.begin();
    while (i != m_pittingCars.end())
    {
        if (i->second + 1 < m_tag)
        {
            i = m_pittingCars.erase(i);
        }
        else
        {
            i++;
        }
    }

    m_tag++;
}