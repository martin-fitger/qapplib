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

#include <string_view>
#include <QtGui/qrgb.h>

namespace qapp
{
	template <typename TValue, typename TChar>
	inline bool TryParseHexValue(std::basic_string_view<TChar> s, TValue& out_value)
	{
		TValue result = 0;
		TValue valid = 1;
		for (TChar c : s)
		{
			const TValue val0 = (TValue)((int)c - '0');
			const TValue val0_mask = (TValue)(val0 > 9) - 1;
			const TValue val1 = (TValue)((int)c - 'A');
			const TValue val1_mask = (TValue)(val1 > 5) - 1;
			const TValue val2 = (TValue)((int)c - 'a');
			const TValue val2_mask = (TValue)(val2 > 5) - 1;
			result <<= 4;
			result |= (val0 & val0_mask) | ((val1 + 10) & val1_mask) | ((val2 + 10) & val2_mask);
			valid &= val0_mask | val1_mask | val2_mask;
		}
		out_value = result;
		return valid != 0;
	}

	template <typename TChar>
	inline bool TryParseColor(std::basic_string_view<TChar> s, QRgb& out_rgba)
	{
		// Skip optional '#'-prefix
		if (!s.empty() && (TChar)'#' == s[0])
		{
			s = std::basic_string_view<TChar>(s.data() + 1, s.size() - 1);
		}

		if (s.length() == 3)
		{
			uint32_t val;
			if (!TryParseHexValue(s, val))
			{
				return false;
			}
			auto dupl_hex = [](uint8_t x) { return x | x << 4; };
			uint8_t a = 0xFF;
			uint8_t r = dupl_hex((uint8_t)(val >> 8));
			uint8_t g = dupl_hex((uint8_t)(val >> 4) & 0xF);
			uint8_t b = dupl_hex((uint8_t)val & 0xF);
			out_rgba = qRgba(r, g, b, a);
			return true;
		}
		else if (s.length() == 6)
		{
			uint32_t val;
			if (!TryParseHexValue(s, val))
			{
				return false;
			}
			uint8_t a = 0xFF;
			uint8_t r = (uint8_t)(val >> 16);
			uint8_t g = (uint8_t)(val >> 8);
			uint8_t b = (uint8_t)val;
			out_rgba = qRgba(r, g, b, a);
			return true;
		}
		else if (s.length() == 8)
		{
			uint32_t val;
			if (!TryParseHexValue(s, val))
			{
				return false;
			}
			uint8_t a = (uint8_t)(val >> 24);
			uint8_t r = (uint8_t)(val >> 16);
			uint8_t g = (uint8_t)(val >> 8);
			uint8_t b = (uint8_t)val;
			out_rgba = qRgba(r, g, b, a);
			return true;
		}
		return false;
	}
}