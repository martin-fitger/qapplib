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

#include <QtWidgets/qwidget.h>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/Editor.hpp>

namespace qapp
{
	const QString CEditor::Title() const
	{
		return "?";
	}

	const QIcon CEditor::Icon() const
	{
		return CDocumentManager::GenericDocumentIcon();
	}

	bool CEditor::Dirty() const
	{
		return false;
	}

	bool CEditor::CanClose() const
	{
		return true;
	}
	
	std::unique_ptr<QWidget> CEditor::CreateWidget(QWidget* parent)
	{
		return nullptr;
	}

	void CEditor::OnActivate()
	{
	}

	void CEditor::OnDeactivate()
	{
	}

	void CEditor::OnSaved()
	{
	}

	void CEditor::Export(QIODevice& out, const SDocumentTypeDesc& format)
	{
	}

	void CEditor::UpdateActions(CActionUpdateContext& ctx)
	{
	}
	
	bool CEditor::OnAction(HAction action_handle)
	{
		return false;
	}
}

#include <moc_Editor.cpp>