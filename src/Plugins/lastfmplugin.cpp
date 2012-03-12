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

#include "lastfmplugin.h"

#include <stdexcept>
#include <cassert>

#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/stock.h>
#include <gdkmm/pixbufloader.h>
#include <lastfmlib/lastfmscrobbler.h>

#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"

using namespace std;
using namespace utils;

static const uint8_t icon[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a, 0xf4, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47,
    0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0xa0, 0xbd, 0xa7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0b, 0x13, 0x00, 0x00, 0x0b, 0x13,
    0x01, 0x00, 0x9a, 0x9c, 0x18, 0x00, 0x00, 0x09, 0x9a, 0x49, 0x44, 0x41, 0x54, 0x58, 0xc3, 0xad, 0x97, 0x79, 0x90, 0x54,
    0xd5, 0x15, 0xc6, 0x7f, 0xf7, 0xbe, 0xad, 0xd7, 0xe9, 0x99, 0xee, 0x61, 0xf6, 0x19, 0x64, 0x18, 0x59, 0x45, 0x14, 0x8c,
    0x28, 0x88, 0xb8, 0x23, 0x60, 0xdc, 0x10, 0xb7, 0x52, 0x8c, 0x0b, 0x51, 0xa2, 0x95, 0x40, 0x95, 0x49, 0x91, 0xaa, 0xa4,
    0x24, 0x54, 0x62, 0x52, 0x5a, 0xee, 0x9a, 0xd2, 0xc4, 0x44, 0x13, 0x2c, 0x77, 0xa3, 0xd1, 0x02, 0x45, 0x70, 0x45, 0x90,
    0x04, 0x50, 0xc1, 0x41, 0x16, 0x41, 0x16, 0x07, 0x66, 0x40, 0x66, 0xa6, 0xa7, 0x67, 0xba, 0x5f, 0x77, 0xbf, 0xf7, 0xee,
    0xcd, 0x1f, 0xcd, 0xa0, 0xa4, 0x48, 0x40, 0x93, 0x5b, 0x75, 0xea, 0xdd, 0xaa, 0x77, 0xee, 0x39, 0xdf, 0x3b, 0xef, 0xde,
    0xef, 0x7c, 0x57, 0xf0, 0x2d, 0xc7, 0xeb, 0xd0, 0x18, 0x25, 0x3a, 0xd8, 0x11, 0x35, 0xc7, 0xca, 0x78, 0x24, 0x25, 0x0d,
    0x69, 0xa8, 0x42, 0x31, 0x9b, 0xcb, 0x75, 0xef, 0xe8, 0xa5, 0xe3, 0x8b, 0xad, 0xb0, 0x71, 0x0e, 0x78, 0x00, 0xbb, 0xa2,
    0xa3, 0x44, 0x53, 0xf6, 0x53, 0xfd, 0xdf, 0xe2, 0x89, 0xa3, 0x4d, 0xbc, 0x92, 0x9a, 0x6b, 0xc3, 0x2d, 0x43, 0xaf, 0xad,
    0x38, 0xf1, 0xb8, 0xb1, 0xc6, 0xc0, 0xfa, 0xa4, 0x51, 0x95, 0x44, 0x46, 0xc2, 0x08, 0x43, 0xa2, 0x8a, 0x1e, 0x2a, 0x9d,
    0x41, 0x75, 0xec, 0xf7, 0x73, 0x1b, 0xb6, 0x6c, 0xea, 0x5d, 0xbf, 0xf9, 0xcd, 0x5d, 0xe9, 0x75, 0xf7, 0x5e, 0x0a, 0xbb,
    0x8f, 0x14, 0xf7, 0x88, 0x00, 0x56, 0xd1, 0x78, 0x75, 0xf9, 0xa4, 0xf1, 0xbf, 0x4b, 0x9e, 0x3f, 0xa9, 0xd1, 0x1e, 0x33,
    0x12, 0x99, 0x88, 0x83, 0x1f, 0xa0, 0x3c, 0x1f, 0x7c, 0x1f, 0x0d, 0x5a, 0x18, 0x52, 0x08, 0xcb, 0x44, 0x58, 0x16, 0x78,
    0x3e, 0x41, 0x5b, 0x07, 0xe9, 0x65, 0x2b, 0xe8, 0x79, 0x7b, 0xe5, 0x13, 0x2b, 0xb6, 0x2f, 0x9f, 0x7b, 0x2b, 0xf4, 0x6c,
    0x38, 0x7e, 0x9a, 0x18, 0xb9, 0x7e, 0x91, 0x3e, 0x6a, 0x00, 0x57, 0x82, 0x35, 0x6f, 0xd0, 0x69, 0xcf, 0x55, 0xdf, 0x78,
    0xc5, 0x25, 0xb1, 0x33, 0x4f, 0x21, 0xf0, 0x95, 0xf2, 0xdb, 0xf7, 0x09, 0x95, 0xcb, 0x0b, 0xad, 0x02, 0x7d, 0xb8, 0xb5,
    0x42, 0x48, 0x84, 0x6d, 0x21, 0xcb, 0xe3, 0xda, 0xae, 0xae, 0xd4, 0xde, 0x8e, 0xdd, 0x32, 0xfd, 0xfc, 0xe2, 0xbd, 0x3b,
    0x5f, 0x7c, 0x62, 0xe6, 0x99, 0xe4, 0xde, 0x3c, 0xea, 0x0a, 0xcc, 0x81, 0xe4, 0xad, 0xe7, 0xcd, 0x5c, 0x9d, 0xba, 0xf9,
    0xaa, 0x66, 0x99, 0x4c, 0x50, 0x6c, 0xeb, 0xd0, 0xda, 0x2d, 0x08, 0xa4, 0x04, 0x29, 0xfa, 0xb3, 0x1d, 0xba, 0x48, 0xeb,
    0xaf, 0x9f, 0x4a, 0x83, 0x14, 0x18, 0xe5, 0x09, 0x6d, 0xa7, 0x12, 0x22, 0xb3, 0x64, 0x39, 0x6d, 0xf7, 0x3f, 0x31, 0xfb,
    0x14, 0x77, 0xe3, 0xa3, 0x47, 0x04, 0x70, 0x2e, 0x44, 0x1f, 0x99, 0x76, 0xe3, 0x47, 0xd5, 0x3f, 0xbc, 0x72, 0x48, 0x31,
    0x9b, 0x55, 0x2a, 0x93, 0x95, 0xc2, 0x32, 0x35, 0x42, 0x08, 0xd0, 0xa0, 0x8f, 0x62, 0xc3, 0x1c, 0x00, 0xa7, 0x95, 0x82,
    0x40, 0x61, 0xd7, 0x55, 0x91, 0x5d, 0xf5, 0x09, 0x7b, 0x1e, 0x78, 0xf2, 0xfa, 0x93, 0xfb, 0x5a, 0x9f, 0xdc, 0x7f, 0xec,
    0x39, 0xa2, 0xf2, 0xf3, 0x65, 0xfa, 0xb0, 0x00, 0xd6, 0x8d, 0xbf, 0xf4, 0x83, 0x86, 0x1b, 0xae, 0x98, 0xe0, 0xa3, 0xb5,
    0xce, 0x17, 0x4b, 0x5f, 0xfd, 0x5d, 0x87, 0xa0, 0x04, 0x58, 0x2b, 0x6d, 0x96, 0xc5, 0x45, 0x76, 0xf9, 0x6a, 0xb6, 0xfc,
    0xe1, 0x81, 0xef, 0x9d, 0x4b, 0x76, 0x4d, 0xbf, 0x8b, 0xf1, 0x4d, 0xff, 0xb7, 0xca, 0x47, 0xce, 0x19, 0x78, 0xf9, 0x85,
    0xb3, 0x54, 0x24, 0x1c, 0xe8, 0x7c, 0x41, 0x96, 0xca, 0xa9, 0xbe, 0xbb, 0x05, 0xfd, 0x73, 0x2d, 0x82, 0xac, 0xab, 0x9d,
    0x86, 0x1a, 0x11, 0xd6, 0x91, 0xf1, 0x0f, 0x7d, 0xfe, 0xc9, 0x63, 0x07, 0xa0, 0x7d, 0x0d, 0xe0, 0xb7, 0xd0, 0x30, 0x7e,
    0xda, 0x65, 0x2f, 0x39, 0x23, 0x87, 0xd8, 0x2a, 0x9b, 0x93, 0x3a, 0x50, 0x68, 0x3f, 0x40, 0x2b, 0x8d, 0x2e, 0x78, 0x28,
    0xd7, 0x45, 0xe7, 0x0b, 0xe8, 0xa2, 0x87, 0x50, 0x0a, 0xad, 0x15, 0xda, 0xf3, 0x4b, 0x3e, 0xdf, 0xb4, 0x40, 0x95, 0x4e,
    0xc7, 0x01, 0x5f, 0x14, 0xe8, 0x20, 0x80, 0x20, 0x10, 0x1a, 0x8d, 0x55, 0x16, 0xaf, 0xba, 0x6e, 0x77, 0x31, 0xfb, 0x68,
    0xf7, 0x8e, 0x95, 0x00, 0x07, 0xeb, 0x7b, 0xda, 0x31, 0xe3, 0x67, 0x85, 0x9a, 0x1b, 0x63, 0x7e, 0x2e, 0xa7, 0xb5, 0xef,
    0x6b, 0xed, 0x79, 0xa8, 0xbe, 0x2c, 0x7e, 0x57, 0x1a, 0x65, 0x19, 0x58, 0xcd, 0x4d, 0xd8, 0x23, 0x87, 0x60, 0x36, 0xd6,
    0x11, 0x08, 0xf0, 0x3b, 0xbb, 0x51, 0x39, 0x17, 0xed, 0x79, 0x87, 0x58, 0x90, 0xce, 0xe0, 0x17, 0x8a, 0x88, 0xca, 0x24,
    0xa2, 0xb2, 0x02, 0xdf, 0x75, 0x09, 0x7a, 0x7a, 0x4b, 0xef, 0xb2, 0x39, 0x6d, 0x24, 0xcb, 0x88, 0x8c, 0x18, 0x7c, 0x7b,
    0x7f, 0x5e, 0xb3, 0x7f, 0x92, 0x18, 0xd8, 0xf8, 0x23, 0xb3, 0xac, 0x0c, 0x2f, 0xeb, 0x22, 0x40, 0x04, 0xf9, 0x02, 0xb2,
    0xba, 0x92, 0xe4, 0x05, 0x67, 0x13, 0x1b, 0xda, 0x7c, 0xe8, 0x86, 0x07, 0x32, 0x6b, 0x3e, 0x25, 0xb3, 0xe4, 0x7d, 0x74,
    0x4f, 0x06, 0x19, 0x72, 0x40, 0x08, 0x82, 0x9c, 0x4b, 0xe8, 0x94, 0x13, 0x49, 0x5d, 0x78, 0x0e, 0x86, 0x65, 0x01, 0xe0,
    0xe5, 0x5c, 0x3a, 0x9f, 0x5f, 0x44, 0x71, 0xe3, 0x56, 0xa4, 0x63, 0x0b, 0x2f, 0x08, 0x54, 0xb8, 0xa9, 0xae, 0x6a, 0x75,
    0xd9, 0x71, 0x33, 0xbe, 0x97, 0x69, 0x7d, 0xc1, 0x04, 0xf8, 0x13, 0xa9, 0x09, 0x91, 0x54, 0x45, 0x65, 0xd1, 0x2b, 0x2a,
    0xed, 0xf9, 0x32, 0xc8, 0xf4, 0xe2, 0x9c, 0x38, 0x92, 0xda, 0x9b, 0xae, 0x44, 0x00, 0xd9, 0x2f, 0x76, 0xe1, 0xb6, 0x6e,
    0x46, 0xb9, 0x79, 0x8c, 0xf2, 0x72, 0xa2, 0x27, 0x0c, 0x27, 0x71, 0xd2, 0x28, 0xa2, 0x23, 0x87, 0xb0, 0xe7, 0x9e, 0x3f,
    0xe2, 0x77, 0x76, 0x23, 0xa4, 0xc4, 0x68, 0xaa, 0xa3, 0x6a, 0xfa, 0x14, 0x8a, 0x9d, 0xdd, 0xec, 0x5f, 0xfc, 0x0e, 0x48,
    0x83, 0xe4, 0xf7, 0xcf, 0xa2, 0xea, 0x07, 0x97, 0xb1, 0xeb, 0x17, 0xf7, 0x40, 0xbe, 0x80, 0x16, 0x42, 0x18, 0xd1, 0x30,
    0xd4, 0x57, 0x5f, 0x42, 0x3f, 0x80, 0x86, 0x8a, 0xda, 0xb3, 0xad, 0x58, 0x94, 0x20, 0xeb, 0x0a, 0x5c, 0x17, 0xa3, 0xae,
    0x9a, 0xba, 0x9b, 0xae, 0x44, 0x03, 0xbb, 0x7f, 0xbf, 0x90, 0xfc, 0xc7, 0x1b, 0x90, 0x88, 0x12, 0x07, 0x28, 0x4d, 0xfa,
    0xc5, 0xc5, 0xc4, 0xa7, 0x9d, 0x41, 0xd5, 0xc5, 0x93, 0xa9, 0xb9, 0x6d, 0x26, 0xbb, 0x17, 0x3c, 0x48, 0xd0, 0xd5, 0xcd,
    0x80, 0x19, 0x53, 0x01, 0xe8, 0x78, 0xe4, 0x29, 0xbc, 0x6d, 0x3b, 0x11, 0x96, 0x49, 0xae, 0x75, 0x33, 0x32, 0x12, 0x86,
    0x4c, 0x1f, 0x4a, 0x4a, 0xd0, 0x5a, 0x28, 0xcb, 0x24, 0x5c, 0x51, 0x3e, 0xe6, 0x04, 0x08, 0x4b, 0x00, 0x33, 0xe4, 0x8c,
    0xd0, 0xa6, 0x01, 0xae, 0x8b, 0x9f, 0xcb, 0x93, 0x9a, 0x31, 0x0d, 0x80, 0xdd, 0x0f, 0xff, 0x05, 0x77, 0xc5, 0x5a, 0x8c,
    0x90, 0x83, 0x70, 0x6c, 0x84, 0x69, 0x22, 0x1c, 0x1b, 0xc3, 0xb6, 0xe8, 0x79, 0x7e, 0x11, 0x5f, 0xbd, 0xb6, 0x0c, 0xbb,
    0xbc, 0x8c, 0xd8, 0xa4, 0x71, 0x04, 0xe9, 0x5e, 0x64, 0x24, 0x84, 0x0a, 0x14, 0xfe, 0xfe, 0x2e, 0x84, 0x56, 0x48, 0xdb,
    0x42, 0xa4, 0x33, 0xa8, 0xb6, 0x0e, 0x08, 0x02, 0xf0, 0x3c, 0xf0, 0x3c, 0x8d, 0xe7, 0x61, 0x3b, 0x4e, 0xd5, 0xcf, 0x88,
    0xd7, 0xca, 0xd2, 0x4e, 0x14, 0xb5, 0x7e, 0x4f, 0x2f, 0xba, 0xe0, 0x61, 0x1d, 0xd3, 0x40, 0x74, 0xc8, 0x20, 0x7a, 0xd7,
    0x6d, 0x24, 0xfb, 0xf6, 0x87, 0x48, 0xcb, 0x44, 0x65, 0x5d, 0x94, 0x9b, 0x47, 0xe5, 0x0b, 0x25, 0x2b, 0x14, 0x31, 0xc3,
    0x61, 0xba, 0x9f, 0x5f, 0x84, 0xd7, 0x97, 0x25, 0x79, 0xfe, 0x24, 0x70, 0x6c, 0xfa, 0xd6, 0xb6, 0x22, 0x0d, 0x49, 0x6a,
    0xe6, 0xa5, 0xe8, 0x8a, 0x04, 0x5e, 0x3a, 0x83, 0xdf, 0xd5, 0x03, 0x9e, 0x57, 0x5a, 0x9f, 0x73, 0xd1, 0xbe, 0x2f, 0x50,
    0x1a, 0xbf, 0x50, 0x48, 0xc4, 0x64, 0x45, 0xc2, 0x00, 0xc4, 0x0d, 0xe5, 0x0d, 0x37, 0x1b, 0xfb, 0xba, 0x1a, 0x8a, 0xed,
    0xfb, 0x88, 0x8c, 0x1b, 0x2d, 0x12, 0xe3, 0xc7, 0xd2, 0xbd, 0xe4, 0x3d, 0x82, 0xf6, 0xbd, 0x18, 0xd1, 0x68, 0xa9, 0xf4,
    0x5a, 0x83, 0xd6, 0xfd, 0x47, 0x0a, 0xad, 0x14, 0xba, 0xa7, 0x0f, 0x1d, 0x09, 0x13, 0x3f, 0x7e, 0x18, 0x85, 0x7d, 0x9d,
    0xa4, 0x5f, 0x7a, 0x03, 0xf3, 0xd8, 0x81, 0x94, 0x9f, 0x7c, 0x02, 0xc9, 0x29, 0x67, 0x10, 0x1a, 0x3d, 0x1c, 0x65, 0x18,
    0x14, 0xda, 0xda, 0x11, 0x85, 0x02, 0x46, 0x34, 0x4a, 0x90, 0xe9, 0xc3, 0xdd, 0xf4, 0x05, 0x5e, 0x5b, 0x87, 0x28, 0xc4,
    0xc2, 0x8f, 0x9b, 0x80, 0xf6, 0x85, 0xf0, 0x50, 0x9a, 0xfc, 0x8e, 0x36, 0xbc, 0x74, 0x06, 0x80, 0xf4, 0xd2, 0x0f, 0xc8,
    0x2f, 0x5f, 0x8d, 0x55, 0x5f, 0x8b, 0x11, 0x0b, 0x63, 0xc4, 0x63, 0x88, 0xb0, 0x83, 0x70, 0x9c, 0x52, 0x69, 0x2d, 0x13,
    0x23, 0x1e, 0x23, 0xbf, 0x61, 0x0b, 0x6a, 0xc6, 0x54, 0x2a, 0xa7, 0x4f, 0xc1, 0xdf, 0xb1, 0x9b, 0xf4, 0xd3, 0xaf, 0x52,
    0xd8, 0xb0, 0x95, 0xc8, 0xe8, 0x61, 0xc4, 0xc6, 0x8e, 0x22, 0xda, 0x72, 0x0c, 0xfa, 0xa6, 0x2b, 0xd8, 0x3e, 0xff, 0x7e,
    0x3a, 0x1f, 0x7b, 0x1a, 0x21, 0x25, 0x42, 0x80, 0x34, 0x4d, 0x55, 0x84, 0xc0, 0x04, 0xf0, 0x75, 0xd0, 0x25, 0x84, 0x8d,
    0xb4, 0x2d, 0x54, 0x2e, 0x5f, 0x3a, 0x9f, 0x55, 0x29, 0xbc, 0xce, 0x34, 0x3a, 0x28, 0x11, 0x4c, 0x3f, 0xc7, 0x0b, 0x29,
    0x41, 0x4a, 0x30, 0x0d, 0x84, 0x10, 0xf8, 0x3d, 0x19, 0x42, 0xa3, 0x87, 0x53, 0x7d, 0xc9, 0x64, 0x72, 0x3b, 0xdb, 0x70,
    0xdf, 0xf9, 0x90, 0xae, 0x67, 0x5e, 0x45, 0x29, 0x8d, 0xd5, 0xdc, 0x44, 0xe2, 0xa2, 0x73, 0x19, 0x38, 0x6f, 0x36, 0xcd,
    0xf3, 0xe7, 0xd0, 0xba, 0xea, 0x63, 0x8a, 0x6b, 0x5b, 0x91, 0xb1, 0x08, 0x42, 0xd3, 0x97, 0x0d, 0xbc, 0x3e, 0x09, 0x90,
    0xd1, 0xea, 0x33, 0xd3, 0x36, 0x91, 0xb1, 0x30, 0xf9, 0x0d, 0x9b, 0x01, 0x48, 0x5e, 0x74, 0x1e, 0x84, 0x1d, 0x64, 0x24,
    0x84, 0x91, 0x88, 0x62, 0x24, 0x62, 0x18, 0xf1, 0x08, 0x32, 0x1a, 0x42, 0x84, 0x2c, 0xa4, 0x29, 0xf1, 0x0b, 0x05, 0xa4,
    0x65, 0xd1, 0xf1, 0xd0, 0x93, 0x00, 0x54, 0x5e, 0x37, 0x9d, 0xc0, 0x2b, 0x62, 0xa6, 0x12, 0xd8, 0x15, 0x71, 0x74, 0xc7,
    0x5e, 0xf6, 0xff, 0xe6, 0x21, 0x36, 0xdf, 0x50, 0xe2, 0x9d, 0xc4, 0xb4, 0xb3, 0xd0, 0x5a, 0x21, 0x1c, 0x0b, 0xcf, 0x94,
    0x1d, 0x77, 0xb7, 0x7f, 0xbe, 0x47, 0x02, 0x7c, 0x96, 0xef, 0xfd, 0x87, 0x72, 0x2c, 0x8c, 0xf2, 0xb8, 0x2e, 0x6e, 0xdd,
    0x4e, 0x7a, 0xf5, 0x3a, 0x92, 0x93, 0xc6, 0x11, 0xbf, 0xe8, 0x1c, 0xfc, 0xee, 0x74, 0xa9, 0xc7, 0x87, 0x6c, 0x44, 0xd8,
    0x41, 0x86, 0x1d, 0x0c, 0xdb, 0xc4, 0xcf, 0x66, 0x49, 0xdd, 0x3e, 0x0b, 0x7b, 0xcc, 0x08, 0x82, 0x2d, 0xdb, 0xe8, 0x5a,
    0xf5, 0x11, 0x03, 0xa6, 0x9e, 0x89, 0x3d, 0x6a, 0x18, 0x04, 0x01, 0x32, 0x16, 0xc1, 0x4c, 0x95, 0xe3, 0x34, 0xd5, 0xe0,
    0xb5, 0xb5, 0x97, 0x1a, 0x4f, 0x3c, 0x0a, 0x61, 0x5b, 0x9b, 0x61, 0x07, 0x57, 0xb2, 0x71, 0x0d, 0xd9, 0xac, 0x04, 0xb8,
    0xa3, 0xf3, 0xf3, 0x95, 0x39, 0xa9, 0xf7, 0x3b, 0x89, 0xb8, 0xb4, 0xe2, 0x11, 0xda, 0x7e, 0x75, 0x1f, 0x1a, 0x18, 0xfa,
    0xf8, 0x5d, 0xc4, 0xaf, 0xba, 0x90, 0x40, 0x05, 0xa8, 0x42, 0x1e, 0x94, 0x8f, 0xd6, 0x01, 0x2a, 0x12, 0xa2, 0x6a, 0xfe,
    0x5c, 0x06, 0xcd, 0xbd, 0x89, 0xf2, 0x8b, 0x27, 0x23, 0x43, 0x16, 0x9d, 0x0b, 0x5f, 0x02, 0xa0, 0xf1, 0xce, 0x9f, 0x12,
    0x98, 0x12, 0x95, 0xcb, 0xa2, 0xf2, 0x39, 0xfc, 0xc0, 0xa7, 0xea, 0x96, 0x6b, 0x00, 0x70, 0x37, 0x6f, 0xc5, 0x88, 0x47,
    0xb4, 0xe1, 0x58, 0xec, 0x2b, 0xe6, 0x5e, 0x38, 0xa4, 0x1d, 0x3f, 0x37, 0xe4, 0xd4, 0x3f, 0x8f, 0xa9, 0xab, 0xbf, 0xbe,
    0x28, 0xd0, 0x41, 0x77, 0x5a, 0x38, 0x13, 0x4f, 0xa5, 0xe5, 0xc1, 0x05, 0x48, 0x20, 0xb3, 0x7e, 0x23, 0x3d, 0xcb, 0xff,
    0x89, 0xca, 0xf4, 0xe2, 0x0c, 0x6c, 0xa0, 0xec, 0xf4, 0x71, 0x44, 0x1a, 0x6a, 0x71, 0x3b, 0xbe, 0x62, 0xd7, 0xcf, 0xef,
    0x44, 0x77, 0xec, 0xc3, 0x77, 0xf3, 0x54, 0xcf, 0xfb, 0x31, 0x03, 0xce, 0x9f, 0x44, 0xef, 0x86, 0x2d, 0x74, 0x2d, 0x5a,
    0x06, 0x4a, 0x53, 0x7e, 0xde, 0x24, 0x12, 0x63, 0x8e, 0xa3, 0x67, 0xfd, 0x46, 0x76, 0xce, 0x9e, 0xa7, 0x1d, 0xcb, 0x14,
    0xb9, 0x5c, 0xbe, 0x6f, 0xcc, 0xea, 0xc5, 0x75, 0x40, 0xef, 0x41, 0x00, 0xa7, 0x8b, 0xd8, 0xa9, 0x0f, 0x4f, 0x3c, 0x77,
    0xa9, 0x13, 0x8b, 0x44, 0x95, 0x56, 0x04, 0x3d, 0xbd, 0x50, 0x5b, 0x43, 0xed, 0x4f, 0x66, 0x91, 0x98, 0x70, 0x12, 0xc6,
    0x37, 0x74, 0x81, 0x9f, 0x2f, 0xd2, 0xb5, 0xe4, 0x1d, 0xba, 0x9e, 0x7d, 0x05, 0xc3, 0x2b, 0x22, 0x23, 0x11, 0xb4, 0xe7,
    0x51, 0xcc, 0x64, 0xa9, 0xbc, 0x65, 0x26, 0xa9, 0xa9, 0x67, 0x61, 0x18, 0xa5, 0x46, 0x1b, 0xf8, 0x01, 0x5d, 0xcb, 0x96,
    0xd3, 0x71, 0xf7, 0x23, 0xda, 0x32, 0x84, 0xb0, 0x4c, 0x8b, 0xd6, 0xed, 0x5f, 0x2e, 0xb8, 0x78, 0xd3, 0xf2, 0x3b, 0xee,
    0x1d, 0x30, 0xec, 0x10, 0x5d, 0x65, 0x3f, 0xd1, 0x72, 0xf2, 0x3d, 0x67, 0x8d, 0x1a, 0x79, 0x5b, 0x8e, 0x40, 0x09, 0x21,
    0x04, 0x45, 0x4f, 0xf8, 0x85, 0x22, 0xd6, 0xc0, 0x46, 0xec, 0xa6, 0x06, 0x64, 0x34, 0x82, 0xca, 0xe6, 0xf0, 0x3b, 0xf6,
    0xa2, 0xba, 0xd3, 0x58, 0x89, 0xb2, 0x83, 0xf4, 0xac, 0x94, 0x42, 0xf8, 0x01, 0x5e, 0xba, 0x07, 0x99, 0x4a, 0x62, 0x54,
    0x26, 0xd1, 0x81, 0xc2, 0xdb, 0xdd, 0x4e, 0xd0, 0xbe, 0x17, 0x33, 0x16, 0xd1, 0x96, 0x63, 0x8b, 0xee, 0xbd, 0x9d, 0x6d,
    0x27, 0xbd, 0xff, 0xf7, 0x13, 0x80, 0xce, 0x43, 0xba, 0xa1, 0x9a, 0x7f, 0x9f, 0x27, 0xe7, 0xcf, 0xbd, 0xf3, 0xdd, 0xaa,
    0x01, 0xa7, 0x37, 0xb7, 0x34, 0x1d, 0xef, 0xfa, 0xbe, 0x96, 0x91, 0x10, 0x86, 0x69, 0x42, 0x31, 0x4f, 0xf0, 0xc5, 0x36,
    0x02, 0x69, 0x20, 0x6c, 0x0b, 0xd3, 0xb6, 0x11, 0x8d, 0x35, 0x5f, 0xff, 0x41, 0xa5, 0x11, 0x07, 0x44, 0x88, 0x1d, 0x71,
    0xd0, 0xae, 0x4b, 0xb0, 0x73, 0x17, 0xf8, 0x1e, 0x52, 0x6b, 0x2d, 0xab, 0x93, 0x48, 0xd3, 0x14, 0xba, 0xd7, 0xe5, 0xe5,
    0x8f, 0xd7, 0xcf, 0xea, 0x4f, 0x7e, 0x38, 0x49, 0x26, 0x87, 0xc2, 0xc4, 0xa7, 0x27, 0x5f, 0xfe, 0x62, 0xd5, 0xa0, 0xfa,
    0xca, 0x82, 0x0a, 0xb4, 0x34, 0x4d, 0x84, 0x65, 0x09, 0x61, 0x9a, 0x08, 0xd3, 0x00, 0xc3, 0x44, 0x18, 0x12, 0x8c, 0x12,
    0x0f, 0x94, 0x74, 0xa8, 0x2e, 0xb1, 0xe3, 0x01, 0x31, 0xa2, 0xfc, 0x00, 0x7c, 0x0f, 0xe5, 0xf9, 0x68, 0xcf, 0xd3, 0x52,
    0x0a, 0x61, 0xe4, 0x8a, 0xbc, 0xb1, 0x6a, 0xcd, 0x2f, 0x6f, 0xde, 0xba, 0xe6, 0xd7, 0x47, 0x52, 0xc5, 0x4e, 0x0b, 0x8c,
    0x7f, 0xf6, 0x82, 0xab, 0x9f, 0x69, 0x18, 0xd1, 0x5c, 0x9d, 0x07, 0x25, 0xa4, 0x94, 0xc2, 0xb6, 0x10, 0xc6, 0x01, 0x10,
    0xa6, 0x81, 0x30, 0xcc, 0x92, 0xf8, 0xd4, 0x1c, 0xa0, 0x68, 0xbf, 0x04, 0xc2, 0xf3, 0xd1, 0x81, 0x8f, 0xf6, 0x7c, 0x94,
    0xe7, 0x29, 0x43, 0x4a, 0xa9, 0xf7, 0xf7, 0xb0, 0xf8, 0xfd, 0x95, 0x77, 0xcc, 0xde, 0xf6, 0xd1, 0x82, 0x7f, 0x4f, 0x66,
    0x1c, 0x06, 0x40, 0xd0, 0x05, 0x7b, 0x5f, 0xde, 0xf2, 0xe9, 0x87, 0x13, 0x9d, 0xd4, 0xd0, 0xfa, 0xe6, 0xc6, 0x26, 0xa3,
    0x22, 0x01, 0x8e, 0x8d, 0xb0, 0x2d, 0x4d, 0xc8, 0x11, 0x32, 0xe4, 0x20, 0x42, 0x0e, 0x32, 0xe4, 0x20, 0x1c, 0x0b, 0x6c,
    0xeb, 0x20, 0x30, 0x6d, 0x1a, 0x08, 0xc7, 0xd2, 0x32, 0xec, 0x88, 0x50, 0xc8, 0x11, 0xe9, 0x1d, 0x7b, 0xdc, 0x67, 0xde,
    0x7a, 0x6f, 0xce, 0xed, 0x3b, 0xd7, 0xdf, 0x73, 0x38, 0xdd, 0x6a, 0xfc, 0x07, 0x3d, 0xeb, 0xe7, 0xa0, 0x63, 0xe1, 0xf6,
    0x4d, 0x4b, 0x52, 0x6d, 0x3d, 0xd9, 0x86, 0x01, 0x03, 0x5a, 0xca, 0x6a, 0xaa, 0xe2, 0x76, 0x75, 0x4a, 0x50, 0x16, 0xd5,
    0x3a, 0xe4, 0x68, 0x42, 0x21, 0x44, 0x24, 0x0c, 0x8e, 0x03, 0xb6, 0x85, 0x76, 0x6c, 0x44, 0x34, 0xa2, 0xed, 0x44, 0x5c,
    0x98, 0xd2, 0x14, 0xf9, 0xfd, 0xe9, 0x60, 0xdd, 0x8a, 0xb5, 0xaf, 0x4f, 0xfd, 0xdb, 0x5f, 0xaf, 0x79, 0x23, 0xf3, 0xd5,
    0x6b, 0xdf, 0xe9, 0x6a, 0x36, 0x65, 0xf0, 0x30, 0xeb, 0xf5, 0x6d, 0x9b, 0xca, 0x80, 0x21, 0x77, 0x8d, 0x39, 0xed, 0xe2,
    0xf1, 0x63, 0xc7, 0x4e, 0xae, 0x1b, 0x36, 0x78, 0x74, 0xb4, 0xae, 0x1a, 0x27, 0x11, 0x87, 0x03, 0xb2, 0x0b, 0x3f, 0x20,
    0xc8, 0xe5, 0xc8, 0xed, 0xeb, 0xa4, 0x7b, 0xfb, 0x97, 0x1d, 0x5b, 0x5a, 0x37, 0xbd, 0xf7, 0xd4, 0x92, 0xa5, 0x4f, 0xbf,
    0x12, 0x74, 0xbf, 0xcf, 0xc2, 0x81, 0x3d, 0x5c, 0xbb, 0x53, 0xff, 0xaf, 0x97, 0x53, 0x03, 0x88, 0x01, 0xc9, 0x1a, 0x68,
    0x99, 0x51, 0xd5, 0x32, 0x74, 0xf8, 0xb1, 0x2d, 0x83, 0xe2, 0xa9, 0xf2, 0x84, 0x21, 0xa5, 0x99, 0xcf, 0xb9, 0x7d, 0xed,
    0x5f, 0xee, 0x69, 0x7b, 0x77, 0xe3, 0x27, 0x5b, 0x96, 0x52, 0x68, 0x05, 0xf6, 0x00, 0x7d, 0xfc, 0xbf, 0xc7, 0xf4, 0xfa,
    0xc1, 0x26, 0xf5, 0xb5, 0x61, 0x20, 0x0e, 0x54, 0x00, 0x95, 0x40, 0x0a, 0x28, 0x07, 0x22, 0xd4, 0xdb, 0xf6, 0x6d, 0x63,
    0x27, 0x18, 0xdf, 0x26, 0xe6, 0xbf, 0x00, 0x60, 0x3f, 0x98, 0x85, 0x4f, 0x36, 0x56, 0xaf, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

static const string CLIENT_IDENTIFIER = "gej";
static const string CLIENT_VERSION = "1.0";


LastFmPlugin::LastFmPlugin()
{
    utils::trace("Create LastFmPlugin");
    m_HasSettingsDialog = true;
}

LastFmPlugin::~LastFmPlugin()
{
    destroy();
}

bool LastFmPlugin::initialize(Gejengel::IGejengelCore& core)
{
    utils::trace("Init LastFmPlugin");
    assert(!m_pScrobbler);

    string user = core.getSettings().get("LastFmUser");
    string pass = core.getSettings().get("LastFmPass");
    if (user.empty() || pass.empty())
    {
        return false;
    }
    
    log::debug("user", user, "pass", pass);

    m_pScrobbler.reset(new LastFmScrobbler(CLIENT_IDENTIFIER, CLIENT_VERSION, user, pass, true, false));
    m_pScrobbler->authenticate();
    return true;
}

std::string LastFmPlugin::getName() const
{
    return "Last.fm";
}

std::string LastFmPlugin::getDescription() const
{
    return _("Scrobble your tracks on Last.fm");
}

Glib::RefPtr<Gdk::Pixbuf> LastFmPlugin::getIcon() const
{
    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
    loader->set_size(32, 32);
    loader->write((guint8*)icon, 2567);
    Glib::RefPtr<Gdk::Pixbuf> pixBuf = loader->get_pixbuf();
    loader->close();

    return pixBuf;
}

void LastFmPlugin::onPlay(const Gejengel::Track& track)
{
    assert(m_pScrobbler);

    SubmissionInfo info(track.artist, track.title);
    info.setAlbum(track.album);
    info.setTrackNr(track.trackNr);
    info.setTrackLength(track.durationInSec);
    log::debug("Scrobble:", track.artist, "-", track.title);
    m_pScrobbler->startedPlaying(info);
}

void LastFmPlugin::onPause()
{
    assert(m_pScrobbler);
    m_pScrobbler->pausePlaying(true);
}

void LastFmPlugin::onResume()
{
    assert(m_pScrobbler);
    m_pScrobbler->pausePlaying(false);
}

void LastFmPlugin::onStop()
{
    assert(m_pScrobbler);
    m_pScrobbler->finishedPlaying();
}

void LastFmPlugin::destroy()
{
    m_pScrobbler.reset();
}

void LastFmPlugin::showSettingsDialog(Gejengel::Settings& settings)
{
    Gtk::Dialog dlg(string("Last.fm ") + _("settings"), true);
    Gtk::Table layout(2, 2, false);

    Gtk::Entry userEntry;
    userEntry.set_text(settings.get("LastFmUser"));

    Gtk::Entry passEntry;
    passEntry.set_visibility(false);
    passEntry.set_text(settings.get("LastFmPass"));

    layout.attach(*Gtk::manage(new Gtk::Label(_("Username: "), Gtk::ALIGN_LEFT)), 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    layout.attach(*Gtk::manage(new Gtk::Label(_("Password: "), Gtk::ALIGN_LEFT)), 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    layout.attach(userEntry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    layout.attach(passEntry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

    layout.set_border_width(5);
    layout.set_col_spacings(10);
    layout.set_row_spacings(2);

    dlg.set_default_size(200, 100);
    dlg.set_resizable(true);
    dlg.get_vbox()->pack_start(layout, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
    dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

    dlg.show_all_children();
    dlg.run();

    string user = userEntry.get_text();
    string pass = passEntry.get_text();

    settings.set("LastFmUser", user);
    if (pass != settings.get("LastFmPass"))
    {
        pass = LastFmClient::generatePasswordHash(pass);
        settings.set("LastFmPass", pass);
    }

    m_pScrobbler.reset(new LastFmScrobbler(userEntry.get_text(), pass, true, true));
}