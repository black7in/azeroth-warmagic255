/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EscortMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Errors.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "Unit.h"
#include "World.h"

template<class T>
void EscortMovementGenerator<T>::DoInitialize(T* unit)
{
    if (!unit->IsStopped())
        unit->StopMoving();

    unit->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    _recalculateSpeed = false;
    Movement::MoveSplineInit init(unit);

    if (_precomputedPath.size() == 2) // xinef: simple case, just call move to
        init.MoveTo(_precomputedPath[1].x, _precomputedPath[1].y, _precomputedPath[1].z, true);
    else if (_precomputedPath.size())
        init.MovebyPath(_precomputedPath);

    init.Launch();

    _splineId = unit->movespline->GetId();
}

template<class T>
bool EscortMovementGenerator<T>::DoUpdate(T* unit, uint32  /*diff*/)
{
    if (!unit)
        return false;

    if (unit->HasUnitState(UNIT_STATE_NOT_MOVE) || unit->IsMovementPreventedByCasting())
    {
        unit->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    unit->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    bool arrived = unit->movespline->Finalized();

    if (_recalculateSpeed && !arrived)
    {
        _recalculateSpeed = false;
        Movement::MoveSplineInit init(unit);

        // xinef: speed changed during path execution, calculate remaining path and launch it once more
        if (_precomputedPath.size())
        {
            uint32 offset = std::min(uint32(unit->movespline->_currentSplineIdx()), uint32(_precomputedPath.size()));
            Movement::PointsArray::iterator offsetItr = _precomputedPath.begin();
            std::advance(offsetItr, offset);
            _precomputedPath.erase(_precomputedPath.begin(), offsetItr);

            // restore 0 element (current position)
            _precomputedPath.insert(_precomputedPath.begin(), G3D::Vector3(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ()));

            if (_precomputedPath.size() > 2)
                init.MovebyPath(_precomputedPath);
            else if (_precomputedPath.size() == 2)
                init.MoveTo(_precomputedPath[1].x, _precomputedPath[1].y, _precomputedPath[1].z, true);
        }

        init.Launch();
        // Xinef: Override spline Id on recalculate launch
        _splineId = unit->movespline->GetId();
    }

    return !arrived;
}

template<class T>
void EscortMovementGenerator<T>::DoFinalize(T* unit)
{
    unit->ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void EscortMovementGenerator<T>::DoReset(T* unit)
{
    if (!unit->IsStopped())
        unit->StopMoving();

    unit->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
}

template void EscortMovementGenerator<Player>::DoInitialize(Player*);
template void EscortMovementGenerator<Creature>::DoInitialize(Creature*);
template void EscortMovementGenerator<Player>::DoFinalize(Player*);
template void EscortMovementGenerator<Creature>::DoFinalize(Creature*);
template void EscortMovementGenerator<Player>::DoReset(Player*);
template void EscortMovementGenerator<Creature>::DoReset(Creature*);
template bool EscortMovementGenerator<Player>::DoUpdate(Player* unit, uint32 diff);
template bool EscortMovementGenerator<Creature>::DoUpdate(Creature* unit, uint32 diff);
