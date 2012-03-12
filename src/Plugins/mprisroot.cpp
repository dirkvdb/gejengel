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

#include "mprisroot.h"

#include "config.h"

#include <dbus/dbus-glib.h>
#include <glib.h>
#include <glib-object.h>

#include "Core/gejengelcore.h"

G_DEFINE_TYPE(MprisRoot, mpris_root, G_TYPE_OBJECT)

static void mpris_root_init(MprisRoot* pInstance)
{
}

static void mpris_root_class_init(MprisRootClass* pInstance)
{
}


gboolean mprisRoot_identity(MprisRoot* pInstance, gchar** pIdentity, GError** pError)
{
    *pIdentity = g_strdup(PACKAGE_STRING);
    return TRUE;
}

gboolean mprisRoot_quit(MprisRoot* pInstance, GError** pError)
{
    pInstance->pCore->quitApplication();
    return TRUE;
}

#define DBUS_STRUCT_UINT_UINT (dbus_g_type_get_struct ("GValueArray", G_TYPE_UINT, G_TYPE_UINT, G_TYPE_INVALID))

gboolean mprisRoot_mpris_version(MprisRoot* pInstance, GValue** pVersion, GError** pError)
{
    GValue value = { 0 };
    g_value_init(&value, DBUS_STRUCT_UINT_UINT);
    g_value_take_boxed(&value, dbus_g_type_specialized_construct(DBUS_STRUCT_UINT_UINT));
    // field number, value, G_MAXUINT at the end
    dbus_g_type_struct_set(&value, 0, 1, 1, 0, G_MAXUINT);
    *pVersion = reinterpret_cast<GValue*>(g_value_get_boxed(&value));

    return TRUE;
}
