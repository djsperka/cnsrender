/*
 * RCliMainWindow.cpp
 *
 *  Created on: Aug 3, 2015
 *      Author: dan
 */

#include "RCliMainWindowDialog.h"
#include "RMessageStruct.h"
#include "transport.h"
#include "ui_RCliMainWindowForm.h"
#include <QIntValidator>
#include <QThread>
#include <QtDebug>
#include <QListView>
#include <QStringListModel>
#include <QItemSelectionModel>
#include <QUrl>
#include "transport.h"
#include "actions.h"
#include "RJSAsyncTcpClient.h"
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <string>

RCliMainWindowDialog::RCliMainWindowDialog(const QStringList& list, const QList<RCommandWidget*>& wList, const QString& sIPPort)
: QMainWindow()
, ui(new Ui::RCliMainWindowForm())
, m_pingCount(0)
, m_currentEditor(0)
, m_listEditorWidgets(wList)
, m_socket(NULL)
, m_bInitOK(false)
{
    ui->setupUi(this);
    ui->lineeditPort->setValidator(new QIntValidator(1024, 65535));

    // intialize command list
    QStringListModel *model = new QStringListModel();
    model->setStringList(list);
    ui->lvCommands->setModel(model);

    QWidget *w;
    foreach(w, m_listEditorWidgets)
    	ui->stackEditors->addWidget(w);

    // defaults
    QUrl url;
    url.setScheme("http");
    url.setAuthority(sIPPort);
    qDebug() << sIPPort << url.isValid() << url.isRelative() << url.host() << url.port();
    ui->lineeditIPAddress->setText(url.host());
    ui->lineeditPort->setText(QString("%1").arg(url.port()));
    ui->rbJS->setChecked(true);

    connect(ui->rbJS, SIGNAL(clicked(bool)), this, SLOT(rbJSClicked(bool)));
    connect(ui->rbBinary, SIGNAL(clicked(bool)), this, SLOT(rbBinaryClicked(bool)));
    connect(ui->lineeditIPAddress, SIGNAL(textChanged(const QString&)), this, SLOT(ipAddressChanged(const QString&)));
    connect(ui->lineeditPort,  SIGNAL(textChanged(const QString&)), this, SLOT(portChanged(const QString&)));
    connect(ui->lvCommands->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(currentCommandChanged(const QModelIndex&, const QModelIndex&)));
    connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    connect(ui->pbQueue, SIGNAL(clicked()), this, SLOT(queueClicked()));
    connect(ui->pbSendNow, SIGNAL(clicked()), this, SLOT(sendNowClicked()));

    // set current selection to trigger an initial editor...
    ui->lvCommands->selectionModel()->setCurrentIndex(model->index(0), QItemSelectionModel::Select);

}

RCliMainWindowDialog::~RCliMainWindowDialog()
{
    delete ui;
}

void RCliMainWindowDialog::rbJSClicked(bool)
{
	m_bInitOK = false;
}

void RCliMainWindowDialog::rbBinaryClicked(bool)
{
	m_bInitOK = false;
}

bool RCliMainWindowDialog::initRenderConnection()
{
	int status = 0;
	m_pingCount = 0;
	if (!m_bInitOK)
	{
		close_tcpip();
		status = init_tcpip(NULL, getIP().toLocal8Bit().data(), getPort(), 1);
		statusBar()->showMessage(QString("CONNECT %1:%2 status: %3").arg(getIP()).arg(getPort()).arg(status));
		if (!status)
		{
			ping();
			m_bInitOK = true;
		}
	}
	return m_bInitOK;
}

void RCliMainWindowDialog::currentCommandChanged(const QModelIndex & current, const QModelIndex & previous)
{
	ui->stackEditors->setCurrentWidget(m_listEditorWidgets.at(current.row()));
}

void RCliMainWindowDialog::ping()
{
	int status=0;
	struct timespec ts100 = { 0, (100) * 1000 * 1000 };	// 100 ms
	struct timespec ts500 = { 0, (500) * 1000 * 1000 };	// 500 ms
	PingStruct ping;
	double rate=0;
	int width=0, height=0;

	m_pingCount++;
	// send ping command, wait briefly for response
	render_send_ping();
	nanosleep(&ts100, NULL);
	status = render_check_for_ping();
	if (!status)
	{
		nanosleep(&ts500, NULL);
		status = render_check_for_ping();
	}
	if (status)
	{
		render_get_parameters(&width, &height, &rate);
		statusBar()->showMessage(QString("PING (%1) %2:%3 status: %4 res: %5x%6 rate: %7").arg(m_pingCount).arg(getIP()).arg(getPort()).arg(status).arg(width).arg(height).arg(rate));
	}
	else
	{
		statusBar()->showMessage(QString("No response from server."));
	}
}

void RCliMainWindowDialog::ipAddressChanged(const QString& txt)
{
	m_bInitOK = false;
}

void RCliMainWindowDialog::portChanged(const QString&)
{
	m_bInitOK = false;
}

QString RCliMainWindowDialog::getIP()
{
	return ui->lineeditIPAddress->text();
}

int RCliMainWindowDialog::getPort()
{
	return ui->lineeditPort->text().toInt();
}

void RCliMainWindowDialog::cancelClicked()
{
	qDebug() << "cancelClicked()";
}

void RCliMainWindowDialog::queueClicked()
{
	qDebug() << "queueClicked()";
}

void RCliMainWindowDialog::sendNowClicked()
{
	if (ui->rbBinary->isChecked())
	{
		render::MessageStruct msg;
		initRenderConnection();
		m_listEditorWidgets.at(ui->lvCommands->selectionModel()->currentIndex().row())->getCommand(msg);
		msg_send((char *)&msg);
	}
	else
	{
		QString js;
		m_listEditorWidgets.at(ui->lvCommands->selectionModel()->currentIndex().row())->getJS(js);
		if (js.size() > 0)
		{
			boost::asio::io_service ioservice;
			QString s = QString("%1:%2").arg(getIP()).arg(getPort());
			render::RJSAsyncTcpClient tcpClient(ioservice, s.toStdString(), js.toStdString());
			ioservice.run();
		}
		else
		{
			qDebug() << "No JS for this command";
		}
	}
}
