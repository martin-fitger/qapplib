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

namespace qapp
{
	template <typename TVal, typename TAlign> TVal align_up(TVal val, TAlign alignment)
	{
		const auto align_mask = (TVal)(alignment - 1);
		return (val + align_mask) & ~align_mask;
	}

	template <typename T, class TLambda>
	inline static void for_each_set_bit(T bits, TLambda&& fn)
	{
		while (bits)
		{
			const auto bit_index = std::countr_zero(bits);
			bits &= ~((T)1 << bit_index);
			fn(bit_index);
		}
	}
}