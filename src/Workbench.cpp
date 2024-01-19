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

#include <ranges>

#include <QtCore/qdir.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qfile.h>
#include <QtCore/qsettings.h>
#include <QtWidgets/qapplication.h>

#include <qapplib/actions/ActionManager.hpp>
#include <qapplib/actions/StandardActions.h>
#include <qapplib/utils/PathUtils.h>
#include <qapplib/UiUtils.h>
#include <qapplib/Document.hpp>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/Editor.hpp>
#include <qapplib/Workbench.hpp>

namespace qapp
{
	static const size_t MAX_RECENT_FILES_LIST_LENGTH = 8;

	static bool CanHandlerSaveExtension(IDocumentTypeHandler& handler, QStringView extension_no_dot)
	{
		for (const auto& type : handler.SupportedSaveFormats())
		{
			if (extension_no_dot.compare(type.ExtensionNoDot, Qt::CaseInsensitive) == 0)
			{
				return true;
			}
		}
		return false;
	}

	inline static QString CleanPath(const QString& path)
	{
		return QDir::toNativeSeparators(QDir::cleanPath(path));
	}

	// TODO: Improve by removing the extra allocation here
	static std::string ToUtf8String(const QString& s)
	{
		auto utf8Data = s.toUtf8();
		std::string_view utf8_string_view(utf8Data.constData(), utf8Data.size());
		return std::string(utf8_string_view);
	}

	CWorkbench::CWorkbench(CDocumentManager& document_manager, CActionManager& action_manager, QSettings& settings)
		: m_DocumentManager(document_manager)
		, m_ActionManager(action_manager)
		, m_Settings(settings)
	{
		m_ActionManager.AddActionTarget(this);
		LoadRecentFiles();
		RemoveMissingFilesFromRecentFiles();
	}
		
	CWorkbench::~CWorkbench()
	{
	}

	void CWorkbench::AddEditor(std::unique_ptr<CEditor> editor, std::shared_ptr<CDocument> document, IDocumentTypeHandler* handler)
	{
		m_Editors.push_back({ std::move(editor), std::move(document), handler });
		emit EditorAdded(*m_Editors.back().Editor);
	}

	void CWorkbench::RemoveEditor(CEditor& editor)
	{
		for (auto it = m_Editors.begin(); m_Editors.end() != it; ++it)
		{
			if (it->Editor.get() == &editor)
			{
				SEditor ed = std::move(*it);
				m_Editors.erase(it);
				if (m_CurrentEditor == &editor)
				{
					SetCurrentEditor(nullptr);
				}
				emit EditorRemoved(editor);
				return;
			}
		}
		throw std::runtime_error("Trying to remove editor that doesn't exist");
	}

	void CWorkbench::SetCurrentEditor(CEditor* editor)
	{
		if (editor == m_CurrentEditor)
			return;

		if (m_CurrentEditor)
		{
			m_ActionManager.RemoveActionTarget(m_CurrentEditor);
			m_CurrentEditor->OnDeactivate();
		}

		auto* prev_editor = m_CurrentEditor;
		m_CurrentEditor = editor;

		if (m_CurrentEditor)
		{
			m_CurrentEditor->OnActivate();
			m_ActionManager.AddActionTarget(m_CurrentEditor);
		}

		emit CurrentEditorChanged(m_CurrentEditor, prev_editor);
	}

	IEditor* CWorkbench::CurrentEditor() const
	{
		return m_CurrentEditor;
	}

	CEditor* CWorkbench::EditorFromPath(const QString& path) const
	{
		auto clean_path = CleanPath(path);
		for (auto& editor : m_Editors)
		{
			if (editor.Document->Path() == clean_path)
			{
				return editor.Editor.get();
			}
		}
		return nullptr;
	}

	void CWorkbench::New(IDocumentTypeHandler& document_type_handler)
	{
		std::shared_ptr<CDocument> document(document_type_handler.NewDocument().release());
		auto editor = document_type_handler.CreateEditor(*document);
		AddEditor(std::move(editor), std::move(document), &document_type_handler);
		SetCurrentEditor(m_Editors.back().Editor.get());
	}

