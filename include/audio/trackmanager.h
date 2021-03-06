/*
 * Copyright 2011-2012 Marshmallow Engine. All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO AUDIO SHALL MARSHMALLOW ENGINE OR
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

#ifndef MARSHMALLOW_AUDIO_TRACKMANAGER_H
#define MARSHMALLOW_AUDIO_TRACKMANAGER_H 1

#include <core/environment.h>
#include <core/fd.h>
#include <core/global.h>

MARSHMALLOW_NAMESPACE_BEGIN

namespace Audio { /****************************************** Audio Namespace */

	struct ITrack;
	typedef Core::Shared<ITrack> SharedTrack;

	/*! @brief Track Manager */
	class MARSHMALLOW_AUDIO_EXPORT
	TrackManager
	{
		struct Private;
		Private *m_p;

		NO_ASSIGN_COPY(TrackManager);
	public:

		TrackManager(void);
		virtual ~TrackManager(void);

	public: /* virtual */

	public: /* static */

		static TrackManager *Instance(void);
	};
	typedef Core::Shared<TrackManager> SharedTrackManager;
	typedef Core::Weak<TrackManager> WeakTrackManager;

} /********************************************************** Audio Namespace */
MARSHMALLOW_NAMESPACE_END

#endif
