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

#include "graphics/viewport_p.h"

/*!
 * @file
 *
 * @author Guillermo A. Amaral B. (gamaral) <g@maral.me>
 */

#include <bps/bps.h>
#include <bps/navigator.h>
#include <bps/screen.h>

#include <screen/screen.h>

#include <cassert>
#include <cstring>

#include "core/logger.h"
#include "core/platform.h"
#include "core/type.h"

#include "event/eventmanager.h"
#include "event/keyboardevent.h"
#include "event/quitevent.h"
#include "event/viewportevent.h"

#ifdef MARSHMALLOW_INPUT_QNX_SCREEN
#  include "input/qnx/screen.h"
#endif

#include "graphics/camera.h"
#include "graphics/color.h"
#include "graphics/display.h"
#include "graphics/painter_p.h"


#include "headers.h"
#ifdef MARSHMALLOW_OPENGL_EGL
#  include <EGL/egl.h>
#else
#  error Building without EGL support!
#endif
#ifndef MARSHMALLOW_OPENGL_ES2
#  error Building without OpenGL ES2 support!
#endif
#include "extensions.h"


/*
 * QNX Viewport Notes
 *
 * The fullscreen flag in the Display parameters is ignored since we will always
 * be in fullscreen.
 */

MARSHMALLOW_NAMESPACE_BEGIN
namespace Graphics { /************************************ Graphics Namespace */
namespace OpenGL { /****************************** Graphics::OpenGL Namespace */
namespace { /************************ Graphics::OpenGL::<anonymous> Namespace */

namespace QNXViewport {

	inline bool Initialize(void);
	inline void Finalize(void);

	inline void Reset(int state);
	inline void Tick(float delta);

	inline bool Create(const Display &display);
	inline void Destroy(void);

	inline void SwapBuffer(void);

	inline bool CreateGLContext(void);
	inline void DestroyGLContext(void);

	inline bool CreateScreenWindow(void);
	inline void DestroyScreenWindow(void);

	enum StateFlag
	{
		sfUninitialized = 0,

		sfReady         = (1 <<  0),
		sfBPS           = (1 <<  1),

		sfScreenContext = (1 <<  2),
		sfScreenWindow  = (1 <<  3),
		sfScreenDisplay = (1 <<  4),

		sfGLDisplay     = (1 <<  5),
		sfGLSurface     = (1 <<  6),
		sfGLContext     = (1 <<  7),
		sfGLCurrent     = (1 <<  8),

		sfActive        = (1 <<  9),
		sfReset         = (1 << 10),
		sfTerminated    = (1 << 11),

		sfScreenValid   = sfScreenContext|sfScreenWindow|sfScreenDisplay,
		sfGLValid       = sfGLDisplay|sfGLSurface|sfGLContext|sfGLCurrent,
		sfValid         = sfBPS|sfScreenValid|sfGLValid,
		sfReadyValid    = sfReady|sfValid,
		sfActiveReadyValid = sfActive|sfReadyValid
	};

	/************************* MARSHMALLOW */
	Display                   dpy;
	Math::Size2i              wsize;
	Math::Size2f              vsize;
	int                       flags;

	/************************** QNX Screen */
	screen_context_t          scrn_ctx;
	screen_display_t          scrn_dpy;
	screen_window_t           scrn_window;

