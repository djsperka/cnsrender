/*
 * RConstantDensityPlaneCommandWidget.h
 *
 *  Created on: Dec 11, 2017
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RCONSTANTDENSITYPLANECOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RCONSTANTDENSITYPLANECOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include "constantdensityplane.h"
#include "ui_ConstantDensityPlaneForm.h"

class RConstantDensityPlaneCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::ConstantDensityPlaneForm *ui;
	render::ConstantDensityPlaneStruct m_cdp;
	QColor m_dotColor;
	void setDotColor(QColor c);
	void initialize();
public:
	RConstantDensityPlaneCommandWidget();
	virtual ~RConstantDensityPlaneCommandWidget();
	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);

protected slots:
		void colorClicked();
};

#endif /* RENDER_CLIENT_COMMANDS_RCONSTANTDENSITYPLANECOMMANDWIDGET_H_ */



#if 0
#include "ff2d.h"
#include "ui_FF2DCommandForm.h"
#include <QColor>

namespace Ui {
    class FF2DCommandForm;
};


class RFF2DCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::FF2DCommandForm *ui;
	render::FF2DStruct m_ff2d;
	QColor m_dotColor;

	void setDotColor(QColor c);
	void initialize();

public:
	RFF2DCommandWidget();
	RFF2DCommandWidget(const render::FF2DStruct& ff2d);
	virtual ~RFF2DCommandWidget() {};

	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);

protected slots:
	void colorClicked();
};


#endif
