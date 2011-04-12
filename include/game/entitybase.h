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

#pragma once

/*!
 * @file
 *
 * @author Guillermo A. Amaral B. (gamaral) <g@maral.me>
 */

#ifndef GAME_ENTITYBASE_H
#define GAME_ENTITYBASE_H 1

#include "game/ientity.h"

#include "EASTL/list.h"
using namespace eastl;

MARSHMALLOW_NAMESPACE_BEGIN

namespace Game
{

	/*! @brief Entity Base Class */
	class GAME_EXPORT EntityBase : public IEntity
	{
		typedef list<SharedComponent> ComponentList;

		ComponentList m_components;
		Core::Identifier m_id;
		bool m_killed;

	public:
		EntityBase(const Core::Identifier &identifier);
		virtual ~EntityBase(void);

	public: /* virtual */

		VIRTUAL const Core::Identifier & id(void) const
		    { return(m_id); }

		VIRTUAL const Core::Type & type(void) const
		    { return(Type); }

		VIRTUAL void addComponent(SharedComponent &component);
		VIRTUAL void removeComponent(const SharedComponent &component);
		VIRTUAL SharedComponent component(const Core::Identifier &identifier) const;

		VIRTUAL void update(void);

		VIRTUAL void kill(void)
		    { m_killed = true; }
		VIRTUAL bool isZombie(void) const
		    { return(m_killed); }

	public: /* static */

		static const Core::Type Type;
	};

}

MARSHMALLOW_NAMESPACE_END

#endif
