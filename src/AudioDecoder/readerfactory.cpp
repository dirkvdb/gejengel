//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "readerfactory.h"

#include <stdexcept>

#include "filereader.h"
#include "utils/log.h"

#include "config.h"

#ifdef HAVE_LIBUPNP
    #include "httpreader.h"
#endif

namespace Gejengel
{

IReader* ReaderFactory::create(const std::string& uri)
{
	utils::log::info("ReaderFactory:", uri);
    if (uri.substr(0, 7) == "http://")
    {
#ifdef HAVE_LIBUPNP
        return new HttpReader(uri);
#else
        throw std::logic_error("Not compiled with support for urls as input");
#endif
    }

    return new FileReader(uri);
}

}
   
