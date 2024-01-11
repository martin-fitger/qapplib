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

#include <QtGui/qicon.h>

#include <qapplib/utils/PathUtils.h>
#include <qapplib/DocumentManager.hpp>

namespace qapp
{
	QIcon CDocumentManager::s_GenericDocumentIcon;

	CDocumentManager::CDocumentManager()
	{

	}
	
	CDocumentManager::~CDocumentManager()
	{

	}

	void CDocumentManager::RegisterDocumentType(const SDocumentTypeDesc& desc, IDocumentTypeHandler& handler)
	{
		m_DocumentTypes.push_back({ desc, &handler });
	}

	size_t CDocumentManager::MinIdBlockSize() const
	{
		size_t min_size = 0;
		for (const auto& document_type : m_DocumentTypes)
		{
			min_size = std::max(min_size, (size_t)document_type.Description.MinIdBlockSize);
		}
		return min_size;
	}

	const std::vector<SDocumentType>& CDocumentManager::DocumentTypes() const
	{
		return m_DocumentTypes;
	}

	QIcon CDocumentManager::IconFromPath(const QString& path)
	{
		if (auto* doc_type = TryGetDocumentTypeFromExtensionNoDot(ExtensionNoDotFromPath(path)))
		{
			if (!doc_type->Description.Icon.isNull())
			{
				return doc_type->Description.Icon;
			}
		}
		return GenericDocumentIcon();
	}

	const SDocumentType* CDocumentManager::TryGetDocumentTypeFromExtensionNoDot(QStringView extension_no_dot)
	{
		for (const auto& document_type : m_DocumentTypes)
		{
			if (document_type.Description.ExtensionNoDot.compare(extension_no_dot, Qt::CaseInsensitive) == 0)
			{
				return &document_type;
			}
		}
		return nullptr;
	}

	const SDocumentType* CDocumentManager::TryGetDocumentTypeFromIdBlock(const QByteArray& id_block)
	{
		for (const auto& document_type : m_DocumentTypes)
		{
			if (document_type.Handler->TryIdBlock(id_block))
			{
				return &document_type;
			}
		}
		return nullptr;
	}

	QIcon CDocumentManager::GenericDocumentIcon()
	{
		if (s_GenericDocumentIcon.isNull())
		{
			s_GenericDocumentIcon = QIcon(":/file.png");
		}
		return s_GenericDocumentIcon;
	}
}

#include <moc_DocumentManager.cpp>