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

#include <iostream>

namespace qapp
{
	class IEditor;

	void DoOperations(IEditor& editor, std::istream& in);

	void UndoOperations(IEditor& editor, std::istream& in);

	enum EOperation
	{
		EOperation_Do,
		EOperation_Undo
	};

	typedef void(*FOperationProcessor)(IEditor& editor, EOperation op, std::istream& in, std::streamsize size);

	struct SOperationHeader
	{
		FOperationProcessor m_Processor = nullptr;
		std::streamsize     m_Size = 0;
	};

	template <class TLambda>
	void WriteOperation(std::ostream& out, FOperationProcessor processor, TLambda&& lambda)
	{
		const auto beg = out.tellp();
		SOperationHeader header;
		out.write((const char*)&header, sizeof(header));
		lambda(out);
		const auto end = out.tellp();
		header.m_Processor = processor;
		header.m_Size = end - beg - sizeof(header);
		out.seekp(beg);
		out.write((char*)&header, sizeof(header));
		out.seekp(end);
	}
}