/*
 * RCliMainWindowDialog.h
 *
 *  Created on: Aug 3, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_RCLIMAINWINDOWDIALOG_H_
#define RENDER_CLIENT_RCLIMAINWINDOWDIALOG_H_

#include <QMainWindow>
#include <QList>
#include "ui_RCliMainWindowForm.h"
#include "RCommandWidget.h"
#include "RCliRenderSocket.h"

namespace Ui {
    class RCliMainWindowForm;
};

class RCliMainWindowDialog : public QMainWindow
{
	Q_OBJECT

	Ui::RCliMainWindowForm *ui;
	unsigned int m_pingCount;
	RCommandWidget* m_currentEditor;
	QList<RCommandWidget*> m_listEditorWidgets;
	RCliRenderSocket *m_socket;
	bool m_bInitOK;

	bool initRenderConnection();
	void ping();
public:
	RCliMainWindowDialog(const QStringList& list = QStringList(), const QList<RCommandWidget*>& wList = QList<RCommandWidget*>(), const QString& sIPPort = "127.0.0.1:1999");
	virtual ~RCliMainWindowDialog();

	QString getIP();
	int getPort();

protected slots:
	void rbJSClicked(bool checked);
	void rbBinaryClicked(bool checked);
	void ipAddressChanged(const QString& txt);
	void portChanged(const QString& txt);
	void currentCommandChanged(const QModelIndex & current, const QModelIndex & previous);
	void cancelClicked();
	void queueClicked();
	void sendNowClicked();
};


#endif /* RENDER_CLIENT_RCLIMAINWINDOWDIALOG_H_ */