	/********************************* EGL */
	EGLDisplay                egl_dpy;
	EGLSurface                egl_surface;
	EGLContext                egl_ctx;

}

bool
QNXViewport::Initialize(void)
{
	/* default display display */
	dpy.depth      = MARSHMALLOW_VIEWPORT_DEPTH;
	dpy.fullscreen = MARSHMALLOW_VIEWPORT_FULLSCREEN;
	dpy.height     = MARSHMALLOW_VIEWPORT_HEIGHT;
	dpy.width      = MARSHMALLOW_VIEWPORT_WIDTH;
	dpy.vsync      = MARSHMALLOW_VIEWPORT_VSYNC;

	Reset(sfUninitialized);

	/*
	 * BlackBerry Platform Services (BPS)
	 */
	bps_initialize();
	navigator_request_events(0);
	flags |= sfBPS;

	/*
	 * Initial Camera Reset (IMPORTANT)
	 */
	Camera::Reset();

	/*
	 * Initial Background Color (IMPORTANT)
	 */
	Painter::SetBackgroundColor(Color::Black());

	return(true);
}

void
QNXViewport::Finalize(void)
{
	/* set termination flag */
	flags |= sfTerminated;

	/*
	 * Destroy viewport if valid
	 */
	if (sfValid == (flags & sfValid))
		Destroy();

	/*
	 * BlackBerry Platform Services (BPS)
	 */
	if (sfBPS == (flags & sfBPS)) {
		bps_shutdown();
		flags ^= sfBPS;
	}

	/* sanity check */
	assert(flags == sfTerminated && "We seem to have some stray flags!");

	flags = sfUninitialized;
}

void
QNXViewport::Reset(int state)
{
	flags = state;
	vsize.zero();
	wsize.zero();

	scrn_dpy = 0;
	scrn_window = 0;
	scrn_ctx = 0;

	egl_dpy     = EGL_NO_DISPLAY;
	egl_surface = EGL_NO_SURFACE;
	egl_ctx     = EGL_NO_CONTEXT;
}

void
QNXViewport::Tick(float delta)
{
	MMUNUSED(delta);

	bool l_redisplay = false;
	bps_event_t *l_event = 0;
	while ((bps_get_event(&l_event, 0) == BPS_SUCCESS) && l_event) {

		const int l_domain = bps_event_get_domain(l_event);
		const int l_evcode = bps_event_get_code(l_event);

		/*
		 * Navigator Events
		 */
		if (l_domain == navigator_get_domain()) {
			switch (l_evcode) {
			case NAVIGATOR_EXIT: {
				Event::QuitEvent l_mmevent(-1);
				Event::EventManager::Instance()->dispatch(l_mmevent);
				} break;

			case NAVIGATOR_WINDOW_ACTIVE:
			case NAVIGATOR_WINDOW_FULLSCREEN:
				flags |= sfActive;
				break;

			case NAVIGATOR_WINDOW_INACTIVE:
				flags &= ~(sfActive);
				break;
			}
		}

		/*
		 * Screen Events
		 */
		else if (l_domain == screen_get_domain()) {

			screen_event_t l_screen_event =
			    screen_event_get_event(l_event);

			int l_type = 0;
			screen_get_event_property_iv(l_screen_event,
			    SCREEN_PROPERTY_TYPE, &l_type);

			switch(l_type) {
			case SCREEN_EVENT_DISPLAY:
				l_redisplay = true;
				break;

#ifdef MARSHMALLOW_INPUT_QNX_SCREEN
			case SCREEN_EVENT_KEYBOARD:
			case SCREEN_EVENT_POINTER:
				Input::QNX::Screen::HandleEvent(l_type, l_screen_event);
				break;
			}
#endif
		}
	}

	/*
	 * Redisplay Viewport
	 */
	if (l_redisplay)
		Viewport::Setup(dpy);
}

bool
QNXViewport::Create(const Display &display_)
{
	/*
	 * Check if already valid (no no)
	 */
	if (sfValid == (flags & sfValid))
		return(false);

	/* assign new display display */
	dpy = display_;

	/*
	 * Create QNX Screen Window
	 */
	if (!CreateScreenWindow()) {
		MMERROR("QNX: Failed to create window.");
		return(false);
	}

	/*
	 * Create EGL Context
	 */
	if (!CreateGLContext()) {
		MMERROR("GL: Failed to create surface.");
		DestroyScreenWindow();
		return(false);
	}

	/* sanity check */
	assert(sfValid == (flags & sfValid)
	    && "Valid viewport was expected!");

	/* initialize context */

	glViewport(0, 0, wsize.width, wsize.height);

	if (glGetError() != GL_NO_ERROR) {
		MMERROR("GL: Failed during initialization.");
		DestroyGLContext();
		DestroyScreenWindow();
		return(false);
	}

	/* viewport size */

#if MARSHMALLOW_VIEWPORT_LOCK_WIDTH
	vsize.width = static_cast<float>(dpy.width);
	vsize.height = (vsize.width * static_cast<float>(wsize.height)) /
	    static_cast<float>(wsize.width);
#else
	vsize.height = static_cast<float>(dpy.height);
	vsize.width = (vsize.height * static_cast<float>(wsize.width)) /
	    static_cast<float>(wsize.height);
#endif

	/* sub-systems */
	Camera::Update();
	Painter::Initialize();

	/* broadcast */
	Event::ViewportEvent l_event(Event::ViewportEvent::Created);
	Event::EventManager::Instance()->dispatch(l_event);

	/* viewport ready */
	flags |= sfActive|sfReady;

	return(true);
}

void
QNXViewport::Destroy(void)
{
	/* check for valid state */
	if (sfValid != (flags & sfValid))
		return;

	/* deactivate */
	flags &= ~(sfActive|sfReady);

	/* broadcast */
	if (0 == (flags & sfTerminated)) {
		Event::ViewportEvent l_event(Event::ViewportEvent::Destroyed);
		Event::EventManager::Instance()->dispatch(l_event);
	}

	Painter::Finalize();

	DestroyGLContext();
	DestroyScreenWindow();

	/* sanity check */
	assert(0 == (flags & ~(sfBPS|sfReady|sfTerminated))
	    && "We seem to have some stray flags!");

	Reset(flags & (sfBPS|sfTerminated));
}

void
QNXViewport::SwapBuffer(void)
{
	if (sfValid != (flags & sfValid))
		return;

	eglSwapBuffers(egl_dpy, egl_surface);
}

bool
QNXViewport::CreateGLContext(void)
{
	/*
	 * Open EGL Display
	 */
	egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (egl_dpy == EGL_NO_DISPLAY) {
		MMERROR("EGL: No display was found.");
		return(false);
	}
	flags |= sfGLDisplay;

	if (eglInitialize(egl_dpy, 0, 0) == EGL_FALSE) {
		MMERROR("EGL: Initialization failed.");
		return(false);
	}

	/*
	 * Bind OpenGL API
	 */
	if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
		MMERROR("EGL: Failed to bind required OpenGL API.");
		return(false);
	}

