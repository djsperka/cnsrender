/*
 * RJSCommandWidget.cpp
 *
 *  Created on: Aug 30, 2016
 *      Author: dan
 */

#include "RJSCommandWidget.h"
#include "RCommands.h"
#include <QtDebug>
#include <QFileSystemModel>
#include <QTextStream>

RJSCommandWidget::RJSCommandWidget(const QString& rootDir)
: RCommandWidget()
, ui(new Ui::JSCommandForm())
{
	ui->setupUi(this);

	m_pModel = new QFileSystemModel;
	QStringList filters;
	filters << "*.json" << "*.js";
	m_pModel->setNameFilters(filters);
	m_pModel->setRootPath(rootDir);

	ui->treeView->setModel(m_pModel);
	ui->treeView->setRootIndex(m_pModel->index(rootDir));
	ui->treeView->hideColumn(1);
	ui->treeView->hideColumn(2);
	ui->treeView->hideColumn(3);

	connect(ui->treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(filenameClicked(const QModelIndex &)));
}

render::MessageStruct& RJSCommandWidget::getCommand(render::MessageStruct& msg)
{
	QString sJS = ui->pteJS->toPlainText();
	unsigned char *pfull = (unsigned char *)(&msg);
	unsigned int *plen = (unsigned int *)(pfull+4);
	*plen = sJS.length();
	memcpy(pfull+8, sJS.toStdString().c_str(), sJS.length());
	qDebug() << "Got JS length " << sJS.length();
	pfull[0] = cmdJSON;
	pfull[1] = 255;
	pfull[2] = pfull[3] = 0;
	return msg;
}

void RJSCommandWidget::getJS(QString& js)
{
	js = ui->pteJS->toPlainText();
}

void RJSCommandWidget::filenameClicked(const QModelIndex &index)
{
	QFile file(m_pModel->filePath(index));
    if (!file.open(QIODevice::ReadOnly))
    	qWarning() << "Cannot open file " << m_pModel->filePath(index);
    else
    {
		QTextStream s1(&file);
		QString s = s1.readAll();
		ui->pteJS->setPlainText(s);
//		qDebug() << m_pModel->filePath(index);
//		qDebug() << s;
    }
}

