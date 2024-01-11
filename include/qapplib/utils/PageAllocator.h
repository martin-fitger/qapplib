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

#include <memory>
#include <stdlib.h>

// TODO: Move
#ifdef _MSC_VER
	#include <malloc.h>
#else
	#include <stdlib.h>
#endif

namespace qapp
{
	inline void* aligned_alloc(size_t alignment, size_t size)
	{
		#ifdef _MSC_VER
			return _aligned_malloc(size, alignment);
		#else
			return ::aligned_alloc(alignment, size);
		#endif
	}

	inline void aligned_free(void* ptr)
	{
		#ifdef _MSC_VER
			return _aligned_free(ptr);
		#else
			return ::free(ptr);
		#endif
	}

	class IPageAllocator
	{
	public:
		virtual ~IPageAllocator() {}
		virtual size_t PageSize() const = 0;
		virtual void*  AllocPage() = 0;
		virtual void   FreePage(void* page) = 0;
	};

	template <size_t TPageSize>
	class TStdPageAllocator: public IPageAllocator
	{
	public:
		size_t PageSize() const override { return TPageSize; }
		void*  AllocPage() override { return qapp::aligned_alloc(TPageSize >= 4096 ? 4096 : TPageSize, TPageSize); }
		void   FreePage(void* page) override { qapp::aligned_free(page); };
	};
}