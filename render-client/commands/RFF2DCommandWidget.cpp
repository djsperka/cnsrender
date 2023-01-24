/*
 * RFF2DCommandWidget.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: dan
 */

#include "RFF2DCommandWidget.h"
#include "make_msg.h"
#include <climits>
#include <QColorDialog>

RFF2DCommandWidget::RFF2DCommandWidget()
: RCommandWidget()
, ui(new Ui::FF2DCommandForm())
{
	ui->setupUi(this);

	// validators
	ui->leX->setValidator(new QIntValidator(-2000, 2000));
	ui->leY->setValidator(new QIntValidator(-2000, 2000));
	ui->leCSeed->setValidator(new QIntValidator(-INT_MAX, INT_MAX));
	ui->lePSeed->setValidator(new QIntValidator(-INT_MAX, INT_MAX));

	ui->leV->setValidator(new QDoubleValidator(0, 100, 1));
	ui->leW->setValidator(new QDoubleValidator(0, 100, 1));
	ui->leDir->setValidator(new QDoubleValidator(0, 360, 0));

	// initial values
	m_ff2d.x = m_ff2d.y = 0;
	m_ff2d.r = m_ff2d.g = m_ff2d.b = 0;
	m_ff2d.a = 255;
	m_ff2d.linear = 1;
	m_ff2d.cseed = 1234;
	m_ff2d.pseed = 5678;
	m_ff2d.angle = 0;
	m_ff2d.width = 0;
	m_ff2d.npts = 100;
	m_ff2d.pixsz = 2;
	m_ff2d.prob = 100;
	m_ff2d.v = 1;
	m_ff2d.depth = 1;
	m_ff2d.radius = 100;
	initialize();

	// connection
	connect(ui->pbColor, SIGNAL(clicked()), this, SLOT(colorClicked()));

}


void RFF2DCommandWidget::initialize()
{
	ui->rbLinear->setChecked(m_ff2d.linear != 0);
	ui->sbNpts->setValue(m_ff2d.npts);
	ui->slProb->setValue(m_ff2d.prob);
	ui->leR->setText(QString("%1").arg(m_ff2d.radius));
	ui->lePSeed->setText("5678");
	ui->leCSeed->setText("1234");
	ui->sbPixsz->setValue(m_ff2d.pixsz);
	ui->sbDepth->setValue(m_ff2d.depth);
	ui->leX->setText(QString("%1").arg(m_ff2d.x));
	ui->leY->setText(QString("%1").arg(m_ff2d.y));
	ui->leV->setText(QString("%1").arg(m_ff2d.v));
	ui->leW->setText(QString("%1").arg(m_ff2d.width));
	ui->leDir->setText(QString("%1").arg(m_ff2d.angle));
	setDotColor(QColor((int)m_ff2d.r, (int)m_ff2d.g, (int)m_ff2d.b, (int)m_ff2d.a));
	ui->cbTest->setChecked(false);
}

void RFF2DCommandWidget::setDotColor(QColor c)
{
	m_dotColor = c;
	ui->lbColor->setText(QString("(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue()));
}

void RFF2DCommandWidget::colorClicked()
{
	QColor c = QColorDialog::getColor(m_dotColor, this);
	if (c.isValid()) setDotColor(c);
}

render::MessageStruct& RFF2DCommandWidget::getCommand(render::MessageStruct& msg)
{
	m_ff2d.linear = ui->rbLinear->isChecked() ? 1 : 0;
	m_ff2d.npts = ui->sbNpts->value();
	m_ff2d.prob = (float)ui->slProb->value();
	m_ff2d.pseed = ui->lePSeed->text().toInt();
	m_ff2d.cseed = ui->leCSeed->text().toInt();
	m_ff2d.pixsz = ui->sbPixsz->value();
	m_ff2d.depth = ui->sbDepth->value();
	m_ff2d.radius = ui->leR->text().toDouble();
	m_ff2d.x = ui->leX->text().toInt();
	m_ff2d.y = ui->leY->text().toInt();
	m_ff2d.v = ui->leV->text().toDouble();
	m_ff2d.width = ui->leW->text().toDouble();
	m_ff2d.angle = ui->leDir->text().toDouble();
	m_ff2d.r = (unsigned char)m_dotColor.red();
	m_ff2d.g = (unsigned char)m_dotColor.green();
	m_ff2d.b = (unsigned char)m_dotColor.blue();
	m_ff2d.a = (unsigned char)m_dotColor.alpha();

	return *make_ff2d_msg_s(&msg, &m_ff2d);
}






