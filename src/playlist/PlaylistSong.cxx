// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "PlaylistSong.hxx"
#include "SongLoader.hxx"
#include "tag/Tag.hxx"
#include "tag/Builder.hxx"
#include "fs/Traits.hxx"
#include "song/DetachedSong.hxx"
#include "util/UriExtract.hxx"
#include "util/UriUtil.hxx"

#include <algorithm>
#include <string>

#include <string.h>

static void
merge_song_metadata(DetachedSong &add, const DetachedSong &base) noexcept
{
	if (base.GetTag().IsDefined()) {
		TagBuilder builder(add.GetTag());
		builder.Complement(base.GetTag());
		add.SetTag(builder.Commit());
	}

	add.SetLastModified(base.GetLastModified());

	if (add.GetStartTime().IsZero()) {
		add.SetStartTime(base.GetStartTime());
	}
	if (add.GetEndTime().IsZero()) {
		add.SetEndTime(base.GetEndTime());
	}

	if (!add.GetAudioFormat().IsDefined())
		add.SetAudioFormat(base.GetAudioFormat());
}

static bool
playlist_check_load_song(DetachedSong &song, const SongLoader &loader) noexcept
try {
	DetachedSong tmp = loader.LoadSong(song.GetURI());

	song.SetURI(tmp.GetURI());
	if (!song.HasRealURI() && tmp.HasRealURI())
		song.SetRealURI(tmp.GetRealURI());

	merge_song_metadata(song, tmp);
	return true;
} catch (...) {
	return false;
}

bool
playlist_check_translate_song(DetachedSong &song, std::string_view base_uri,
			      const SongLoader &loader) noexcept
{
	if (base_uri.compare(".") == 0)
		/* PathTraitsUTF8::GetParent() returns "." when there
		   is no directory name in the given path; clear that
		   now, because it would break the database lookup
		   functions */
		base_uri = {};

	const char *uri = song.GetURI();

#ifdef _WIN32
	if (!PathTraitsUTF8::IsAbsolute(uri) && std::strchr(uri, '\\') != nullptr) {
		/* Windows uses the backslash as path separator, but
		   the MPD protocol uses the (forward) slash by
		   definition; to allow backslashes in relative URIs
		   loaded from playlist files, this step converts all
		   backslashes to (forward) slashes */

		std::string new_uri(uri);
		std::replace(new_uri.begin(), new_uri.end(), '\\', '/');
		song.SetURI(std::move(new_uri));
		uri = song.GetURI();
	}
#endif

	if (base_uri.data() != nullptr &&
	    !PathTraitsUTF8::IsAbsoluteOrHasScheme(uri)) {
		song.SetURI(PathTraitsUTF8::Build(base_uri, uri));
		uri = song.GetURI();
	}

	/* Remove dot segments */
	std::string new_uri = uri_squash_dot_segments(uri);
	song.SetURI(std::move(new_uri));

	return playlist_check_load_song(song, loader);
}
