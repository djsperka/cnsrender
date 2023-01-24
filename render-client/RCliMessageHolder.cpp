/*
 * RCliMessageHolder.cpp
 *
 *  Created on: Aug 14, 2015
 *      Author: dan
 */

#include "RCliMessageHolder.h"

RCliMessageHolder::RCliMessageHolder()
: m_bSentAlready(false)
, m_bTimeout(false)
{
	m_msg.header.commandID = 0;
	m_msg.header.id2 = 0;
	m_msg.header.id3 = 0;
	m_msg.header.length = 0;
	m_resp.c[0] = 0;
	m_resp.c[1] = 0;
	m_resp.c[2] = 0;
	m_resp.c[3] = 0;
};

RCliMessageHolder::RCliMessageHolder(const RCliMessageHolder& h)
: m_bSentAlready(h.isSent())
, m_bTimeout(h.isTimeout())
, m_msg(h.msg())
, m_resp(h.resp())
{};

RCliMessageHolder::RCliMessageHolder(const render::MessageStruct& msg)
: m_bSentAlready(false)
, m_bTimeout(false)
, m_msg(msg)
{
	m_resp.c[0] = 0;
	m_resp.c[1] = 0;
	m_resp.c[2] = 0;
	m_resp.c[3] = 0;
}

RCliMessageHolder& RCliMessageHolder::operator=(const RCliMessageHolder& rhs)
{
	if (this != &rhs)
	{
		setSent(rhs.isSent());
		setTimeout(rhs.isTimeout());
		m_msg = rhs.msg();
		m_resp = rhs.resp();
	}
	return *this;
}