	void CWorkbench::Open(QString path)
	{
		path = CleanPath(path);

		auto* editor = EditorFromPath(path);
		
		if (nullptr == editor)
		{
			QFile file(path);
			if (!file.open(QIODevice::ReadOnly))
			{
				throw std::runtime_error(ToUtf8String(file.errorString()));
			}

			auto* document_type = m_DocumentManager.TryGetDocumentTypeFromExtensionNoDot(ExtensionNoDotFromPath(path));
			if (!document_type)
			{
				const size_t min_id_block_size = m_DocumentManager.MinIdBlockSize();
				auto id_block = file.peek(min_id_block_size);
				document_type = m_DocumentManager.TryGetDocumentTypeFromIdBlock(id_block);
			}

			if (!document_type)
			{
				throw std::runtime_error("Unsupported file format");
			}

			std::shared_ptr<CDocument> document(document_type->Handler->LoadDocument(file).release());
			document->SetPath(path);
			auto unique_editor = document_type->Handler->CreateEditor(*document);
			AddEditor(std::move(unique_editor), std::move(document), document_type->Handler);
			editor = m_Editors.back().Editor.get();
		}

		SetCurrentEditor(editor);

		AddToRecentFiles(path);
	}

	bool CWorkbench::Save(IEditor& editor)
	{
		auto& editor_entry = m_Editors[EditorIndex(editor)];
		
		if (!editor_entry.Document.get() || !editor_entry.Handler)
		{
			return false;
		}

		if (!QFileInfo(editor_entry.Document->Path()).isAbsolute() || !CanHandlerSaveExtension(*editor_entry.Handler, ExtensionNoDotFromPath(editor_entry.Document->Path())))
		{
			return SaveAs(editor);
		}

		return Save(editor, editor_entry.Document->Path());
	}

	bool CWorkbench::SaveAs(IEditor& editor)
	{
		auto& editor_entry = m_Editors[EditorIndex(editor)];

		if (!editor_entry.Document.get() || !editor_entry.Handler)
		{
			return false;
		}

		auto supported_types = editor_entry.Handler->SupportedSaveFormats();
		if (supported_types.empty())
		{
			return false;
		}

		QString initialPath = editor_entry.Document->Path();

		if (!QFileInfo(initialPath).isAbsolute())
		{
			initialPath = QDir(this->LastDirectory()).filePath(editor_entry.Document->Title());
		}

		QString path = qapp::GetSaveFileName(QApplication::activeWindow(), supported_types, initialPath);
		if (path.isEmpty())
		{
			return false;
		}

		return Save(editor, path);
	}

	bool CWorkbench::Save(IEditor& editor, QString path)
	{
		path = CleanPath(path);

		QFileInfo fileInfo(path);
		const bool path_is_real = fileInfo.isAbsolute() || fileInfo.isRelative();
		
		if (!path_is_real)
		{
			return SaveAs(editor);
		}

		auto& editor_entry = m_Editors[EditorIndex(editor)];

		if (!editor_entry.Document.get() || !editor_entry.Handler)
		{
			return false;
		}

		auto supported_types = editor_entry.Handler->SupportedSaveFormats();
		if (supported_types.empty())
		{
			throw std::runtime_error("Can not save this file type");
		}
		const SDocumentTypeDesc* current_type = &supported_types.front();
		const auto extension_no_dot = ExtensionNoDotFromPath(path);
		for (const auto& type : supported_types)
		{
			if (extension_no_dot.compare(type.ExtensionNoDot, Qt::CaseInsensitive) == 0)
			{
				current_type = &type;
			}
		}

		if (!current_type)
		{
			throw std::runtime_error("Unsupported file extension");
		}

		{
			QFile file(path);
			if (!file.open(QIODevice::WriteOnly))
			{
				throw std::runtime_error(ToUtf8String(file.errorString()));
			}
			editor_entry.Handler->SaveDocument(*editor_entry.Document, file, *current_type);
		}

		if (editor_entry.Document->Path() != path)
		{
			editor_entry.Document->SetPath(path);
		}

		AddToRecentFiles(path);

		editor.OnSaved();

		return true;
	}

