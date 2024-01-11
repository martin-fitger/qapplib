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

#include <vector>
#include <QtWidgets/qwidget.h>

class QTabBar;
class QVBoxLayout;

namespace qapp
{
	class CEditor;
	class CWorkbench;

	class CWorkbenchWidget: public QWidget
	{
		Q_OBJECT
	public:
		CWorkbenchWidget(QWidget* parent, CWorkbench& workbench);
		~CWorkbenchWidget();

	Q_SIGNALS:

	private Q_SLOTS:
		void OnEditorAdded(CEditor& editor);
		void OnEditorDirtyChanged(bool dirty);
		void OnEditorRemoved(CEditor& editor);
		void OnCurrentEditorChanged(CEditor* curr, CEditor* prev);
		void OnTabCurrentChanged(int index);
		void OnTabCloseRequested(int index);
		void OnTabContextMenu(const QPoint& pt);
		void OnTabDoubleClick(int tab_index);

	private:
		int TabIndexFromEditor(CEditor& editor) const;
		QString TextForTab(int tab_index);

		class CTabWidget;
		CWorkbench& m_Workbench;
		CTabWidget* m_Tabs = nullptr;
		QVBoxLayout* m_Layout = nullptr;
		std::vector<CEditor*> m_Editors;
	};
}