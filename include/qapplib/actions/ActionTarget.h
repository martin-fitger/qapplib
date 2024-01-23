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

namespace qapp
{
	typedef unsigned char HAction;
	static const HAction INVALID_ACTION_HANDLE = 0;

	class CActionUpdateContext;

	class IActionTarget
	{
	public:
		virtual void UpdateActions(CActionUpdateContext& ctx) = 0;
		virtual bool OnAction(HAction action_handle) = 0;
	};

	class CActionUpdateContext
	{
	public:
		CActionUpdateContext(size_t& enable_bits)
			: m_EnableBits(enable_bits)
		{}

		inline void Enable(HAction handle)
		{
			const auto index = IndexFromHandle(handle);
			m_EnableBits |= ((size_t)1 << index);
		}

	private:
		inline static size_t IndexFromHandle(HAction handle) { return (size_t)handle - 1; }

		size_t& m_EnableBits;
	};
}