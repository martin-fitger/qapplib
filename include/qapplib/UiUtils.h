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

#include <QtCore/qstring.h>

class QWidget;

namespace qapp
{
	class CDocumentManager;
	struct SDocumentTypeDesc;

	QString GetOpenFileName(QWidget* parent, CDocumentManager& document_manager, QString initial_directory = QString());

	QString GetSaveFileName(QWidget* parent, const std::span<const SDocumentTypeDesc>& types, QString initial_path = QString());
}