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

#include <qapplib/commands/Operations.h>

namespace qapp
{
	void DoOperations(IEditor& editor, std::istream& in)
	{
		for (;;)
		{
			SOperationHeader header;
			in.read((char*)&header, sizeof(header));
			if (in.gcount() != sizeof(header))
			{
				if (in.eof())
					break;
				throw std::runtime_error("Operation stream error");
			}
			const auto end = in.tellg() + header.m_Size;
			header.m_Processor(editor, EOperation_Do, in, header.m_Size);
			in.seekg(end);
		}
	}

	void UndoOperations(IEditor& editor, std::istream& in)
	{
		SOperationHeader header;
		in.read((char*)&header, sizeof(header));
		if (in.gcount() != sizeof(header))
		{
			if (in.eof())
			{
				in.clear();  // Clear eof-bit etc, to be able to read again
				return;
			}
			throw std::runtime_error("Operation stream error");
		}
		const auto at = in.tellg();
		in.seekg(header.m_Size, std::ios::cur);
		UndoOperations(editor, in);
		in.seekg(at);
		header.m_Processor(editor, EOperation_Undo, in, header.m_Size);
	}
}