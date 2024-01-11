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

#include "PagePool.h"

namespace qapp
{
	class page_buffer
	{
	public:
		page_buffer(CPagePool& page_pool = CPagePool::DefaultPagePool());
		~page_buffer();

		void clear();

		inline bool empty() const { return 0 == size(); }

		void resize(size_t size);

		void reserve(size_t size);

		void append(const void* data, size_t size);

		void write(size_t offset, const void* data, size_t size);

		size_t read(size_t offset, void* buffer, size_t buffer_size) const;

		inline size_t size() const { return m_Size; }

	private:
		inline size_t capacity() const { return m_Capacity; }

		void locate(size_t offset, size_t& ret_page_index, size_t& ret_page_offset) const;

		size_t page_size() const;

		unsigned char page_size_max_bits() const;

		size_t page_size_min() const;

		size_t page_size_max() const;

		void* page_ptr(size_t page_index);
		const void* page_ptr(size_t page_index) const;

		CPagePool& m_PagePool;
		size_t m_Size = 0;
		size_t m_Capacity = 0;
		std::vector<CPagePool::handle_t> m_Pages;
	};
}