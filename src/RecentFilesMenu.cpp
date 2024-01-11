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

#include <qapplib/DocumentManager.hpp>
#include <qapplib/RecentFilesMenu.hpp>
#include <qapplib/Workbench.hpp>

namespace qapp
{
	CRecentFilesMenu::CRecentFilesMenu(QWidget* parent, CDocumentManager& document_manager, CWorkbench& workbench)
		: QMenu(parent)
		, m_DocumentManager(document_manager)
		, m_Workbench(workbench)
	{
		setTitle("Recent Files");
		RebuildMenu();
		connect(&workbench, &CWorkbench::RecentFilesChanged, this, &CRecentFilesMenu::OnRecentFilesChanged);
	}

	CRecentFilesMenu::~CRecentFilesMenu()
	{
		for (auto* action : actions())
		{
			removeAction(action);
		}
	}

	void CRecentFilesMenu::OnRecentFilesChanged()
	{
		RebuildMenu();
	}

	void CRecentFilesMenu::RebuildMenu()
	{
		const auto& recent_files = m_Workbench.RecentFiles();

		for (auto* action : actions())
		{
			removeAction(action);
		}
		
		if (recent_files.empty())
		{
			if (!m_NoFilesAction)
			{
				m_NoFilesAction = std::make_unique<QAction>(this);
				m_NoFilesAction->setText("No Recent Files");
				m_NoFilesAction->setEnabled(false);
			}
			addAction(m_NoFilesAction.get());
			return;
		}

		for (int file_index = 0; file_index < (int)recent_files.size(); ++file_index)
		{
			const auto& path = recent_files[file_index];
			
			if (file_index >= (size_t)m_Actions.size())
			{
				auto action = std::make_unique<QAction>(this);
				auto* workbench = &m_Workbench;
				connect(action.get(), &QAction::triggered, [workbench, file_index]() { workbench->Open(workbench->RecentFiles()[file_index]); });
				m_Actions.push_back(std::move(action));
			}

			auto* action = m_Actions[file_index].get();

			QFileInfo fileInfo(path);
			action->setText(QString("%1\t(%2)").arg(fileInfo.fileName()).arg(path));

			action->setIcon(m_DocumentManager.IconFromPath(path));

			addAction(action);
		}
	}
}

#include <moc_RecentFilesMenu.cpp>