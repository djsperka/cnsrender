/*
 * RBackgroundCommandWidget.cpp
 *
 *  Created on: Aug 6, 2015
 *      Author: dan
 */

#include "RBackgroundCommandWidget.h"
#include "make_msg.h"

RBackgroundCommandWidget::RBackgroundCommandWidget()
: RCommandWidget()
{
	m_background.a = (unsigned char)256;
	m_background.r = m_background.g = m_background.b = (unsigned char)0;
	components();
	connections();
}

void RBackgroundCommandWidget::components()
{
	m_colorDialog = new QColorDialog();
	m_colorDialog->setWindowFlags(Qt::Widget);
	m_colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
	m_colorDialog->setCurrentColor(QColor(m_background.r, m_background.g, m_background.b, m_background.a));
	QVBoxLayout* v = new QVBoxLayout;
	v->addWidget(m_colorDialog);
	setLayout(v);
	return;
}

void RBackgroundCommandWidget::connections()
{
	return;
}

render::MessageStruct& RBackgroundCommandWidget::getCommand(render::MessageStruct& msg)
{
	QColor color = m_colorDialog->currentColor();
	m_background.r = color.red();
	m_background.g = color.green();
	m_background.b = color.blue();
	m_background.a = color.alpha();
	return *make_background_msg_s(&msg, &m_background);
}