	/*
	 * Choose EGL Config
	 */
	EGLint l_attr[] = {
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BUFFER_SIZE,     dpy.depth,
		EGL_DEPTH_SIZE,      0,
		EGL_RED_SIZE,        8,
		EGL_GREEN_SIZE,      8,
		EGL_BLUE_SIZE,       8,
		EGL_ALPHA_SIZE,      0,
		EGL_NONE,            EGL_NONE
	};
	EGLint l_config_count;
	EGLConfig l_config;
	if (!eglChooseConfig(egl_dpy, l_attr, &l_config, 1, &l_config_count))
		MMERROR("EGL: No config was chosen. EGLERROR=" << eglGetError());

	/*
	 * Create EGL Surface
	 */
	egl_surface = eglCreateWindowSurface(egl_dpy,
	                                     l_config,
	                                     scrn_window,
	                                     0);
	if (egl_surface == EGL_NO_SURFACE) {
		MMERROR("EGL: No surface was created. EGLERROR=" << eglGetError());
		return(false);
	}
	flags |= sfGLSurface;

	/*
	 * Create EGL Context
	 */
	EGLint l_ctxattr[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,                   EGL_NONE
	};
	egl_ctx = eglCreateContext(egl_dpy, l_config, EGL_NO_CONTEXT, l_ctxattr);
	if (egl_ctx == EGL_NO_CONTEXT) {
		MMERROR("EGL: No context was created. EGLERROR=" << eglGetError());
		return(false);
	}
	flags |= sfGLContext;

	/*
	 * Make Current EGL Context
	 */
	if (!eglMakeCurrent(egl_dpy, egl_surface, egl_surface, egl_ctx)) {
		MMERROR("EGL: Failed to switch current context. EGLERROR=" << eglGetError());
		return(false);
	}
	flags |= sfGLCurrent;

	/* extensions */

	Extensions::Initialize(eglQueryString(egl_dpy, EGL_EXTENSIONS));

	/* vsync */

	if (eglSwapInterval(egl_dpy, dpy.vsync) != EGL_TRUE)
		MMERROR("EGL: Swap interval request was ignored!");

	/* clear error state */

	eglGetError();

	return(true);
}

void
QNXViewport::DestroyGLContext(void)
{
	if (0 == (flags & sfGLValid))
		return;

	/* extensions */

	Extensions::Finalize();

	/*
	 * Clear Current EGL Context
	 */
	if (sfGLCurrent == (flags & sfGLCurrent)) {
		eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
		               EGL_NO_CONTEXT);
		flags ^= sfGLCurrent;
	}

	/*
	 * Destroy EGL Context
	 */
	if (sfGLContext == (flags & sfGLContext)) {
		eglDestroyContext(egl_dpy, egl_ctx),
		    egl_ctx = EGL_NO_CONTEXT;
		flags ^= sfGLContext;
	}

	/*
	 * Destroy EGL Surface
	 */
	if (sfGLSurface == (flags & sfGLSurface)) {
		eglDestroySurface(egl_dpy, egl_surface),
		    egl_surface = EGL_NO_SURFACE;
		flags ^= sfGLSurface;
	}

	/*
	 * Close EGL Display
	 */
	if (sfGLDisplay == (flags & sfGLDisplay)) {
		eglTerminate(egl_dpy), egl_dpy = EGL_NO_DISPLAY;
		flags ^= sfGLDisplay;
	}
}

bool
QNXViewport::CreateScreenWindow(void)
{
	/*
	 * QNX Screen App Context
	 */
	if (screen_create_context(&scrn_ctx, SCREEN_APPLICATION_CONTEXT) != 0) {
		perror("screen_create_context");
		return(false);
	}
	screen_request_events(scrn_ctx);
	flags |= sfScreenContext;

	/*
	 * QNX Screen Window
	 */
	if (screen_create_window(&scrn_window, scrn_ctx)) {
		perror("screen_create_window");
		return(false);
	}
	flags |= sfScreenWindow;

	const int l_format = SCREEN_FORMAT_RGBX8888;
	if (screen_set_window_property_iv(scrn_window,
	    SCREEN_PROPERTY_FORMAT, &l_format)) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
		return(false);
	}

