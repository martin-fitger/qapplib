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
#include <QtCore/qobject.h>

#include <qapplib/actions/ActionTarget.h>

class QIODevice;
class QWidget;

namespace qapp
{
	class CDocument;
	struct SDocumentTypeDesc;

	class IEditor: public IActionTarget
	{
	public:
		virtual const QString Title() const = 0;

		virtual const QIcon Icon() const = 0;

		virtual bool Dirty() const = 0;

		virtual bool CanClose() const = 0;

		virtual void OnActivate() = 0;

		virtual void OnDeactivate() = 0;

		virtual void OnSaved() = 0;

		virtual void Export(QIODevice& out, const SDocumentTypeDesc& format) = 0;

		virtual std::unique_ptr<QWidget> CreateWidget(QWidget* parent) = 0;
	};

	class CEditor : public QObject, public IEditor
	{
		Q_OBJECT
	public:
		// IEditor Interface
		const QString Title() const override;
		const QIcon Icon() const override;
		bool Dirty() const override;
		bool CanClose() const override;
		void OnActivate() override;
		void OnDeactivate() override;
		void OnSaved() override;
		void Export(QIODevice& out, const SDocumentTypeDesc& format) override;
		std::unique_ptr<QWidget> CreateWidget(QWidget* parent) override;

		// IActionTarget Interface
		void UpdateActions(CActionUpdateContext& ctx) override;
		bool OnAction(HAction action_handle) override;

	Q_SIGNALS:
		void DirtyChanged(bool modified);
	};
}