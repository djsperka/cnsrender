/*
 * RPerspectiveCommandWidget.h
 *
 *  Created on: Dec 19, 2017
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RPERSPECTIVECOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RPERSPECTIVECOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "perspective.h"
#include "ui_PerspectiveCommandForm.h"

namespace Ui {
    class PerspectiveCommandForm;
};

class RPerspectiveCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::PerspectiveCommandForm *ui;
	render::PerspectiveStruct m_perspective;

	void initialize();

public:
	RPerspectiveCommandWidget();
	RPerspectiveCommandWidget(const render::PerspectiveStruct& perspective);
	virtual ~RPerspectiveCommandWidget();
	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);
};

#endif /* RENDER_CLIENT_COMMANDS_RPERSPECTIVECOMMANDWIDGET_H_ */
