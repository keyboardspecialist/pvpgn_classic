/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef INCLUDED_D2CHAR_CHECKSUM_H
#define INCLUDED_D2CHAR_CHECKSUM_H

#define D2CHARSAVE_VERSION_OFFSET			0x04
#define D2CHARSAVE_CHECKSUM_OFFSET			0x0C
#define D2CHARSAVE_CHECKSUM_MIN_VERSION		0x0000005C

namespace pvpgn
{

	extern int d2charsave_checksum(unsigned char const * data, unsigned int len, unsigned int offset);

}

#endif
