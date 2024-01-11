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
#include "PageBuffer.h"

namespace qapp
{
	class page_buffer_istream : public std::istream
	{
	public:
		page_buffer_istream(const page_buffer& buffer, size_t beg, size_t end);

	private:
		class streambuf : public std::streambuf
		{
		public:
			streambuf(const page_buffer& buffer, size_t beg, size_t end);

		protected:
			std::streamsize xsgetn(char* s, std::streamsize n) override;
			std::streambuf::int_type underflow() override;
			std::streampos seekoff(std::streamoff off, ios_base::seekdir way, ios_base::openmode which) override;
			std::streampos seekpos(std::streampos sp, ios_base::openmode which) override;

			const page_buffer* m_Buffer = nullptr;
			size_t m_Beg = 0;
			size_t m_Cur = 0;
			size_t m_End = 0;
		};

		streambuf m_StreamBuf;
	};

	class page_buffer_ostream : public std::ostream
	{
	public:
		page_buffer_ostream(page_buffer& buffer, size_t beg = 0);

	private:
		class streambuf : public std::streambuf
		{
		public:
			streambuf(page_buffer& buffer, size_t beg);

		protected:
			std::streamsize xsputn(const char* s, std::streamsize n) override;
			std::streambuf::int_type overflow(std::streambuf::int_type c) override;
			std::streampos seekoff(std::streamoff off, ios_base::seekdir way, ios_base::openmode which) override;
			std::streampos seekpos(std::streampos sp, ios_base::openmode which) override;

			page_buffer* m_Buffer = nullptr;
			size_t m_Beg = 0;
			size_t m_Cur = 0;
		};

		streambuf m_StreamBuf;
	};
}