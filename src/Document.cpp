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

#include <QtCore/qfileinfo.h>
#include <qapplib/Document.hpp>

namespace qapp
{
	QString CDocument::Title() const
	{
		return QFileInfo(m_Path).baseName();
	}

	const QString& CDocument::Path() const
	{
		return m_Path;
	}

	void CDocument::SetPath(QString path)
	{
		m_Path = path;
	}
}

#include <moc_Document.cpp>