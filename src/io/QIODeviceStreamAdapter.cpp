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

#include <qapplib/io/QIODeviceStreamAdapter.h>
#include <qapplib/Debug.h>

namespace qapp
{
	std::streamsize QIODeviceStreamBuf::xsgetn(char* s, std::streamsize n)
	{
		return m_Device.read(s, n);
	}

	std::streamsize QIODeviceStreamBuf::xsputn(const char* s, std::streamsize n)
	{
		return m_Device.write(s, n);
	}

	// TODO: Remove?
	std::streambuf::int_type QIODeviceStreamBuf::underflow()
	{
		QAPP_ASSERT(false && "QIODeviceStreamBuf::underflow not implemented!");
		return std::streambuf::underflow();
	}

	// TODO: Remove?
	std::streambuf::int_type QIODeviceStreamBuf::overflow(std::streambuf::int_type c)
	{
		QAPP_ASSERT(false && "QIODeviceOStream::streambuf::overflow not implemented!");
		return std::streambuf::overflow(c);
	}

	std::streampos QIODeviceStreamBuf::seekoff(std::streamoff offs, std::ios_base::seekdir dir, std::ios_base::openmode mode)
	{
		switch (dir)
		{
		case std::ios::beg:
			return seekpos(offs, mode);
		case std::ios::cur:
			return seekpos(m_Device.pos() + offs, mode);
		case std::ios::end:
			return seekpos(m_Device.size() + offs, mode);
		}
		return std::streampos(std::streamoff(-1));
	}

	std::streampos QIODeviceStreamBuf::seekpos(std::streampos pos, std::ios_base::openmode)
	{
		return m_Device.seek(pos) ? pos : std::streampos(std::streamoff(-1));
	}
}