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

#pragma once

#include <bit>
#include <vector>
#include <qapplib/Debug.h>
#include "PageAllocator.h"

namespace qapp
{
	class CPagePool
	{
	public:
		typedef void* handle_t;
		
		CPagePool(IPageAllocator& page_allocator, unsigned char page_size_min_bits = 12);
		~CPagePool();

		inline unsigned char PageSizeMinBits() const { return m_PageSizeMinBits; }

		inline unsigned char PageSizeMaxBits() const { return m_PageSizeMaxBits; }

		inline size_t PageSizeMin() const { return SizeFromBits(PageSizeMinBits()); }

		inline size_t PageSizeMax() const { return SizeFromBits(PageSizeMaxBits()); }

		inline static size_t SizeFromBits(unsigned char size_bits) { return (size_t)1 << size_bits; }

		handle_t Alloc(unsigned char page_size_bits);

		void Free(handle_t handle);

		inline void* PtrFromHandle(handle_t handle) const { return (void*)(reinterpret_cast<uintptr_t>(handle) & ~PageSizeMask()); }

		inline size_t PageSize(handle_t handle) const { return (size_t)1 << PageSizeBitsFromHandle(handle); }

		inline static CPagePool& DefaultPagePool() { QAPP_ASSERT(s_DefaultPagePool); return *s_DefaultPagePool; }

		inline static void SetDefaultPagePool(CPagePool* page_pool) { s_DefaultPagePool = page_pool; }

	private:
		unsigned char SizeIndexFromSizeBits(unsigned char size_bits) const;
		
		unsigned char PageSizeBitsFromHandle(handle_t handle) const;

		handle_t CreateHandle(void* ptr, unsigned char page_size_bits) const;

		handle_t AllocInternal(unsigned char page_size_bits);

		inline size_t PageSizeMask() const { return std::bit_ceil((uint8_t)(PageSizeMaxBits() - PageSizeMinBits())) - 1; }

		IPageAllocator&       m_PageAllocator;
		const unsigned char   m_PageSizeMinBits;
		const unsigned char   m_PageSizeMaxBits;
		std::vector<void*>    m_AllocatedPages;  // Pages allocated from IPageAllocator that are used internally in this object (splitted into smaller pages)
		std::vector<handle_t> m_FreePages[16];

		static CPagePool* s_DefaultPagePool;
	};
}