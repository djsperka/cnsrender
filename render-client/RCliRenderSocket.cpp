/*
 * RCliRenderSocket.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: dan
 */

#include "RCliRenderSocket.h"
#include <QHostAddress>
#include <QtDebug>

RCliRenderSocket::RCliRenderSocket(QString hostAddress, int port)
{
	m_socket.connectToHost(hostAddress, port);
}

bool RCliRenderSocket::send(RCliMessageHolder& h)
{
	qint64 length = h.msg().header.length + 2;
	qint64 n = m_socket.write((char *)&h.msg(), length);
	return n==length;
}
