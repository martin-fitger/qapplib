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
#include <bit>
#include <stdexcept>

#include <qapplib/Debug.h>

#include <qapplib/utils/Bits.h>
#include <qapplib/utils/PageStackAllocator.h>

namespace qapp
{
	page_stack_allocator_base::page_stack_allocator_base(CPagePool& page_pool)
		: m_PagePool(page_pool)
	{
	}

	page_stack_allocator_base::~page_stack_allocator_base()
	{
		clear();
	}

	void page_stack_allocator_base::clear()
	{
		while (!m_Pages.empty())
			pop_page();
	}

	void* page_stack_allocator_base::alloc(size_t size, size_t alignment)
	{
		if (!m_Pages.empty())
		{
			const auto page_offset = align_up(m_LastPageUsage, alignment);
			const auto avail = (ptrdiff_t)m_PagePool.PageSize(m_Pages.back()) - (ptrdiff_t)page_offset;
			if (avail >= (ptrdiff_t)size)
				return alloc_internal(page_offset, size);
		}
		add_page(size);
		return alloc_internal(m_LastPageUsage, size);
	}

	page_stack_allocator_base::state_t page_stack_allocator_base::state()
	{
		return (m_Pages.size() << 32) | m_LastPageUsage;
	}

	void page_stack_allocator_base::restore(state_t s)
	{
		const auto page_count = s >> 32;
		while (m_Pages.size() > page_count)
			pop_page();
		m_LastPageUsage = (unsigned int)s;
	}

	inline void* page_stack_allocator_base::last_page_ptr()
	{
		QAPP_ASSERT(!m_Pages.empty());
		return m_PagePool.PtrFromHandle(m_Pages.back());
	}

	void page_stack_allocator_base::add_page(size_t min_size)
	{
		const auto min_size_pow_2 = std::bit_ceil(min_size);
		if (min_size_pow_2 > m_PagePool.PageSizeMax())
			throw std::runtime_error("Requested allocation size is larger than max page size");

		size_t page_size = m_Pages.empty() ?
			m_PagePool.PageSizeMin() :
			std::min(m_PagePool.PageSize(m_Pages.back()) * 2, m_PagePool.PageSizeMax());
		
		page_size = std::max(min_size_pow_2, page_size);

		m_Pages.push_back(m_PagePool.Alloc((unsigned char)std::countr_zero(page_size)));
		
		m_LastPageUsage = 0;
	}

	void page_stack_allocator_base::pop_page()
	{
		m_PagePool.Free(m_Pages.back());
		m_Pages.pop_back();
		m_LastPageUsage = m_Pages.empty() ? 0 : (unsigned int)m_PagePool.PageSize(m_Pages.back());
	}

	void* page_stack_allocator_base::alloc_internal(size_t offset, size_t size)
	{
		auto* ptr = (char*)last_page_ptr() + offset;
		m_LastPageUsage = (unsigned int)(offset + size);
		return ptr;
	}
}