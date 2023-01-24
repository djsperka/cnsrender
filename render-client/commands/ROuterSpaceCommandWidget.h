/*
 * ROuterSpaceCommandWidget.h
 *
 *  Created on: Jun 27, 2017
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_ROUTERSPACECOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_ROUTERSPACECOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "outerspace.h"
#include "ui_OuterSpaceCommandForm.h"
#include <QColor>

namespace Ui {
    class OuterSpaceCommandForm;
};



class ROuterSpaceCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::OuterSpaceCommandForm *ui;
	render::OuterSpaceStruct m_outerspace;
	QColor m_dotColor;

	void initialize();
	void setDotColor(QColor c);

public:
	ROuterSpaceCommandWidget();
	virtual ~ROuterSpaceCommandWidget() {};
	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);

protected slots:
		void colorClicked();
};

#endif /* RENDER_CLIENT_COMMANDS_ROUTERSPACECOMMANDWIDGET_H_ */
