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


#include <QtCore/qstring.h>
#include <QtWidgets/qaction.h>

#include <qapplib/actions/ActionManager.hpp>
#include <qapplib/actions/StandardActions.h>

namespace qapp
{
	SStandardActions s_StandardActions;
	SStandardActionHandles s_StandardActionHandles;

	// Helper
	static QAction* NewAction(QString title, QString icon_path, const QKeySequence& keys, QObject* parent, bool enabled)
	{
		auto* action = new QAction(icon_path.isEmpty() ? QIcon() : QIcon(icon_path), title, parent);
		action->setShortcut(keys);
		auto tooltip = title;
		tooltip.replace("&", "");
		if (!keys.isEmpty())
			tooltip += " (" + keys.toString() + ")";
		action->setToolTip(tooltip);
		action->setEnabled(enabled);
		return action;
	}

	void InitStandardActions(QObject* parent, CActionManager& action_manager)
	{
		#define DEF_ACTION(member, name, img, keys, enabled) \
			s_StandardActions.member = NewAction(name, img, (keys == 0) ? QKeySequence() : QKeySequence(keys), parent, enabled); \
			s_StandardActionHandles.member = action_manager.RegisterAction(s_StandardActions.member);
		DEF_ACTION(New,       "&New",        ":/new.png",     Qt::CTRL + Qt::Key_N, false);
		DEF_ACTION(Open,      "&Open",       ":/open.png",    Qt::CTRL + Qt::Key_O, false);
		DEF_ACTION(Save,      "&Save",       ":/save.png",    Qt::CTRL + Qt::Key_S, false);
		DEF_ACTION(SaveAs,    "Save As...",  ":/save_as.png", 0,                    false);
		DEF_ACTION(Undo,      "&Undo",       ":/undo.png",    Qt::CTRL + Qt::Key_Z, false);
		DEF_ACTION(Redo,      "&Redo",       ":/redo.png",    Qt::CTRL + Qt::Key_Y, false);
		DEF_ACTION(Cut,       "Cu&t",        ":/cut.png",     Qt::CTRL + Qt::Key_X, false);
		DEF_ACTION(Copy,      "&Copy",       ":/copy.png",    Qt::CTRL + Qt::Key_C, false);
		DEF_ACTION(Paste,     "&Paste",      ":/paste.png",   Qt::CTRL + Qt::Key_V, false);
		DEF_ACTION(Delete,    "&Delete",     ":/delete.png",  Qt::Key_Delete,       false);
		DEF_ACTION(SelectAll, "Select &All", "",              Qt::CTRL + Qt::Key_A, false);
		DEF_ACTION(Exit,      "E&xit",       ":/exit.png",    Qt::CTRL + Qt::Key_Q, false);
		DEF_ACTION(About,     "About",       ":/about.png",   Qt::Key_F1,           false);
	}
}