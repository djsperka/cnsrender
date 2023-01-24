/*
 * RCameraCommandWidget.cpp
 *
 *  Created on: Jan 4, 2018
 *      Author: dan
 */

#include "RCameraCommandWidget.h"
#include "make_msg.h"

RCameraCommandWidget::RCameraCommandWidget()
: RCommandWidget()
, ui(new Ui::CameraCommandForm())
{
	ui->setupUi(this);

	// validators
	ui->leX->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leY->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leZ->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leLookX->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leLookY->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leLookZ->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leUpX->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leUpY->setValidator(new QDoubleValidator(-999999999, 999999999, 2));
	ui->leUpZ->setValidator(new QDoubleValidator(-999999999, 999999999, 2));


	// initial values
	m_cam.ex = m_cam.ey = m_cam.ez = 0;
	m_cam.dx = m_cam.dy = 0;
	m_cam.dz = -1;
	m_cam.ux = m_cam.uz = 0;
	m_cam.uy = 1;
	m_cam.flag = CAMERA_DEFAULT;

	m_cam.a0 = m_cam.a1 = m_cam.a2 = 0;

	initialize();
}

RCameraCommandWidget::~RCameraCommandWidget()
{
}

void RCameraCommandWidget::initialize()
{
	ui->leX->setText(QString("%1").arg(m_cam.ex));
	ui->leY->setText(QString("%1").arg(m_cam.ey));
	ui->leZ->setText(QString("%1").arg(m_cam.ez));
	ui->leLookX->setText(QString("%1").arg(m_cam.dx));
	ui->leLookY->setText(QString("%1").arg(m_cam.dy));
	ui->leLookZ->setText(QString("%1").arg(m_cam.dz));
	ui->leUpX->setText(QString("%1").arg(m_cam.ux));
	ui->leUpY->setText(QString("%1").arg(m_cam.uy));
	ui->leUpZ->setText(QString("%1").arg(m_cam.uz));
	switch (m_cam.flag)
	{
	case CAMERA_PURSUIT:
		//ui->comboType->
		ui->comboType->setCurrentIndex(1);
		ui->stackedWidget->setCurrentIndex(1);
		break;
	case CAMERA_AZIMUTH:
		//ui->comboType->
		ui->comboType->setCurrentIndex(2);
		ui->stackedWidget->setCurrentIndex(2);
		break;
	default:
		ui->comboType->setCurrentIndex(0);
		ui->stackedWidget->setCurrentIndex(0);
		break;
	}
}



render::MessageStruct& RCameraCommandWidget::getCommand(render::MessageStruct& msg)
{
	m_cam.ex = ui->leX->text().toFloat();
	m_cam.ey = ui->leY->text().toFloat();
	m_cam.ez = ui->leZ->text().toFloat();
	switch(ui->comboType->currentIndex())
	{
	case 1:
		m_cam.flag = CAMERA_PURSUIT;
		m_cam.a0 = ui->lePhi->text().toFloat() * M_PI/180.0;
		m_cam.a1 = ui->leBeta->text().toFloat() * M_PI/180.0;
		m_cam.a2 = ui->leRho->text().toFloat() * M_PI/180.0;
		break;
	case 2:
		m_cam.flag = CAMERA_AZIMUTH;
		m_cam.a0 = ui->leAzimuth->text().toFloat() * M_PI/180.0;
		m_cam.a1 = ui->leElevation->text().toFloat() * M_PI/180.0;
		break;
	default:
		m_cam.flag = CAMERA_DEFAULT;
		m_cam.dx = ui->leLookX->text().toFloat();
		m_cam.dy = ui->leLookY->text().toFloat();
		m_cam.dz = ui->leLookZ->text().toFloat();
		m_cam.ux = ui->leUpX->text().toFloat();
		m_cam.uy = ui->leUpY->text().toFloat();
		m_cam.uz = ui->leUpZ->text().toFloat();
		break;
	}

	return *make_camera_msg_s(&msg, &m_cam);
}
