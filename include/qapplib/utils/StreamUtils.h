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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ranges>

namespace qapp
{
	inline void read_exact(std::istream& in, void* ptr, size_t size)
	{
		in.read((char*)ptr, size);
		if (in.gcount() != size)
		{
			if (in.eof())
				throw std::runtime_error("Unexpected end of file reached when trying to read data");
			throw std::runtime_error("Error occurred when trying to read data");
		}
	}

	template <typename T>
	inline bool tread(std::istream& in, T& obj)
	{
		in.read((char*)&obj, sizeof(T));
		return in.gcount() == sizeof(T);
	}

	template <typename T>
	inline T tread(std::istream& in)
	{
		T obj;
		read_exact(in, &obj, sizeof(obj));
		return obj;
	}

	template <typename T>
	inline bool twrite(std::ostream& out, const T& obj)
	{
		out.write((char*)&obj, sizeof(T));
		return !out.bad();
	}
	
	template <std::ranges::input_range TRange>
	inline bool write_range(std::ostream& out, const TRange& values)
	{
		for (const auto& value : values)
		{
			out.write((char*)&value, sizeof(value));
		}
		return !out.bad();
	}

	template <typename T, size_t TBufferSize = 64*1024, class TLambda>
	inline void for_each_in_stream(std::istream& in, TLambda&& lambda)
	{
		T batch[TBufferSize / sizeof(T)];
		for (;;)
		{
			in.read((char*)batch, sizeof(batch));
			const size_t count = in.gcount() / sizeof(T);
			if (!count)
			{
				if (in.eof())
					return;
				throw std::runtime_error("Stream read error");
			}
			for (size_t i = 0; i < count; ++i)
				lambda(batch[i]);
		}
	}

	template <typename T, size_t TBufferSize = 64 * 1024, class TLambda>
	inline void for_each_in_stream(std::istream& in, size_t count, TLambda&& lambda)
	{
		T batch[TBufferSize / sizeof(T)];
		while (count)
		{
			const size_t n = std::min(_countof(batch), count);
			in.read((char*)batch, n * sizeof(T));
			if (in.gcount() != n * sizeof(T))
			{
				if (in.eof())
					throw std::runtime_error("End of stream reached when reading objects");
				throw std::runtime_error("Stream read error");
			}
			for (size_t i = 0; i < n; ++i)
				lambda(batch[i]);
			count -= n;
		}
	}

	template <class TLambda>
	void for_each_line(std::istream& in, size_t max_line_length, TLambda&& lambda)
	{
		auto* buf = (char*)alloca(max_line_length + 1);
		char* end = buf;
		for (;;)
		{
			in.read(end, max_line_length - (end - buf));
			const size_t nread = in.gcount();
			if (0 == nread)
			{
				if (!in.eof())
					throw std::runtime_error("I/O error");
				if (end > buf)
				{
					*end = 0;
					lambda(buf, end - buf);
				}
				return;
			}
			char* p = end;
			end += nread;
			char* line_beg = buf;
			for (; p < end; ++p)
			{
				if ('\n' == *p)
				{
					auto* line_end = p;
					if (line_end > line_beg && '\r' == *(line_end - 1))
						--line_end;
					*line_end = 0;
					lambda(line_beg, line_end - line_beg);
					line_beg = p + 1;
				}
			}
			if (line_beg < end && (line_beg == buf))
			{
				char buf[1024];
				sprintf_s(buf, "Too long line in file. Current limit is %zu characters.", max_line_length);
				throw std::runtime_error(buf);
			}
			const size_t chars_remaining = end - line_beg;
			if (chars_remaining)
				memmove(buf, line_beg, chars_remaining);
			end = buf + chars_remaining;
		}

	}

	class CBufferedReader
	{
	public:
		CBufferedReader(std::istream& in, void* buffer, size_t buffer_size)
			: m_Cur((char*)buffer), m_End((char*)buffer), m_In(in), m_Buffer((char*)buffer), m_BufferSize(buffer_size) {}

		template <typename T>
		inline T Get() 
		{ 
			if (BufferedSize() < sizeof(T)) 
				Fetch(sizeof(T)); 
			const auto val = *(T*)m_Cur; 
			m_Cur += sizeof(T);
			return val;
		}

		template <class TLambda>
		void WithData(size_t size, TLambda&& lambda)
		{
			if (BufferedSize() < size)
				Fetch(size);
			lambda(m_Cur);
			m_Cur += size;
		}

		void ReadExact(void* buf, size_t size)
		{
			char* p = (char*)buf;
			char* p_end = p + size;
			for (;;)
			{
				const size_t remaining_to_read = p_end - p;
				const size_t buffered_size = BufferedSize();
				const size_t n = remaining_to_read < buffered_size ? remaining_to_read : buffered_size;
				memcpy(p, m_Cur, n);
				m_Cur += n;
				p += n;
				if (p >= p_end)
					return;
				Fetch(1);
			}
		}

	private:
		void Fetch(size_t expected_min)
		{
			Fetch();
			if (BufferedSize() < expected_min)
				throw std::runtime_error("Unexpected end of file reached");
		}

		void Fetch()
		{
			const auto unread = m_End - m_Cur;
			if (unread)
			{
				memmove(m_Buffer, m_Cur, unread);
			}
			m_Cur = m_Buffer;
			m_End = m_Buffer + unread;
			m_In.read(m_End, m_BufferSize - unread);
			m_End += m_In.gcount();
		}

		inline size_t BufferedSize() const { return m_End - m_Cur; }

		std::istream& m_In;
		char* m_Cur = nullptr;
		char* m_End = nullptr;
		char* m_Buffer;
		size_t m_BufferSize;
	};

	template <size_t TBufferSize>
	class TBufferedReader : public CBufferedReader
	{
	public:
		TBufferedReader(std::istream& in) : CBufferedReader(in, m_Buffer, TBufferSize) {}
	private:
		unsigned long long m_Buffer[(TBufferSize + 7) / 8];
	};


	class imemstream : public std::istream
	{
	private:
		class streambuf : public std::streambuf
		{
		public:
			streambuf(const void* beg, const void* end) { setg((char*)beg, (char*)beg, (char*)end); }

		protected:
			std::streamsize xsgetn(char* s, std::streamsize n) override
			{
				n = std::min((std::streamsize)(egptr() - gptr()), n);
				memcpy(s, gptr(), n);
				gbump((int)n);
				return n;
			}

			std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in) override
			{
				if (std::ios_base::in != which)
					return -1;
				auto* p = gptr();
				switch (way)
				{
				case std::ios_base::beg: p = eback(); break;
				case std::ios_base::end: p = egptr(); break;
				case std::ios_base::cur: break;
				default:
					throw std::runtime_error("Unsupported seek direction");
				}
				return seek(p + off);
			}

			std::streampos seekpos(std::streampos sp, std::ios_base::openmode which = std::ios_base::in) override
			{
				if (std::ios_base::in != which)
					return -1;
				return seek(eback() + sp);
			}

		private:
			std::streampos seek(char* p)
			{
				if (p < eback())
					p = eback();
				else if (p > egptr())
					p = egptr();
				setg(eback(), p, egptr());
				return (std::streampos)(gptr() - eback());
			}
		};

	public:
		imemstream(const void* beg, const void* end) : std::istream(&m_StreamBuf), m_StreamBuf(beg, end) {}

	private:
		streambuf m_StreamBuf;
	};
}