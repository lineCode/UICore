/*
**  UICore
**  Copyright (c) 1997-2015 The UICore Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    Mark Page
*/

#include "UICore/precomp.h"
#include "gl1_transfer_buffer.h"

namespace uicore
{
	GL1TransferBuffer::GL1TransferBuffer(int new_size, BufferUsage usage)
	{
		data_ptr = new char[new_size];
		size = new_size;
	}

	GL1TransferBuffer::GL1TransferBuffer(const void *init_data, int new_size, BufferUsage usage)
	{
		data_ptr = new char[new_size];
		size = new_size;
		memcpy(data_ptr, init_data, size);
	}

	GL1TransferBuffer::~GL1TransferBuffer()
	{
		delete[] data_ptr;
	}

	void GL1TransferBuffer::upload_data(const GraphicContextPtr &gc, int offset, const void *data, int size)
	{
		if ((size < 0) || (offset < 0) || ((size + offset) > this->size))
			throw Exception("Transfer buffer, invalid size");

		memcpy(data_ptr + offset, data, size);
	}
}