	const int l_usage = SCREEN_USAGE_OPENGL_ES2;
	if (screen_set_window_property_iv(scrn_window,
	    SCREEN_PROPERTY_USAGE, &l_usage)) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
		return(false);
	}

	const int l_sensitivity = SCREEN_SENSITIVITY_FULLSCREEN;
	if (screen_set_window_property_iv(scrn_window,
	    SCREEN_PROPERTY_SENSITIVITY, &l_sensitivity)) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_SENSITIVITY)");
		return(false);
	}

	if (screen_get_window_property_pv(scrn_window,
	    SCREEN_PROPERTY_DISPLAY, reinterpret_cast<void **>(&scrn_dpy))) {
		perror("screen_get_window_property_pv");
		return(false);
	}

	/*
	 * Create window buffers (IMPORTANT)
	 */
	if (screen_create_window_buffers(scrn_window, 2)) {
		perror("screen_create_window_buffers");
		return(false);
	}

	screen_display_mode_t l_screen_mode;
	if (screen_get_display_property_pv(scrn_dpy,
	    SCREEN_PROPERTY_MODE, reinterpret_cast<void**>(&l_screen_mode))) {
		perror("screen_get_display_property_pv");
		return(false);
	}
	flags |= sfScreenDisplay;

	wsize.set(l_screen_mode.width, l_screen_mode.height);

	MMDEBUG("QNX: Display size (" << wsize.width << "x" << wsize.height << ") ...");

	return(true);
}

void
QNXViewport::DestroyScreenWindow(void)
{
	/* clear screen display */
	scrn_dpy = 0;
	flags &= ~(sfScreenDisplay);

	/* destroy screen window */
	if (sfScreenWindow == (flags & sfScreenWindow)) {
		if (screen_destroy_window(scrn_window) != 0)
			MMERROR("QNX: Failed to destroy screen window. IGNORING.");
		scrn_window = 0;
		flags ^= sfScreenWindow;
	}

	/* destroy screen context */
	if (sfScreenContext == (flags & sfScreenContext)) {
		if (screen_destroy_context(scrn_ctx) != 0)
			MMERROR("QNX: Failed to destroy screen context. IGNORING.");

		screen_stop_events(scrn_ctx);
		scrn_ctx = 0;
		flags ^= sfScreenContext;
	}
}

} /********************************** Graphics::OpenGL::<anonymous> Namespace */

PFNPROC
glGetProcAddress(const char *f)
{
	return(eglGetProcAddress(f));
}

} /*********************************************** Graphics::OpenGL Namespace */

bool
Viewport::Active(void)
{
	using namespace OpenGL::QNXViewport;
	return(sfActiveReadyValid == (flags & sfActiveReadyValid));
}

bool
Viewport::Initialize(void)
{
	using namespace OpenGL;
	return(QNXViewport::Initialize());
}

void
Viewport::Finalize(void)
{
	using namespace OpenGL;
	QNXViewport::Finalize();
}

bool
Viewport::Setup(const Graphics::Display &display)
{
	using namespace Core;
	using namespace OpenGL;

	static bool s_working = false;
	if (s_working)
		return(false);
	s_working = true;

	QNXViewport::Destroy();

	if (!QNXViewport::Create(display)) {
		QNXViewport::Destroy();
		s_working = false;
		return(false);
	}

	s_working = false;
	return(true);
}

void
Viewport::Tick(float delta)
{
	using namespace OpenGL;
	QNXViewport::Tick(delta);
}

void
Viewport::SwapBuffer(void)
{
	using namespace OpenGL;
	QNXViewport::SwapBuffer();
	Painter::Reset();
}

const Graphics::Display &
Viewport::Display(void)
{
	using namespace OpenGL;
	return(QNXViewport::dpy);
}

const Math::Size2f &
Viewport::Size(void)
{
	using namespace OpenGL;
	return(QNXViewport::vsize);
}

const Math::Size2i &
Viewport::WindowSize(void)
{
	using namespace OpenGL;
	return(QNXViewport::wsize);
}

const Core::Type &
Viewport::Type(void)
{
	static const Core::Type s_type("QNX");
	return(s_type);
}

} /******************************************************* Graphics Namespace */
MARSHMALLOW_NAMESPACE_END

