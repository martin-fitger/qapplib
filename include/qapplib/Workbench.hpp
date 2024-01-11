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

#include <memory>
#include <vector>
#include <QtCore/qobject.h>
#include <qapplib/actions/ActionTarget.h>

class QSettings;

namespace qapp
{
	class CActionManager;
	class CDocumentManager;
	class CDocument;
	class IEditor;
	class CEditor;
	class IDocumentTypeHandler;

	class CWorkbench: public QObject, public IActionTarget
	{
		Q_OBJECT
	public:
		CWorkbench(CDocumentManager& document_manager, CActionManager& action_manager, QSettings& settings);
		~CWorkbench();

		void AddEditor(std::unique_ptr<CEditor> editor, std::shared_ptr<CDocument> document, IDocumentTypeHandler* handler);

		void RemoveEditor(CEditor& editor);

		void SetCurrentEditor(CEditor* editor);

		IEditor* CurrentEditor() const;

		CEditor* EditorFromPath(const QString& path) const;

		void New(IDocumentTypeHandler& document_type_handler);

		void Open(QString path);

		bool Save(IEditor& editor);

		bool SaveAs(IEditor& editor);

		bool Save(IEditor& editor, QString path);

		const std::vector<QString>& RecentFiles() const;

		const QString LastDirectory() const;

		// IActionTarget interface
		void UpdateActions(CActionUpdateContext& ctx) override;
		void OnAction(HAction action_handle) override;

	Q_SIGNALS:
		void RecentFilesChanged();
		void EditorAdded(CEditor& editor);
		void EditorRemoved(CEditor& editor);
		void CurrentEditorChanged(CEditor* curr, CEditor* prev);

	private:
		struct SEditor
		{
			std::unique_ptr<CEditor> Editor;
			std::shared_ptr<CDocument> Document;
			IDocumentTypeHandler* Handler = nullptr;
		};
		
		CDocument* CurrentDocument() const;

		void AddToRecentFiles(QString path);
		void LoadRecentFiles();
		void RemoveMissingFilesFromRecentFiles();
		void SaveRecentFiles() const;
		int EditorIndex(IEditor& editor) const;

		CDocumentManager& m_DocumentManager;
		CActionManager& m_ActionManager;
		QSettings& m_Settings;
		CEditor* m_CurrentEditor = nullptr;
		std::vector<SEditor> m_Editors;
		std::vector<QString> m_RecentFiles;
	};
}