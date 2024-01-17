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

#include <QtWidgets/qfiledialog.h>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/UiUtils.h>

namespace qapp
{
	QString GetOpenFileName(QWidget* parent, CDocumentManager& document_manager, QString initial_directory)
	{
		// Build filter string
		QString filter;
		if (!document_manager.DocumentTypes().empty())
		{
			size_t numLoadableDocumentTypes = 0;
			filter = "All supported files (";
			for (const auto& doc_type : document_manager.DocumentTypes())
			{
				if (!(doc_type.Description.Capabilities & SDocumentTypeDesc::Caps_Load))
				{
					continue;
				}
				if (!filter.isEmpty())
				{
					filter += " ";
				}
				filter += "*.";
				filter += doc_type.Description.ExtensionNoDot;
				++numLoadableDocumentTypes;
			}
			filter += ")";

			if (numLoadableDocumentTypes <= 1)
			{
				filter.clear();
			}

			for (const auto& doc_type : document_manager.DocumentTypes())
			{
				if (!(doc_type.Description.Capabilities & SDocumentTypeDesc::Caps_Load))
				{
					continue;
				}
				if (!filter.isEmpty())
				{
					filter += ";;";
				}
				filter += doc_type.Description.Name;
				filter += " Files (*.";
				filter += doc_type.Description.ExtensionNoDot;
				filter += ")";
			}
		}

		if (!filter.isEmpty())
		{
			filter += ";;";
		}
		filter += "All files (*)";

		return QFileDialog::getOpenFileName(parent, "Open File", initial_directory, filter);
	}

	QString GetSaveFileName(QWidget* parent, const std::span<const SDocumentTypeDesc>& types, QString initial_path)
	{
		// Build filter string
		QString filter;
		for (const auto& desc : types)
		{
			if (!filter.isEmpty())
			{
				filter += ";;";
			}
			filter += desc.Name;
			filter += " File (*.";
			filter += desc.ExtensionNoDot;
			filter += ")";
		}

		return QFileDialog::getSaveFileName(parent, "Save File", initial_path, filter);
	}
}