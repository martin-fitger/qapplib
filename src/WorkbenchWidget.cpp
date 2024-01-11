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

#include <QtWidgets/qtabbar.h>
#include <QtWidgets/qtabwidget.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qmessagebox.h>

#include <qapplib/Debug.h>
#include <qapplib/Editor.hpp>
#include <qapplib/Workbench.hpp>
#include <qapplib/WorkbenchWidget.hpp>

namespace qapp
{
	// Hack to be able to access QTabWidget::tabBar()
	class CWorkbenchWidget::CTabWidget : public QTabWidget
	{
	public:
		CTabWidget(QWidget* parent = nullptr) : QTabWidget(parent) {}
		QTabBar* tabBar() { return QTabWidget::tabBar(); }
	};

	CWorkbenchWidget::CWorkbenchWidget(QWidget* parent, CWorkbench& workbench)
		: QWidget(parent)
		, m_Workbench(workbench)
	{
		// Connect workbench signals
		connect(&workbench, &CWorkbench::EditorAdded, this, &CWorkbenchWidget::OnEditorAdded);
		connect(&workbench, &CWorkbench::EditorRemoved, this, &CWorkbenchWidget::OnEditorRemoved);
		connect(&workbench, &CWorkbench::CurrentEditorChanged, this, &CWorkbenchWidget::OnCurrentEditorChanged);

		// Tabs
		m_Tabs = new CTabWidget(this);
		m_Tabs->setTabsClosable(true);
		m_Tabs->setContextMenuPolicy(Qt::CustomContextMenu);
		//m_Tabs->setExpanding(false);
		connect(m_Tabs->tabBar(), &QTabBar::customContextMenuRequested, this, &CWorkbenchWidget::OnTabContextMenu);
		connect(m_Tabs->tabBar(), &QTabBar::tabBarDoubleClicked, this, &CWorkbenchWidget::OnTabDoubleClick);
		connect(m_Tabs->tabBar(), &QTabBar::currentChanged, this, &CWorkbenchWidget::OnTabCurrentChanged);
		connect(m_Tabs->tabBar(), &QTabBar::tabCloseRequested, this, &CWorkbenchWidget::OnTabCloseRequested);
	
		m_Layout = new QVBoxLayout(this);
		m_Layout->setSpacing(0);
		m_Layout->setContentsMargins(0, 0, 0, 0);
		m_Layout->addWidget(m_Tabs, 0);
		//m_Layout->addStretch(1);
		setLayout(m_Layout);
	}
	
	CWorkbenchWidget::~CWorkbenchWidget()
	{
	}

	void CWorkbenchWidget::OnEditorAdded(CEditor& editor)
	{
		auto widget = editor.CreateWidget(m_Tabs);
		m_Editors.push_back(&editor);
		const int tab_index = m_Tabs->addTab(widget.release(), TextForTab(m_Tabs->count()));  // Move ownership here, we'll take it back on tab removal
		m_Tabs->setTabIcon(tab_index, editor.Icon());

		// Hook up signals from editor
		connect(&editor, &CEditor::DirtyChanged, this, &CWorkbenchWidget::OnEditorDirtyChanged);
	}

	void CWorkbenchWidget::OnEditorDirtyChanged(bool dirty)
	{
		auto* editor = dynamic_cast<CEditor*>(sender());
		if (!editor)
		{
			QAPP_ASSERT(false && "Expected CEditor object to be sender");
			return;
		}

		const auto tab_index = TabIndexFromEditor(*editor);
		if (tab_index < 0)
		{
			QAPP_ASSERT(false && "Got signal from editor without tab");
			return;
		}

		m_Tabs->setTabText(tab_index, TextForTab(tab_index));
	}

	void CWorkbenchWidget::OnEditorRemoved(CEditor& editor)
	{
		// Disconnect all signals from editor to this
		editor.disconnect(this);

		const auto tab_index = TabIndexFromEditor(editor);
		
		std::unique_ptr<QWidget> widget(m_Tabs->widget(tab_index));  // Delete on scope exit

		m_Editors.erase(m_Editors.begin() + tab_index);

		m_Tabs->removeTab(tab_index);
	}

	void CWorkbenchWidget::OnCurrentEditorChanged(CEditor* curr, CEditor* prev)
	{
		if (!curr)
		{
			return;
		}
		const auto tab_index = TabIndexFromEditor(*curr);
		if (m_Tabs->currentIndex() != tab_index)
		{
			m_Tabs->setCurrentIndex(tab_index);
		}
	}

	void CWorkbenchWidget::OnTabCurrentChanged(int index)
	{
		m_Workbench.SetCurrentEditor(index >= 0 ? m_Editors[index] : nullptr);
	}

	void CWorkbenchWidget::OnTabCloseRequested(int index)
	{
		auto& editor = *m_Editors[index];

		if (!editor.CanClose())
		{
			QMessageBox::information(this, "Can't close document", QString("Can't close document '") + editor.Title() + "' at the moment. Please wait.");
			return;
		}
		
		if (editor.Dirty())
		{
			const auto reply = QMessageBox::question(this, "Unsaved changes", QString("Save changes to '") + editor.Title() + "'?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (reply == QMessageBox::Cancel)
				return;
			if (reply == QMessageBox::Yes && !m_Workbench.Save(editor))
				return;
		}

		m_Workbench.RemoveEditor(editor);
	}

	void CWorkbenchWidget::OnTabContextMenu(const QPoint& pt)
	{
		//const int tab_index = m_Tabs->tabBar()->tabAt(pt);
		//if (tab_index < 0)
		//	return;
		//const auto pt_global = m_Tabs->tabBar()->mapToGlobal(pt);

		//QMenu* menu = new QMenu(this);

		//QAction* renameAction = new QAction("Rename", menu);
		//connect(renameAction, &QAction::triggered, this, [this, tab_index]() { this->OnRenameTab(tab_index); });
		//menu->addAction(renameAction);

		//menu->addSeparator();

		//QAction* closeAction = new QAction("Close", menu);
		//connect(closeAction, &QAction::triggered, this, [this, tab_index]() { this->OnTabCloseRequested(tab_index); });
		//menu->addAction(closeAction);

		//menu->setAttribute(Qt::WA_DeleteOnClose);
		//menu->popup(pt_global);
	}

	void CWorkbenchWidget::OnTabDoubleClick(int tab_index)
	{
		//OnRenameTab(tab_index);
	}

	int CWorkbenchWidget::TabIndexFromEditor(CEditor& editor) const
	{
		auto it = std::find(m_Editors.begin(), m_Editors.end(), &editor);
		return (m_Editors.end() == it) ? -1 : (int)(it - m_Editors.begin());
	}

	QString CWorkbenchWidget::TextForTab(int tab_index)
	{
		auto& editor = *m_Editors[tab_index];
		auto title = editor.Title();
		if (editor.Dirty())
			title += "*";
		m_Tabs->setTabText(tab_index, title);
		return title;
	}
}

#include <moc_WorkbenchWidget.cpp>