//    Copyright (C) 2011 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef LIBRARYACCESS_H
#define LIBRARYACCESS_H

#include <mutex>
#include <memory>

#include "utils/types.h"
#include "utils/subscriber.h"
#include "MusicLibrary/musiclibraryfactory.h"

namespace Gejengel
{

class Settings;
class IScanSubscriber;
class ILibrarySubscriber;
class Track;
class Album;
class AlbumArt;
class LibrarySource;

class LibraryAccess
{
public:
	LibraryAccess(Settings& settings);
	~LibraryAccess();

	LibraryType getLibraryType();
	void setLibraryType(LibraryType type);

	void cleanup();

	uint32_t getTrackCount();
	uint32_t getAlbumCount();

	void getTrackAsync(const std::string& id, utils::ISubscriber<const Track&>& subscriber);
	void getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber);

	void getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber);
	void getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber);

	void getAlbumAsync(const std::string& albumId, utils::ISubscriber<const Album&>& subscriber);
	void getAlbumsAsync(utils::ISubscriber<const Album&>& subscriber);
	void getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber);
	void getRandomAlbumAsync(utils::ISubscriber<const Track&>& subscriber);

	bool getAlbumArt(const Album& album, AlbumArt& art);

	void scan(bool startFresh, IScanSubscriber& subscriber);
	void search(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber);

	void setSource(const LibrarySource& source);

	void addLibrarySubscriber(ILibrarySubscriber& subscriber);

	// Don't like this
	MusicLibrary& getLibrary();

private:
	Settings&		                m_Settings;
	std::unique_ptr<MusicLibrary>   m_Library;
	LibraryType                     m_LibraryType;
	std::mutex	                    m_Mutex;
};

}

#endif
