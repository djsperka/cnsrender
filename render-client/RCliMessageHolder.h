/*
 * RCliMessageHolder.h
 *
 *  Created on: Aug 14, 2015
 *      Author: dan
 */

#ifndef RENDER_CLIENT_RCLIMESSAGEHOLDER_H_
#define RENDER_CLIENT_RCLIMESSAGEHOLDER_H_

#include "RMessageStruct.h"

class RCliMessageHolder
{
	bool m_bSentAlready;
	bool m_bTimeout;
	render::MessageStruct m_msg;
	render::RFullMessage m_resp;

public:
	RCliMessageHolder();
	RCliMessageHolder(const render::MessageStruct& msg);
	RCliMessageHolder(const RCliMessageHolder& h);
	virtual ~RCliMessageHolder() {};
	RCliMessageHolder& operator=(const RCliMessageHolder& rhs);

	const render::MessageStruct& msg() { return m_msg; };
	const render::MessageStruct& msg() const { return m_msg; };
	render::RFullMessage& resp() { return m_resp; };
	const render::RFullMessage resp() const { return m_resp; };
	void setSent(bool b) { m_bSentAlready = b; };
	void setTimeout(bool b) { m_bTimeout = b; };
	bool isSent() const { return m_bSentAlready; };
	bool isTimeout() const { return m_bTimeout; };

};

#endif /* RENDER_CLIENT_RCLIMESSAGEHOLDER_H_ */
