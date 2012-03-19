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

#include "graphics/tilesetbase.h"

/*!
 * @file
 *
 * @author Guillermo A. Amaral B. (gamaral) <g@maral.me>
 */

#include "core/logger.h"

#include "math/size2.h"

#include "graphics/factory.h"
#include "graphics/itexturecoordinatedata.h"
#include "graphics/itexturedata.h"

#define TILE_ADJUSTMENT 0.00004f

MARSHMALLOW_NAMESPACE_USE
using namespace Graphics;

struct TilesetBase::Private
{
	SharedTextureCoordinateData **cache;

	Core::Identifier  name;
	Math::Size2i      size;
	Math::Size2i      tile_size;
	SharedTextureData texture_data;

	int margin;
	int spacing;

	float *offset_col;
	float *offset_row;
	float  spacing_col;
	float  spacing_row;
};

TilesetBase::TilesetBase()
    : m_p(new Private)
{
	m_p->cache = 0;
	m_p->tile_size[0] = 16;
	m_p->tile_size[1] = 16;
	m_p->margin = 0;
	m_p->spacing = 0;
	m_p->offset_col = 0;
	m_p->offset_row = 0;
	m_p->spacing_col = 0;
	m_p->spacing_row = 0;
}

TilesetBase::~TilesetBase(void)
{
	m_p->texture_data.clear();

	/* reset will clear out cache data and return */
	reset();

	delete m_p;
	m_p = 0;
}

void
TilesetBase::setName(const Core::Identifier &n)
{
	m_p->name = n;
}

void
TilesetBase::setTileSize(const Math::Size2i &s)
{
	m_p->tile_size = s;
	reset();
}

void
TilesetBase::setMargin(int m)
{
	m_p->margin = m;
	reset();
}

void
TilesetBase::setSpacing(int s)
{
	m_p->spacing = s;
	reset();
}

const Core::Identifier &
TilesetBase::name(void) const
{
	return(m_p->name);
}

const Math::Size2i &
TilesetBase::size(void) const
{
	return(m_p->size);
}

const SharedTextureData &
TilesetBase::textureData(void) const
{
	return(m_p->texture_data);
}

const Math::Size2i &
TilesetBase::tileSize(void) const
{
	return(m_p->tile_size);
}

int
TilesetBase::spacing(void) const
{
	return(m_p->spacing);
}

int
TilesetBase::margin(void) const
{
	return(m_p->margin);
}

void
TilesetBase::setTextureData(const SharedTextureData &ts)
{
	if (ts == m_p->texture_data)
		return;

	/* replace texture */
	m_p->texture_data = ts;

	reset();
}

SharedTextureCoordinateData
TilesetBase::getTextureCoordinateData(UINT16 i)
{
	if (!m_p->texture_data || !m_p->texture_data->isLoaded())
		return(SharedTextureCoordinateData());

	assert(i < m_p->size.area() && "index was out of bounds.");
	SharedTextureCoordinateData *l_cached = m_p->cache[i];
	if (l_cached == 0) {
		/* create new entry */
#define TILECOORDINATES 4
		SharedTextureCoordinateData l_data =
		    Factory::CreateTextureCoordinateData(TILECOORDINATES);

		/* calculate row and column */

		const int l_row = i / m_p->size.width();
		const int l_col = i % m_p->size.width();

		const float l_left   = m_p->offset_col[l_col] + TILE_ADJUSTMENT;
		const float l_top    = m_p->offset_row[l_row] + TILE_ADJUSTMENT;
		const float l_right  = m_p->offset_col[l_col + 1] - m_p->spacing_col - TILE_ADJUSTMENT;
		const float l_bottom = m_p->offset_row[l_row + 1] - m_p->spacing_row - TILE_ADJUSTMENT;

		l_data->set(0, l_left,  l_top);
		l_data->set(1, l_left,  l_bottom);
		l_data->set(2, l_right, l_top);
		l_data->set(3, l_right, l_bottom);

		m_p->cache[i] = new SharedTextureCoordinateData(l_data);
		return(l_data);
	}

	return(*m_p->cache[i]);
}

bool
TilesetBase::serialize(TinyXML::TiXmlElement &n) const
{
	MMUNUSED(n);

	/* TODO: Implement */

	return(false);
}

bool
TilesetBase::deserialize(TinyXML::TiXmlElement &n)
{
	MMUNUSED(n);

	/* TODO: Implement */

	return(false);
}

void
TilesetBase::reset(void)
{
	/* clear current tile cache */

	const int l_old_item_count = m_p->size.area();
	for (int i = 0; i < l_old_item_count; ++i)
		delete m_p->cache[i];
	delete[] m_p->cache;
	m_p->cache = 0;

	delete[] m_p->offset_col;
	delete[] m_p->offset_row;

	m_p->offset_col  = 0;
	m_p->offset_row  = 0;
	m_p->spacing_col = 0;
	m_p->spacing_row = 0;
	m_p->size.zero();

	if (!m_p->texture_data)
		return;

	/* calculate size */

	const Math::Size2i &l_texture_size = m_p->texture_data->size();
	m_p->size[0] = (l_texture_size.width() - m_p->margin + m_p->spacing)
	          / (m_p->tile_size.width() + m_p->spacing);
	m_p->size[1] = (l_texture_size.height() - m_p->margin + m_p->spacing)
	          / (m_p->tile_size.height() + m_p->spacing);

	/*
	 * we generate the tile offsets, the extra offset (+1) acts as the limit
	 * for the last row/column.
	 */

	m_p->offset_col = new float[m_p->size.width() + 1];
	m_p->spacing_col = static_cast<float>(m_p->spacing)
	              / static_cast<float>(l_texture_size.width());
	for (int i = m_p->size.width(); i >= 0; --i)
		m_p->offset_col[i] =
		    static_cast<float>(m_p->margin + (i * (m_p->tile_size.width() + m_p->spacing)))
		  / static_cast<float>(l_texture_size.width());

	m_p->offset_row = new float[m_p->size.height() + 1];
	m_p->spacing_row = static_cast<float>(m_p->spacing)
	              / static_cast<float>(l_texture_size.height());
	for (int i = m_p->size.height(); i >= 0; --i)
		m_p->offset_row[i] =
		    static_cast<float>(m_p->margin + (i * (m_p->tile_size.height() + m_p->spacing)))
		  / static_cast<float>(l_texture_size.height());

	const int l_item_count = m_p->size.area();
	m_p->cache = new SharedTextureCoordinateData*[l_item_count];
	for (int i = 0; i < l_item_count; ++i)
		m_p->cache[i] = 0;
}

