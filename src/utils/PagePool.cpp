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

#include <qapplib/Debug.h>
#include <qapplib/utils/PagePool.h>

namespace qapp
{
	CPagePool* CPagePool::s_DefaultPagePool = nullptr;

	CPagePool::CPagePool(IPageAllocator& page_allocator, unsigned char page_size_min_bits)
		: m_PageAllocator(page_allocator)
		, m_PageSizeMinBits(page_size_min_bits)
		, m_PageSizeMaxBits((unsigned char)std::countr_zero(page_allocator.PageSize()))
	{
		QAPP_ASSERT(PageSizeMaxBits() - PageSizeMinBits() + 1 <= _countof(m_FreePages));
	}

	CPagePool::~CPagePool()
	{
		for (auto page : m_AllocatedPages)
			m_PageAllocator.FreePage(page);
		m_AllocatedPages.clear();
	}
	
	CPagePool::handle_t CPagePool::Alloc(unsigned char page_size_bits)
	{
		auto& v = m_FreePages[SizeIndexFromSizeBits(page_size_bits)];
		if (v.empty())
			return AllocInternal(page_size_bits);
		const auto page_handle = v.back();
		v.pop_back();
		return page_handle;
	}

	void CPagePool::Free(handle_t handle)
	{
		m_FreePages[SizeIndexFromSizeBits(PageSizeBitsFromHandle(handle))].push_back(handle);
	}

	unsigned char CPagePool::SizeIndexFromSizeBits(unsigned char size_bits) const
	{
		QAPP_ASSERT(size_bits >= m_PageSizeMinBits);
		QAPP_ASSERT(size_bits <= m_PageSizeMaxBits);
		return size_bits - m_PageSizeMinBits;
	}

	unsigned char CPagePool::PageSizeBitsFromHandle(handle_t handle) const
	{
		return PageSizeMinBits() + (unsigned char)(reinterpret_cast<uintptr_t>(handle) & PageSizeMask());
	}

	CPagePool::handle_t CPagePool::CreateHandle(void* ptr, unsigned char page_size_bits) const
	{
		const auto page_size_bit_range = PageSizeMaxBits() - PageSizeMinBits() + 1;
		QAPP_ASSERT(((size_t)ptr & PageSizeMask()) == 0);
		return (void*)(reinterpret_cast<uintptr_t>(ptr) | SizeIndexFromSizeBits(page_size_bits));
	}

	CPagePool::handle_t CPagePool::AllocInternal(unsigned char page_size_bits)
	{
		if (page_size_bits == PageSizeMaxBits())
		{
			auto* ptr = m_PageAllocator.AllocPage();
			return CreateHandle(ptr, page_size_bits);
		}
		const auto parent_handle = Alloc(page_size_bits + 1);
		const auto parent_ptr = PtrFromHandle(parent_handle);
		if (page_size_bits + 1 == PageSizeMaxBits())
			m_AllocatedPages.push_back(parent_ptr);
		m_FreePages[SizeIndexFromSizeBits(page_size_bits)].push_back(CreateHandle((char*)parent_ptr + SizeFromBits(page_size_bits), page_size_bits));
		return CreateHandle((char*)parent_ptr, page_size_bits);
	}
}