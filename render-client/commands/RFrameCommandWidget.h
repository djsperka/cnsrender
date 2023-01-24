/*
 * RFrameCommandWidget.h
 *
 *  Created on: Aug 7, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RFRAMECOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RFRAMECOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "frame.h"
#include "ui_FrameCommandForm.h"

namespace Ui {
    class FrameCommandForm;
};


class RFrameCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::FrameCommandForm *ui;
	render::FrameStruct m_frame;

public:
	RFrameCommandWidget();
	RFrameCommandWidget(const render::FrameStruct& frame);
	virtual ~RFrameCommandWidget() {};

	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);

protected slots:
	void releaseToggled(bool checked);
};

#endif /* RENDER_CLIENT_COMMANDS_RFRAMECOMMANDWIDGET_H_ */
