/*
 * ROuterSpaceCommandWidget.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: dan
 */

#include "ROuterSpaceCommandWidget.h"
#include "make_msg.h"
#include <QColorDialog>

ROuterSpaceCommandWidget::ROuterSpaceCommandWidget()
: RCommandWidget()
, ui(new Ui::OuterSpaceCommandForm())
{
	ui->setupUi(this);
	ui->dotsPerBlock->setValidator(new QIntValidator(1, 1000000));
	ui->poolSize->setValidator(new QIntValidator(100, 10000));
	ui->blockSizeX->setValidator(new QIntValidator(1, 1000000));
	ui->blockSizeY->setValidator(new QIntValidator(1, 1000000));
	ui->blockSizeZ->setValidator(new QIntValidator(1, 1000000));

	// initial values
	m_outerspace.block_size_x = 1000;
	m_outerspace.block_size_y = 1000;
	m_outerspace.block_size_z = 1000;
	m_outerspace.dots_per_block = 250;
	m_outerspace.pool_size = 500;
	m_outerspace.dot_size = 2.0;
	m_outerspace.flags = 0;
	m_outerspace.dot_r = m_outerspace.dot_g = m_outerspace.dot_b = m_outerspace.dot_a = 255;

	initialize();

	// connection
	connect(ui->pbColor, SIGNAL(clicked()), this, SLOT(colorClicked()));

}


void ROuterSpaceCommandWidget::initialize()
{
	ui->dotsPerBlock->setText(QString("%1").arg(m_outerspace.dots_per_block));
	ui->poolSize->setText(QString("%1").arg(m_outerspace.pool_size));
	ui->blockSizeX->setText(QString("%1").arg(m_outerspace.block_size_x));
	ui->blockSizeY->setText(QString("%1").arg(m_outerspace.block_size_y));
	ui->blockSizeZ->setText(QString("%1").arg(m_outerspace.block_size_z));
	ui->dotSize->setValue(m_outerspace.dot_size);
	setDotColor(QColor((int)m_outerspace.dot_r, (int)m_outerspace.dot_g, (int)m_outerspace.dot_b));
	ui->cbFlagColorTest->setChecked(m_outerspace.flags & OUTERSPACE_COLOR_TEST);
	ui->cbFlagGridTest->setChecked(m_outerspace.flags & OUTERSPACE_GRID_TEST);
}

void ROuterSpaceCommandWidget::setDotColor(QColor c)
{
	m_dotColor = c;
	ui->lbColor->setText(QString("(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue()));
}

void ROuterSpaceCommandWidget::colorClicked()
{
	QColor c = QColorDialog::getColor(m_dotColor, this);
	if (c.isValid()) setDotColor(c);
}

render::MessageStruct& ROuterSpaceCommandWidget::getCommand(render::MessageStruct& msg)
{
	m_outerspace.block_size_x = ui->blockSizeX->text().toInt();
	m_outerspace.block_size_y = ui->blockSizeY->text().toInt();
	m_outerspace.block_size_z = ui->blockSizeZ->text().toInt();
	m_outerspace.dot_size = ui->dotSize->value();
	m_outerspace.dots_per_block = ui->dotsPerBlock->text().toInt();
	m_outerspace.pool_size = ui->poolSize->text().toInt();
	m_outerspace.flags = 0;
	if (ui->cbFlagColorTest->isChecked()) m_outerspace.flags |= OUTERSPACE_COLOR_TEST;
	if (ui->cbFlagGridTest->isChecked()) m_outerspace.flags |= OUTERSPACE_GRID_TEST;
	m_outerspace.dot_r = (unsigned char)m_dotColor.red();
	m_outerspace.dot_g = (unsigned char)m_dotColor.green();
	m_outerspace.dot_b = (unsigned char)m_dotColor.blue();
	m_outerspace.dot_a = (unsigned char)m_dotColor.alpha();

	return *make_outerspace_msg_s(&msg, &m_outerspace);
}






