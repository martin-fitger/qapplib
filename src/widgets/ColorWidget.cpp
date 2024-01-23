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

#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qstyleditemdelegate.h>

#include <qapplib/Color.h>
#include <qapplib/Debug.h>
#include <qapplib/utils/StringUtils.h>
#include <qapplib/widgets/ColorWidget.hpp>

namespace qapp
{
	CColorWidget::CColorWidget(QWidget* parent, QColor color)
		: QWidget(parent)
	{
		m_Edit = new QLineEdit(this);
		m_Edit->setStyleSheet("border: 0;");
		QAPP_VERIFY(connect(m_Edit, SIGNAL(editingFinished()), SLOT(OnEditingFinished())));
		setValue(color);
		setCursor(Qt::PointingHandCursor);
	}

	void CColorWidget::setValue(const QColor& color)
	{
		m_Color = color;
		m_Edit->setText(ColorToString(color));
		update(ColorRect());
	}

	void CColorWidget::resizeEvent(QResizeEvent* event)
	{
		const auto new_size = event->size();

		m_ColorWidth = new_size.height() * 2;

		m_Edit->setGeometry(m_ColorWidth + m_Padding, m_Padding, new_size.width() - m_ColorWidth + m_Padding, new_size.height() - m_Padding * 2);
	
		QWidget::resizeEvent(event);
	}

	void CColorWidget::paintEvent(QPaintEvent* event)
	{
        QPainter painter(this);
        const auto& rect = event->rect();

        QRect rcClient = QRect(0, 0, width(), height());

        // Get the frame width
        //const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        //rcClient.adjust(frameWidth, frameWidth, -frameWidth, -frameWidth);

        const auto background_color = palette().color(QPalette::Base);
        //const auto frame_color = palette().color(QPalette::Window);

        // Background
        painter.fillRect(rcClient.left(), rcClient.top(), m_ColorWidth, rcClient.height(), m_Color);
        painter.fillRect(rcClient.left() + m_ColorWidth, rcClient.top(), rcClient.width() - m_ColorWidth, rcClient.height(), background_color);
    }

	void CColorWidget::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton) 
		{
			OnColorClick();
		}

		// Call the base class implementation
		QWidget::mousePressEvent(event);
	}

	void CColorWidget::OnEditingFinished()
	{
		QColor color;
		if (!TryParseColor(m_Edit->text(), color))
		{
			m_Edit->setText(ColorToString(m_Color));
			return;
		}
		setValue(color);
		emit editingFinished();
	}

	void CColorWidget::OnColorPreview(const QColor& color)
	{
		setValue(color);
		emit editingPreview();
	}

	void CColorWidget::OnColorClick()
	{
		const QColor initialColor = m_Color;  // Save this so we can restore previewed colors on user cancellation.

		QColorDialog colorDialog(this);
		colorDialog.setCurrentColor(m_Color);
		colorDialog.setOption(QColorDialog::ShowAlphaChannel);
		colorDialog.setWindowTitle("Please pick a color");
		connect(&colorDialog, &QColorDialog::currentColorChanged, this, &CColorWidget::OnColorPreview);

		if (colorDialog.exec() != QDialog::Accepted)
		{
			OnColorPreview(initialColor);
			return;
		}

		setValue(colorDialog.selectedColor());

		emit editingFinished();
	}

	QString CColorWidget::ColorToString(QColor color)
	{
		QRgb rgba = color.rgba();
		char buf[10];
		if (qAlpha(rgba) == 0xFF)
		{
			snprintf(buf, sizeof(buf), "#%.2x%.2x%.2x", qRed(rgba), qGreen(rgba), qBlue(rgba));
		}
		else
		{
			snprintf(buf, sizeof(buf), "#%.2x%.2x%.2x%.2x", qAlpha(rgba), qRed(rgba), qGreen(rgba), qBlue(rgba));
		}
		return QString(buf);
	}
	
	bool CColorWidget::TryParseColor(QString s, QColor& outColor)
	{
		QRgb rgba;
		if (!qapp::TryParseColor(to_string_view(s), rgba))
		{
			return false;
		}
		outColor = QColor::fromRgba(rgba);
		return true;
	}
}

#include <moc_ColorWidget.cpp>