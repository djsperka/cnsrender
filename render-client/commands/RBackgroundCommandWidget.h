/*
 * RBackgroundCommandWidget.h
 *
 *  Created on: Aug 6, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RBACKGROUNDCOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RBACKGROUNDCOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "background.h"
#include <QColorDialog>

class RBackgroundCommandWidget: public RCommandWidget
{
	Q_OBJECT
	render::BackgroundStruct m_background;
	QColorDialog* m_colorDialog;

	void components();
	void connections();

public:
	RBackgroundCommandWidget();
	RBackgroundCommandWidget(const render::BackgroundStruct& background);
	virtual ~RBackgroundCommandWidget() {};

	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);
};



#endif /* RENDER_CLIENT_COMMANDS_RBACKGROUNDCOMMANDWIDGET_H_ */
