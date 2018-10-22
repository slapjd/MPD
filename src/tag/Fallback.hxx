/*
 * Copyright 2003-2018 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPD_TAG_FALLBACK_HXX
#define MPD_TAG_FALLBACK_HXX

#include <utility>

template<typename F>
bool
ApplyTagFallback(TagType type, F &&f) noexcept
{
	if (type == TAG_ALBUM_ARTIST)
		/* fall back to "Artist" if no "AlbumArtist" was found */
		return f(TAG_ARTIST);

	return false;
}

template<typename F>
bool
ApplyTagWithFallback(TagType type, F &&f) noexcept
{
	return f(type) || ApplyTagFallback(type, std::forward<F>(f));
}

#endif
