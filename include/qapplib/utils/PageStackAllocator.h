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
	template <class TBase>
	class allocator_utils: public TBase
	{
	public:
		template <typename... TArgs>
		inline allocator_utils(TArgs&&... args) : TBase(std::forward<TArgs>(args)...) {}

		template <class T, typename... TArgs>
		inline T* New(TArgs&&... args);
	};

	template <class TBase>
	template <class T, typename... TArgs>
	T* allocator_utils<TBase>::New(TArgs&&... args)
	{
		return new (TBase::alloc(sizeof(T), alignof(T))) T(std::forward<TArgs>(args)...);
	}

	class page_stack_allocator_base
	{
	public:
		typedef unsigned long long state_t;

		page_stack_allocator_base(CPagePool& page_pool);
		~page_stack_allocator_base();

		void clear();

		void* alloc(size_t size, size_t alignment);

		state_t state();

		void restore(state_t s);

	private:
		inline void* last_page_ptr();

		void add_page(size_t min_size);

		void pop_page();

		void* alloc_internal(size_t offset, size_t size);

		CPagePool& m_PagePool;
		unsigned int m_LastPageUsage = 0;
		std::vector<CPagePool::handle_t> m_Pages;
	};

	class page_stack_allocator : public allocator_utils<page_stack_allocator_base> 
	{
	public:
		template <typename... TArgs>
		inline page_stack_allocator(TArgs&&... args) : allocator_utils(std::forward<TArgs>(args)...) {}
	};
}