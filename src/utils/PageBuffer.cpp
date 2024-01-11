/*
Copyright XMN Software AB 2023

QAppLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. The GNU Lesser General Public License
is intended to guarantee your freedom to share and change all versions
of a program--to make sure it remains free software for all its users.

QAppLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QAppLib. If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <qapplib/utils/PageBuffer.h>

namespace qapp
{
	page_buffer::page_buffer(CPagePool& page_pool)
		: m_PagePool(page_pool)
	{
	}

	page_buffer::~page_buffer()
	{
		for (auto handle : m_Pages)
			m_PagePool.Free(handle);
		m_Pages.clear();
	}

	void page_buffer::clear()
	{
		m_Size = 0;
	}

	void page_buffer::resize(size_t size)
	{
		reserve(size);
		m_Size = size;
	}

	void page_buffer::reserve(size_t size)
	{
		if (size <= capacity())
			return;
		const auto page_size_bytes = std::max(page_size_min(), std::min(std::bit_ceil(size), page_size_max()));
		const auto page_size_bits = (unsigned char)std::countr_zero(page_size_bytes);
		if (capacity() < page_size_max())
		{
			const auto new_handle = m_PagePool.Alloc(page_size_bits);
			if (empty())
				m_Pages.push_back(new_handle);
			else
			{
				memcpy(m_PagePool.PtrFromHandle(new_handle), m_PagePool.PtrFromHandle(m_Pages.front()), this->size());
				m_PagePool.Free(m_Pages.front());
				m_Pages.front() = new_handle;
			}
		}
		if (page_size_bytes == page_size_max())
			while (m_Pages.size() < (size + page_size_bytes - 1) >> page_size_bits)
				m_Pages.push_back(m_PagePool.Alloc(page_size_max_bits()));
		m_Capacity = page_size_bytes * m_Pages.size();
	}

	void page_buffer::append(const void* data, size_t size)
	{
		write(this->size(), data, size);
	}

	void page_buffer::write(size_t offset, const void* data, size_t size)
	{
		reserve(offset + size);
		const char* p = (const char*)data;
		const char* end = p + size;
		const auto page_size = this->page_size();
		size_t page_index, page_offset;
		locate(offset, page_index, page_offset);
		for (;;)
		{
			const auto n = std::min(page_size - page_offset, (size_t)(end - p));
			memcpy((char*)page_ptr(page_index) + page_offset, p, n);
			p += n;
			if (p >= end)
				break;
			++page_index;
			page_offset = 0;
		}
		m_Size = std::max(m_Size, offset + size);
	}

	size_t page_buffer::read(size_t offset, void* buffer, size_t buffer_size) const
	{
		if (offset > size())
			return 0;  // TODO: raise exception?
		char* p = (char*)buffer;
		const auto size_to_read = std::min(size() - offset, buffer_size);
		const char* end = p + size_to_read;
		const auto page_size = this->page_size();
		size_t page_index, page_offset;
		locate(offset, page_index, page_offset);
		for (;;)
		{
			const auto n = std::min(page_size - page_offset, (size_t)(end - p));
			memcpy(p, (const char*)page_ptr(page_index) + page_offset, n);
			p += n;
			if (p >= end)
				break;
			++page_index;
			page_offset = 0;
		}
		return size_to_read;
	}

	void page_buffer::locate(size_t offset, size_t& ret_page_index, size_t& ret_page_offset) const
	{
		ret_page_index = offset >> page_size_max_bits();
		ret_page_offset = offset & (page_size_max() - 1);
	}

	size_t page_buffer::page_size() const
	{
		return std::min(capacity(), page_size_max());
	}

	unsigned char page_buffer::page_size_max_bits() const
	{
		return m_PagePool.PageSizeMaxBits();
	}

	size_t page_buffer::page_size_min() const
	{
		return m_PagePool.PageSizeMin();
	}

	size_t page_buffer::page_size_max() const
	{
		return m_PagePool.PageSizeMax();
	}

	void* page_buffer::page_ptr(size_t page_index)
	{
		return m_PagePool.PtrFromHandle(m_Pages[page_index]);
	}

	const void* page_buffer::page_ptr(size_t page_index) const
	{
		return m_PagePool.PtrFromHandle(m_Pages[page_index]);
	}

}

//#include "PageBufferStream.h"
//
//namespace uc
//{
//	class page_buffer_test
//	{
//	public:
//		page_buffer_test()
//		{
//			const char test_data[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//
//			TStdPageAllocator<32> allocator;
//			CPagePool pool(allocator, 3);
//			page_buffer buffer(pool);
//			{
//				page_buffer_ostream out(buffer, 0);
//				out.write("abc", 3);
//				out.write("123", 3);
//				out.write("aabbcc ", 7);
//				out.write(test_data, strlen(test_data));
//			}
//
//			char result[4096];
//			{
//				page_buffer_istream in(buffer, 0, buffer.size());
//				in.read(result, 3);
//				in.read(result + 3, sizeof(result) - 3);
//			}
//			int x = 1 + 2;
//		}
//	};
//
//	static page_buffer_test _page_buffer_test;
//}