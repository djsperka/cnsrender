/*
 * RPerspectiveCommandWidget.cpp
 *
 *  Created on: Dec 19, 2017
 *      Author: dan
 */

#include "RPerspectiveCommandWidget.h"
#include <QDoubleValidator>
#include "make_msg.h"

RPerspectiveCommandWidget::RPerspectiveCommandWidget()
: RCommandWidget()
, ui(new Ui::PerspectiveCommandForm())
{
	ui->setupUi(this);
	m_perspective.nearPlane = 1;
	m_perspective.farPlane = 1000;
	m_perspective.fovy = 60 * M_PI/180.0;

	initialize();
}

RPerspectiveCommandWidget::~RPerspectiveCommandWidget()
{
	// TODO Auto-generated destructor stub
}


void RPerspectiveCommandWidget::initialize()
{
	ui->leNear->setValidator(new QDoubleValidator(1.0, 1000000.0, 1));
	ui->leFar->setValidator(new QDoubleValidator(1.0, 1000000.0, 1));
	ui->leFovy->setValidator(new QDoubleValidator(1.0, 180.0, 0));

	ui->leNear->setText(QString("%1").arg(m_perspective.nearPlane));
	ui->leFar->setText(QString("%1").arg(m_perspective.farPlane));
	ui->leFovy->setText(QString("%1").arg(m_perspective.fovy * 180.0/M_PI));
}

render::MessageStruct& RPerspectiveCommandWidget::getCommand(render::MessageStruct& msg)
{
	m_perspective.nearPlane = ui->leNear->text().toDouble();
	m_perspective.farPlane = ui->leFar->text().toDouble();
	m_perspective.fovy = ui->leFovy->text().toDouble() * M_PI/180.0;
	return *make_perspective_msg_s(&msg, &m_perspective);
}

