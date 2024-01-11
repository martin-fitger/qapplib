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

#include <algorithm>
#include <qapplib/Debug.h>
#include <qapplib/utils/PageBufferStream.h>

namespace qapp
{
	// page_buffer_istream

	page_buffer_istream::page_buffer_istream(const page_buffer& buffer, size_t beg, size_t end)
		: std::istream(&m_StreamBuf)
		, m_StreamBuf(buffer, beg, end)
	{
	}

	page_buffer_istream::streambuf::streambuf(const page_buffer& buffer, size_t beg, size_t end)
		: m_Buffer(&buffer)
		, m_Beg(beg)
		, m_Cur(beg)
		, m_End(end)
	{}

	std::streamsize page_buffer_istream::streambuf::xsgetn(char* s, std::streamsize n)
	{
		const auto num_bytes_read = m_Buffer->read(m_Cur, s, std::min((size_t)n, m_End - m_Cur));
		m_Cur += num_bytes_read;
		return num_bytes_read;
	}

	std::streambuf::int_type page_buffer_istream::streambuf::underflow()
	{
		if (m_Cur >= m_End)
			return EOF;
		char c;
		m_Buffer->read(m_Cur, &c, 1);
		return (m_Buffer->read(m_Cur, &c, 1) == 1) ? traits_type::to_int_type(c) : EOF;
	}

	std::streampos page_buffer_istream::streambuf::seekoff(std::streamoff off, ios_base::seekdir way, ios_base::openmode which)
	{
		QAPP_ASSERT(ios_base::in == which);
		std::streampos sp = 0;
		switch (way)
		{
		case ios_base::beg: sp = 0; break;
		case ios_base::cur: sp = (std::streampos)m_Cur - (std::streampos)m_Beg; break;
		case ios_base::end: sp = m_End - m_Beg; break;
		}
		return seekpos(sp + off, which);
	}

	std::streampos page_buffer_istream::streambuf::seekpos(std::streampos sp, ios_base::openmode which)
	{
		QAPP_ASSERT(ios_base::in == which);
		m_Cur = std::min(m_Beg + std::max((std::streampos)0, sp), m_End);
		return m_Cur - m_Beg;
	}


	// page_buffer_ostream

	page_buffer_ostream::page_buffer_ostream(page_buffer& buffer, size_t beg)
		: std::ostream(&m_StreamBuf)
		, m_StreamBuf(buffer, beg)
	{
	}

	page_buffer_ostream::streambuf::streambuf(page_buffer& buffer, size_t beg)
		: m_Buffer(&buffer)
		, m_Beg(beg)
		, m_Cur(beg)
	{}

	std::streamsize page_buffer_ostream::streambuf::xsputn(const char* s, std::streamsize n)
	{
		m_Buffer->write(m_Cur, s, n);
		m_Cur += n;
		return n;
	}

	std::streambuf::int_type page_buffer_ostream::streambuf::overflow(std::streambuf::int_type c)
	{
		char tmp = c;
		m_Buffer->write(m_Cur, &tmp, 1);
		return traits_type::to_int_type(c);
	}

	std::streampos page_buffer_ostream::streambuf::seekoff(std::streamoff off, ios_base::seekdir way, ios_base::openmode which)
	{
		QAPP_ASSERT(ios_base::out == which);
		std::streampos sp = 0;
		switch (way)
		{
		case ios_base::beg: sp = 0; break;
		case ios_base::cur: sp = (std::streampos)m_Cur - (std::streampos)m_Beg; break;
		case ios_base::end: sp = m_Buffer->size() - m_Beg; break;
		}
		return seekpos(sp + off, which);
	}

	std::streampos page_buffer_ostream::streambuf::seekpos(std::streampos sp, ios_base::openmode which)
	{
		QAPP_ASSERT(ios_base::out == which);
		m_Cur = m_Beg + std::max((std::streampos)0, sp);
		return m_Cur - m_Beg;
	}
}