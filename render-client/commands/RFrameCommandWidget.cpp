/*
 * RFrameCommandWidget.cpp
 *
 *  Created on: Aug 7, 2015
 *      Author: dan
 */

#include "RFrameCommandWidget.h"
#include "make_msg.h"
#include <QtDebug>
#include <QIntValidator>

RFrameCommandWidget::RFrameCommandWidget()
: RCommandWidget()
, ui(new Ui::FrameCommandForm())
{
	ui->setupUi(this);
	ui->rbFrame->setChecked(true);
	ui->leReleaseHandle->setValidator(new QIntValidator(1,65535));
	ui->leReleaseHandle->setText("100");
	ui->leReleaseHandle->setEnabled(false);
	connect(ui->rbRelease, SIGNAL(toggled(bool)), this, SLOT(releaseToggled(bool)));
}

void RFrameCommandWidget::releaseToggled(bool checked)
{
	ui->leReleaseHandle->setEnabled(checked);
}

render::MessageStruct& RFrameCommandWidget::getCommand(render::MessageStruct& msg)
{
	if (ui->rbFrame->isChecked())
		return *make_frame_msg_s(&msg, &m_frame);
	else if (ui->rbRender->isChecked())
		return *make_render_msg(&msg);
	else if (ui->rbGrabSingle->isChecked())
		return *make_grab_single_msg(&msg);
	else if (ui->rbGrabOn->isChecked())
		return *make_grab_on_msg(&msg);
	else if (ui->rbGrabOff->isChecked())
		return *make_grab_off_msg(&msg);
	else if (ui->rbQuit->isChecked())
		return *make_quit_msg(&msg);
	else if (ui->rbReset->isChecked())
		return *make_reset_msg(&msg);
	else if (ui->rbRelease->isChecked())
		return *make_release_msg(&msg, ui->leReleaseHandle->text().toInt());
	else
	{
		qDebug() << "Unrecognized command from frame command widget.";
		return msg;
	}
}




