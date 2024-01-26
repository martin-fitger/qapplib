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

#include <stdexcept>
#include <QtCore/qpoint.h>
#include <QtCore/qvariant.h>

namespace qapp
{
	class QVariantEx
	{
	public:
		enum Type
		{
			Float = QVariant::UserType + 1,
		};
	};

	template <typename T> QVariant::Type QVariantType();

	template <> inline QVariant::Type QVariantType<int>()          { return QVariant::Int; }
	template <> inline QVariant::Type QVariantType<unsigned int>() { return QVariant::UInt; }
	template <> inline QVariant::Type QVariantType<float>()        { return (QVariant::Type)QVariantEx::Float; }
	template <> inline QVariant::Type QVariantType<double>()       { return QVariant::Double; }
	template <> inline QVariant::Type QVariantType<QPointF>()      { return QVariant::PointF; }

	inline QString QVariantTypeToString(QVariant::Type type)
	{
		switch (type)
		{
		case QVariant::Int:
			return QString("Int");
		case QVariant::UInt:
			return QString("UInt");
		case (QVariant::Type)QVariantEx::Float:
			return QString("Float");
		case QVariant::Double:
			return QString("Double");
		case QVariant::String:
			return QString("String");
		case QVariant::PointF:
			return QString("PointF");
		}
		throw std::runtime_error("Unsupported QVariant type");
	}

	inline QVariant::Type QVariantTypeFromString(const QString& s)
	{
		if (s == "Int")
			return QVariant::Int;
		if (s == "UInt")
			return QVariant::UInt;
		if (s == "Float")
			return (QVariant::Type)QVariantEx::Float;
		if (s == "Double")
			return QVariant::Double;
		if (s == "String")
			return QVariant::String;
		if (s == "PointF")
			return QVariant::PointF;
		return QVariant::Invalid;
	}

	inline size_t QVariantTypeSize(QVariant::Type type)
	{
		switch (type)
		{
		case QVariant::Int:
			return sizeof(int32_t);
		case QVariant::UInt:
			return sizeof(uint32_t);
		case (QVariant::Type)QVariantEx::Float:
			return sizeof(float);
		case QVariant::Double:
			return sizeof(double);
		case QVariant::PointF:
			return sizeof(QPointF);
		}
		throw std::runtime_error("Unsupported QVariant type");
	}
}
