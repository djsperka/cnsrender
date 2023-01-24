/*
 * RCameraCommandWidget.h
 *
 *  Created on: Jan 4, 2018
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RCAMERACOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RCAMERACOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "camerabeacon.h"
#include "ui_CameraCommandForm.h"

class RCameraCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::CameraCommandForm *ui;
	render::CameraStruct m_cam;

	void initialize();

public:
	RCameraCommandWidget();
	virtual ~RCameraCommandWidget();
	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);
};

#endif /* RENDER_CLIENT_COMMANDS_RCAMERACOMMANDWIDGET_H_ */
