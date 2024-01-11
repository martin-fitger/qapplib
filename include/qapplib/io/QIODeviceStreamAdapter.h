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
#include <QtCore/qiodevice.h>

namespace qapp
{
	class QIODeviceStreamBuf : public std::streambuf
	{
	public:
		QIODeviceStreamBuf(QIODevice& device) : m_Device(device) {}
		// std::streambuf overrides
		std::streamsize xsgetn(char* s, std::streamsize n) override;
		std::streamsize xsputn(const char* s, std::streamsize n) override;
		std::streambuf::int_type underflow() override;
		std::streambuf::int_type overflow(std::streambuf::int_type c) override;
		std::streampos seekoff(std::streamoff offs, std::ios_base::seekdir dir, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override;
		std::streampos seekpos(std::streampos pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override;
	private:
		QIODevice& m_Device;
	};

	class QIODeviceIStream : public std::istream
	{
	public:
		QIODeviceIStream(QIODevice& device) : std::istream(&m_StreamBuf), m_StreamBuf(device) {}
	private:
		QIODeviceStreamBuf m_StreamBuf;
	};

	class QIODeviceOStream : public std::ostream
	{
	public:
		QIODeviceOStream(QIODevice& device) : std::ostream(&m_StreamBuf), m_StreamBuf(device)  {}
	private:
		QIODeviceStreamBuf m_StreamBuf;
	};
}