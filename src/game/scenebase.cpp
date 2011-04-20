/*
 * Copyright 2011 Marshmallow Engine. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *      conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *      of conditions and the following disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY MARSHMALLOW ENGINE ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MARSHMALLOW ENGINE OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Marshmallow Engine.
 */

#include "game/scenebase.h"

/*!
 * @file
 *
 * @author Guillermo A. Amaral B. (gamaral) <g@maral.me>
 */

#include "core/irenderable.h"
#include "core/iupdateable.h"
#include "event/eventmanager.h"
#include "game/engine.h"
#include "game/ientity.h"

MARSHMALLOW_NAMESPACE_USE;
using namespace Core;
using namespace Game;

Type SceneBase::Type("Game::SceneBase");

SceneBase::SceneBase(const Core::Identifier &i)
    : m_id(i)
{
}

SceneBase::~SceneBase(void)
{
	m_entities.clear();
}

void
SceneBase::addEntity(SharedEntity &e)
{
	m_entities.push_back(e);

	/* TODO: send entity creation message */
}

void
SceneBase::removeEntity(const SharedEntity &e)
{
	/* TODO: send entity removal message */

	m_entities.remove(e);
}

SharedEntity
SceneBase::entity(const Core::Identifier &i) const
{
	EntityList::const_iterator l_i;
	EntityList::const_iterator l_c = m_entities.end();

	/* maybe replace later with a map if required */
	for (l_i = m_entities.begin(); l_i != l_c; ++l_i) {
		if ((*l_i)->id() == i)
			return(*l_i);
	}

	return(SharedEntity());
}

const EntityList &
SceneBase::entities(void) const
{
	return(m_entities);
}

void
SceneBase::activate(void)
{
}

void
SceneBase::deactivate(void)
{
}

void
SceneBase::render(void)
{
	EntityList::const_iterator l_i;

	for (l_i = m_entities.begin(); l_i != m_entities.end();l_i++)
		if (!(*l_i)->isZombie()) (*l_i)->render();
}

void
SceneBase::update(TIME d)
{
	EntityList::const_iterator l_i;

	for (l_i = m_entities.begin(); l_i != m_entities.end();) {
		SharedEntity l_entity = (*l_i++);

		if (l_entity->isZombie())
			removeEntity(l_entity);
		else
			l_entity->update(d);
	}
}
