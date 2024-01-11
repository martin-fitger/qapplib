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

#include <QtWidgets/qmenu.h>

namespace qapp
{
	class CDocumentManager;
	class CWorkbench;

	class CRecentFilesMenu: public QMenu
	{
		Q_OBJECT
	public:
		CRecentFilesMenu(QWidget* parent, CDocumentManager& document_manager, CWorkbench& workbench);
		~CRecentFilesMenu();

	private Q_SLOTS:
		void OnRecentFilesChanged();

	private:
		void RebuildMenu();

		CWorkbench& m_Workbench;
		CDocumentManager& m_DocumentManager;
		std::vector<std::unique_ptr<QAction>> m_Actions;
		std::unique_ptr<QAction> m_NoFilesAction;
	};
}