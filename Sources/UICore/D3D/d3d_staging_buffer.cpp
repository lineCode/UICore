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
**  Note: Some of the libraries UICore may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
*/

#include "UICore/precomp.h"
#include "d3d_staging_buffer.h"
#include "d3d_graphic_context.h"
#include "d3d_display_window.h"
#include "UICore/D3D/d3d_target.h"

namespace uicore
{
	D3DStagingBuffer::D3DStagingBuffer(const ComPtr<ID3D11Device> &device, int new_size, BufferUsage usage)
	{
		handles.push_back(std::shared_ptr<DeviceHandles>(new DeviceHandles(device)));
		map_data.pData = 0;
		map_data.RowPitch = 0;
		map_data.DepthPitch = 0;

		size = new_size;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0; // To do: Debug layer refuses to create a buffer with D3D11_RESOURCE_MISC_SHARED specified. This breaks get_handles().
		desc.StructureByteStride = 0;

		if (usage == usage_stream_draw) // To do: find a buffer usage API that works well for both Direct3D and OpenGL
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;
		}

		HRESULT result = handles.front()->device->CreateBuffer(&desc, 0, handles.front()->buffer.output_variable());
		D3DTarget::throw_if_failed("Unable to create transfer buffer", result);
	}

	D3DStagingBuffer::D3DStagingBuffer(const ComPtr<ID3D11Device> &device, const void *data, int new_size, BufferUsage usage)
	{
		handles.push_back(std::shared_ptr<DeviceHandles>(new DeviceHandles(device)));
		map_data.pData = 0;
		map_data.RowPitch = 0;
		map_data.DepthPitch = 0;

		size = new_size;

		D3D11_SUBRESOURCE_DATA resource_data;
		resource_data.pSysMem = data;
		resource_data.SysMemPitch = 0;
		resource_data.SysMemSlicePitch = 0;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0; // To do: Debug layer refuses to create a buffer with D3D11_RESOURCE_MISC_SHARED specified. This breaks get_handles().
		desc.StructureByteStride = 0;

		if (usage == usage_stream_draw) // To do: find a buffer usage API that works well for both Direct3D and OpenGL
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;
		}

		HRESULT result = handles.front()->device->CreateBuffer(&desc, &resource_data, handles.front()->buffer.output_variable());
		D3DTarget::throw_if_failed("Unable to create transfer buffer", result);
	}

	D3DStagingBuffer::~D3DStagingBuffer()
	{
	}

	void *D3DStagingBuffer::data()
	{
		return map_data.pData;
	}

	ComPtr<ID3D11Buffer> &D3DStagingBuffer::get_buffer(const ComPtr<ID3D11Device> &device)
	{
		if (device)
			return get_handles(device).buffer;
		else
			return handles.front()->buffer;
	}

	void D3DStagingBuffer::lock(const std::shared_ptr<GraphicContext> &gc, BufferAccess access)
	{
		auto d3d_window = static_cast<D3DGraphicContext*>(gc.get())->get_window();
		map_device = &d3d_window->get_device();
		map_device_context = &d3d_window->get_device_context();

		HRESULT result = map_device_context->get()->Map(handles.front()->buffer, 0, to_d3d_map_type(access), 0, &map_data);
		D3DTarget::throw_if_failed("ID3D11DeviceContext.Map failed", result);
	}

	void D3DStagingBuffer::unlock()
	{
		map_device_context->get()->Unmap(get_handles(*map_device).buffer, 0);
		map_device = nullptr;
		map_device_context = nullptr;
		map_data.pData = 0;
		map_data.RowPitch = 0;
		map_data.DepthPitch = 0;
	}

	void D3DStagingBuffer::upload_data(const std::shared_ptr<GraphicContext> &gc, int offset, const void *input, int input_size)
	{
		D3D11_BUFFER_DESC desc;
		handles.front()->buffer->GetDesc(&desc);
		if (input_size < 0 || offset + input_size > desc.ByteWidth)
			throw Exception("Out of bounds!");

		lock(gc, access_write_only);	// Should this be access_write_discard - And force offset ==0 and input_size = desc.ByteWidth
		memcpy(static_cast<char*>(data()) + offset, input, input_size);
		unlock();

	}

	void D3DStagingBuffer::device_destroyed(ID3D11Device *device)
	{
		for (size_t i = 0; i < handles.size(); i++)
		{
			if (handles[i]->device.get() == device)
			{
				handles.erase(handles.begin() + i);
				return;
			}
		}
	}

	D3DStagingBuffer::DeviceHandles &D3DStagingBuffer::get_handles(const ComPtr<ID3D11Device> &device)
	{
		for (size_t i = 0; i < handles.size(); i++)
			if (handles[i]->device == device)
				return *handles[i];

		ComPtr<IDXGIResource> resource;
		HRESULT result = handles.front()->buffer->QueryInterface(__uuidof(IDXGIResource), (void**)resource.output_variable());
		D3DTarget::throw_if_failed("ID3D11Buffer.QueryInterface(IDXGIResource) failed", result);

		HANDLE handle = 0;
		result = resource->GetSharedHandle(&handle);
		D3DTarget::throw_if_failed("IDXGIResource.GetSharedHandle failed", result);

		ComPtr<ID3D11Buffer> buffer_handle;
		result = device->OpenSharedResource(handle, __uuidof(ID3D11Buffer), (void**)buffer_handle.output_variable());
		D3DTarget::throw_if_failed("ID3D11Device.OpenSharedResource failed", result);

		handles.push_back(std::shared_ptr<DeviceHandles>(new DeviceHandles(device)));
		handles.back()->buffer = buffer_handle;
		return *handles.back();
	}

	D3D11_MAP D3DStagingBuffer::to_d3d_map_type(BufferAccess access)
	{
		switch (access)
		{
		case access_read_only:
			return D3D11_MAP_READ;
		case access_write_only:
			return D3D11_MAP_WRITE;
		case access_write_discard:
			return D3D11_MAP_WRITE_DISCARD;
		case access_read_write:
			return D3D11_MAP_READ_WRITE;
		}
		throw Exception("Unsupported access type");
	}
}
