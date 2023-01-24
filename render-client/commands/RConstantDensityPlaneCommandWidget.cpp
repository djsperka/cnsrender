/*
 * RConstantDensityPlaneCommandWidget.cpp
 *
 *  Created on: Dec 11, 2017
 *      Author: dan
 */

#include "RConstantDensityPlaneCommandWidget.h"
#include <QColorDialog>
#include "make_msg.h"

RConstantDensityPlaneCommandWidget::RConstantDensityPlaneCommandWidget()
: RCommandWidget()
, ui(new Ui::ConstantDensityPlaneForm())
{
	ui->setupUi(this);

	// validators
	ui->leDist->setValidator(new QIntValidator(1, 200000));
	ui->leNX->setValidator(new QDoubleValidator(-200000, 200000, 2));
	ui->leNY->setValidator(new QDoubleValidator(-200000, 200000, 2));
	ui->leNZ->setValidator(new QDoubleValidator(-200000, 200000, 2));
	ui->leProbabilitySeed->setValidator(new QIntValidator(-INT_MAX, INT_MAX));
	ui->leNPts->setValidator(new QIntValidator(1, 1000000));


	// initial values
	m_cdp.distance_to_plane = 1000;
	m_cdp.x_normal = m_cdp.y_normal = 0;
	m_cdp.z_normal = 1;
	m_cdp.npts = 100;
	m_cdp.pixsz = 2;
	m_cdp.pseed = 1234;
	m_cdp.r = m_cdp.g = m_cdp.b = m_cdp.a = 255;
	initialize();

	// connection
	connect(ui->pbColor, SIGNAL(clicked()), this, SLOT(colorClicked()));

}

RConstantDensityPlaneCommandWidget::~RConstantDensityPlaneCommandWidget()
{
	// TODO Auto-generated destructor stub
}


void RConstantDensityPlaneCommandWidget::initialize()
{
	ui->leNPts->setText(QString("%1").arg(m_cdp.npts));
	ui->leProbabilitySeed->setText(QString("%1").arg(m_cdp.pseed));
	ui->sbPixelSize->setValue(m_cdp.pixsz);
	ui->leDist->setText(QString("%1").arg(m_cdp.distance_to_plane));
	ui->leNX->setText(QString("%1").arg(m_cdp.x_normal));
	ui->leNY->setText(QString("%1").arg(m_cdp.y_normal));
	ui->leNZ->setText(QString("%1").arg(m_cdp.z_normal));
	setDotColor(QColor((int)m_cdp.r, (int)m_cdp.g, (int)m_cdp.b, (int)m_cdp.a));
}

void RConstantDensityPlaneCommandWidget::setDotColor(QColor c)
{
	m_dotColor = c;
	ui->lbColor->setText(QString("(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue()));
}

void RConstantDensityPlaneCommandWidget::colorClicked()
{
	QColor c = QColorDialog::getColor(m_dotColor, this);
	if (c.isValid()) setDotColor(c);
}

render::MessageStruct& RConstantDensityPlaneCommandWidget::getCommand(render::MessageStruct& msg)
{
	m_cdp.npts = ui->leNPts->text().toInt();
	m_cdp.pseed = ui->leProbabilitySeed->text().toInt();
	m_cdp.pixsz = ui->sbPixelSize->value();
	m_cdp.distance_to_plane = ui->leDist->text().toFloat();
	m_cdp.x_normal = ui->leNX->text().toFloat();
	m_cdp.y_normal = ui->leNY->text().toFloat();
	m_cdp.z_normal = ui->leNZ->text().toFloat();
	m_cdp.r = (unsigned char)m_dotColor.red();
	m_cdp.g = (unsigned char)m_dotColor.green();
	m_cdp.b = (unsigned char)m_dotColor.blue();
	m_cdp.a = (unsigned char)m_dotColor.alpha();

	return *make_constantdensityplane_msg_s(&msg, &m_cdp);
}
