/*
 * RCommandWidget.h
 *
 *  Created on: Aug 6, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RCOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RCOMMANDWIDGET_H_

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include "RMessageStruct.h"

class RCommandWidget: public QWidget
{
	Q_OBJECT
public:
	RCommandWidget();
	virtual ~RCommandWidget() {};

	static RCommandWidget* create(unsigned char cmd);
	static RCommandWidget* create(const render::MessageStruct& msg);

	// each subclass must implement this. The return value should be the same as the passed 'msg',
	// as a convenience to callers.
	virtual render::MessageStruct& getCommand(render::MessageStruct& msg) = 0;
	virtual void getJS(QString& js) { Q_UNUSED(js); };

};



#endif /* RENDER_CLIENT_COMMANDS_RCOMMANDWIDGET_H_ */
