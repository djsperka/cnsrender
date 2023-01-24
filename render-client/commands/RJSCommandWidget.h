/*
 * RJSCommand.h
 *
 *  Created on: Aug 30, 2016
 *      Author: dan
 */

#ifndef RENDER_CLIENT_COMMANDS_RJSCOMMANDWIDGET_H_
#define RENDER_CLIENT_COMMANDS_RJSCOMMANDWIDGET_H_

#include "RCommandWidget.h"
#include <QFileSystemModel>
#include <QModelIndex>
#include "ui_JSCommandForm.h"
#include <QDir>


namespace Ui {
    class JSCommandForm;
};


class RJSCommandWidget: public RCommandWidget
{
	Q_OBJECT
	Ui::JSCommandForm *ui;
	QFileSystemModel *m_pModel;

protected slots:
	void filenameClicked(const QModelIndex &index);

public:
	RJSCommandWidget(const QString& rootDir);
	virtual ~RJSCommandWidget() {};

	virtual	render::MessageStruct& getCommand(render::MessageStruct& msg);
	virtual void getJS(QString& js);
};




#endif /* RENDER_CLIENT_COMMANDS_RJSCOMMANDWIDGET_H_ */
