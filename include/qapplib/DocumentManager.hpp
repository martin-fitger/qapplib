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

#include <span>
#include <QtGui/qicon.h>

class QIODevice;

namespace qapp
{
	class CDocument;
	class CEditor;

	struct SDocumentTypeDesc
	{
		enum ECapabilities : uint32_t
		{
			Caps_None = 0,
			Caps_New = 1 << 0,
			Caps_Load = 1 << 1,
		};

		QString  Name;
		QIcon    Icon;
		QString  FileNamePrefix;
		QString  ExtensionNoDot;
		uint32_t Capabilities = Caps_None;
		uint32_t MinIdBlockSize = 0;
	};

	class IDocumentTypeHandler
	{
	public:
		virtual std::unique_ptr<CDocument> NewDocument() = 0;
		virtual std::unique_ptr<CDocument> LoadDocument(QIODevice& in) = 0;
		virtual std::span<const SDocumentTypeDesc> SupportedSaveFormats() = 0;
		virtual std::span<const SDocumentTypeDesc> SupportedExportFormats() = 0;
		virtual void                       SaveDocument(CDocument& document, QIODevice& out, const SDocumentTypeDesc& format) = 0;
		virtual std::unique_ptr<CEditor>   CreateEditor(CDocument& document) = 0;
		virtual bool                       TryIdBlock(const QByteArray& id_block) = 0;
	};

	struct SDocumentType
	{
		SDocumentTypeDesc Description;
		IDocumentTypeHandler* Handler;
	};

	// TODO: Rename to CDocumentTypeRegistry?

	class CDocumentManager: public QObject
	{
		Q_OBJECT
	public:
		CDocumentManager();
		~CDocumentManager();

		void RegisterDocumentType(const SDocumentTypeDesc& desc, IDocumentTypeHandler& handler);

		size_t MinIdBlockSize() const;

		const std::vector<SDocumentType>& DocumentTypes() const;

		QIcon IconFromPath(const QString& path);

		const SDocumentType* TryGetDocumentTypeFromExtensionNoDot(QStringView extension_no_dot);

		const SDocumentType* TryGetDocumentTypeFromIdBlock(const QByteArray& id_block);

		static QIcon GenericDocumentIcon();

	private:
		std::vector<SDocumentType> m_DocumentTypes;
		static QIcon s_GenericDocumentIcon;
	};
}