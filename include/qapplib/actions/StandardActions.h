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

#include "ActionTarget.h"

class QAction;
class QObject;

namespace qapp
{
	class CActionManager;

	struct SStandardActions
	{
		// Main
		QAction* New = 0;
		QAction* Open = 0;
		QAction* Save = 0;
		QAction* SaveAs = 0;
		QAction* Export = 0;
		QAction* Exit = 0;
		QAction* About = 0;

		// Edit
		QAction* Undo = 0;
		QAction* Redo = 0;
		QAction* Cut = 0;
		QAction* Copy = 0;
		QAction* Paste = 0;
		QAction* Duplicate = 0;
		QAction* Delete = 0;
		QAction* SelectAll = 0;
	};

	struct SStandardActionHandles
	{
		// Main
		HAction New = 0;
		HAction Open = 0;
		HAction Save = 0;
		HAction SaveAs = 0;
		HAction Export = 0;
		HAction Exit = 0;
		HAction About = 0;

		// Edit
		HAction Undo = 0;
		HAction Redo = 0;
		HAction Cut = 0;
		HAction Copy = 0;
		HAction Paste = 0;
		HAction Duplicate = 0;
		HAction Delete = 0;
		HAction SelectAll = 0;
	};

	extern SStandardActions s_StandardActions;
	extern SStandardActionHandles s_StandardActionHandles;

	void InitStandardActions(QObject* parent, CActionManager& action_manager);
}