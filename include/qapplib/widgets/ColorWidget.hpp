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

#include <QtWidgets/qlineedit.h>

namespace qapp
{
	class CColorWidget : public QWidget
	{
		Q_OBJECT
	public:
		CColorWidget(QWidget* parent = nullptr, QColor color = QColorConstants::Black);

		inline QColor value() const { return m_Color; }

		void setValue(const QColor& color);

		void resizeEvent(QResizeEvent* event) override;

		void paintEvent(QPaintEvent* event) override;

		//void mouseReleaseEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;

	public:
		Q_SIGNALS:
		void editingFinished();
		void editingPreview();

	private Q_SLOTS:
		void OnEditingFinished();
		void OnColorPreview(const QColor& color);

	private:
		inline QRect ColorRect() const { return QRect(0, 0, m_ColorWidth, height()); }
		
		void OnColorClick();

		static QString ColorToString(QColor color);
	
		static bool TryParseColor(QString s, QColor& outColor);

		int m_ColorWidth = 0;
		int m_Padding = 0;
		QColor m_Color;
		QLineEdit* m_Edit = nullptr;
	};
}