	const std::vector<QString>& CWorkbench::RecentFiles() const
	{
		return m_RecentFiles;
	}

	const QString CWorkbench::LastDirectory() const
	{
		if (auto* doc = CurrentDocument())
		{
			if (QFileInfo(doc->Path()).isAbsolute())
			{
				return QFileInfo(doc->Path()).absolutePath();
			}
		}
		if (!m_RecentFiles.empty())
		{
			return QFileInfo(m_RecentFiles.front()).absolutePath();
		}
		return QString();
	}

	void CWorkbench::UpdateActions(CActionUpdateContext& ctx)
	{
		ctx.Enable(s_StandardActionHandles.New);
		ctx.Enable(s_StandardActionHandles.Open);
		if (m_CurrentEditor && m_CurrentEditor->Dirty())
		{
			ctx.Enable(s_StandardActionHandles.Save);
			ctx.Enable(s_StandardActionHandles.SaveAs);
		}
	}

	void CWorkbench::OnAction(HAction action_handle)
	{
		if (s_StandardActionHandles.New == action_handle)
		{
			New(*m_DocumentManager.DocumentTypes().front().Handler);
		}
		else if (m_CurrentEditor)
		{
			if (action_handle == s_StandardActionHandles.Save)
			{
				Save(*m_CurrentEditor);
			}
			if (action_handle == s_StandardActionHandles.SaveAs)
			{
				SaveAs(*m_CurrentEditor);
			}
		}
	}

	CDocument* CWorkbench::CurrentDocument() const
	{
		if (m_CurrentEditor)
		{
			for (auto& editor : m_Editors)
			{
				if (editor.Editor.get() == m_CurrentEditor)
				{
					return editor.Document.get();
				}
			}
		}
		return nullptr;
	}

	void CWorkbench::AddToRecentFiles(QString path)
	{
		path = CleanPath(path);

		for (size_t i = 0; i < m_RecentFiles.size(); ++i)
		{
			if (m_RecentFiles[i].compare(path, qapp::PlatformPathCaseSensitivity()) == 0)
			{
				m_RecentFiles.erase(m_RecentFiles.begin() + i); 
				--i;
			}
		}

		while (m_RecentFiles.size() >= MAX_RECENT_FILES_LIST_LENGTH)
		{
			m_RecentFiles.pop_back();
		}

		m_RecentFiles.insert(m_RecentFiles.begin(), path);

		SaveRecentFiles();

		emit RecentFilesChanged();
	}

	void CWorkbench::LoadRecentFiles()
	{
		m_RecentFiles.clear();
		const int size = m_Settings.beginReadArray("recent_files");
		for (int i = 0; i < size; ++i)
		{
			m_Settings.setArrayIndex(i);
			m_RecentFiles.push_back(m_Settings.value("path").toString());
		}
		m_Settings.endArray();
	}

	void CWorkbench::RemoveMissingFilesFromRecentFiles()
	{
		bool modified = false;
		for (size_t i = 0; i < m_RecentFiles.size(); ++i)
		{
			if (!QFile(m_RecentFiles[i]).exists())
			{
				m_RecentFiles.erase(m_RecentFiles.begin() + i);
				--i;
				modified = true;
			}
		}
	}

	void CWorkbench::SaveRecentFiles() const
	{
		m_Settings.beginWriteArray("recent_files");
		for (size_t i = 0; i < m_RecentFiles.size(); ++i)
		{
			m_Settings.setArrayIndex((int)i);
			m_Settings.setValue("path", m_RecentFiles[i]);
		}
		m_Settings.endArray();
	}

	int CWorkbench::EditorIndex(IEditor& editor) const
	{
		for (auto& entry : m_Editors)
		{
			if (entry.Editor.get() == &editor)
			{
				return (int)(&entry - m_Editors.data());
			}
		}
		return -1;
	}

}

#include <moc_Workbench.cpp>