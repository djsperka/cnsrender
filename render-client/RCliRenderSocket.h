/*
 * RCliRenderSocket.h
 *
 *  Created on: Aug 14, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_RCLIRENDERSOCKET_H_
#define RENDER_CLIENT_RCLIRENDERSOCKET_H_

#include <QUdpSocket>
#include "RCliMessageHolder.h"


class RCliRenderSocket
{
	QUdpSocket m_socket;
public:
	RCliRenderSocket(QString hostAddress, int port);
	~RCliRenderSocket() {};
	bool send(RCliMessageHolder& h);
};



#endif /* RENDER_CLIENT_RCLIRENDERSOCKET_H_ */
