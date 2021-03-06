/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "channelparticipantentry.h"
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "channelsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelParticipantEntry::ChannelParticipantEntry (const QString& nick,
			ChannelHandler *ich, IrcAccount *acc)
	: IrcParticipantEntry (nick, acc)
	, ICH_ (ich)
	{
		QMenu *infoMenu = new QMenu (tr ("Information"));
		infoMenu->addAction ("/WHOIS " + Nick_,
				this,
				SLOT (handleWhoIs ()));
		infoMenu->addAction ("/WHOWAS " + Nick_,
				this,
				SLOT (handleWhoWas ()));
		infoMenu->addAction ("/WHO " + Nick_,
				this,
				SLOT (handleWho ()));

		QMenu *ctcpMenu = new QMenu (tr ("CTCP"));
		QAction *ping = ctcpMenu->addAction ("PING");
		ping->setProperty ("ctcp_type", "ping");
		QAction *finger = ctcpMenu->addAction ("FINGER");
		finger->setProperty ("ctcp_type", "finger");
		QAction *version = ctcpMenu->addAction ("VERSION");
		version->setProperty ("ctcp_type", "version");
		QAction *userinfo = ctcpMenu->addAction ("USERINFO");
		userinfo->setProperty ("ctcp_type", "userinfo");
		QAction *clientinfo = ctcpMenu->addAction ("CLIENTINFO");
		clientinfo->setProperty ("ctcp_type", "clientinfo");
		QAction *source = ctcpMenu->addAction ("SOURCE");
		source->setProperty ("ctcp_type", "source");
		QAction *time = ctcpMenu->addAction ("TIME");
		time->setProperty ("ctcp_type", "time");

		connect (ctcpMenu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleCTCPAction (QAction*)));

		Actions_.append (infoMenu->menuAction ());
		Actions_.append (ctcpMenu->menuAction ());

		ServerID_ = ICH_->GetParentID ();
	}

	QObject* ChannelParticipantEntry::GetParentCLEntry () const
	{
		return ICH_->GetCLEntry ();
	}

	QString ChannelParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ServerID_ + "_" + Nick_;
	}

	QString ChannelParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ICH_->GetChannelID ();
	}

	QStringList ChannelParticipantEntry::Groups () const
	{
		return QStringList (ICH_->GetChannelID ());
	}

	void ChannelParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QObject* ChannelParticipantEntry::CreateMessage (IMessage::Type,
			const QString&, const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::Type::ChatMessage,
				IMessage::Direction::Out,
				ServerID_,
				Nick_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		return message;
	}

	QList<ChannelRole> ChannelParticipantEntry::Roles () const
	{
		return Roles_;
	}

	ChannelRole ChannelParticipantEntry::HighestRole ()
	{
		if (Roles_.isEmpty ())
			return ChannelRole::Participant;

		return Roles_.last ();
	}

	void ChannelParticipantEntry::SetRole (const ChannelRole& role)
	{
		if (!Roles_.contains (role))
		{
			Roles_ << role;
			qSort (Roles_.begin (), Roles_.end ());
			emit permsChanged ();
		}
	}

	void ChannelParticipantEntry::SetRoles (const QList<ChannelRole>& roles)
	{
		Roles_ = roles;
		qSort (Roles_.begin (), Roles_.end ());
		emit permsChanged ();
	}

	void ChannelParticipantEntry::RemoveRole (const ChannelRole& role)
	{
		if (Roles_.removeAll (role))
		{
			qSort (Roles_.begin (), Roles_.end ());
			emit permsChanged ();
		}
	}

	void ChannelParticipantEntry::handleWhoIs ()
	{
		ICH_->handleWhoIs (Nick_);
	}

	void ChannelParticipantEntry::handleWhoWas ()
	{
		ICH_->handleWhoWas (Nick_);
	}

	void ChannelParticipantEntry::handleWho ()
	{
		ICH_->handleWho (Nick_);
	}

	void ChannelParticipantEntry::handleCTCPAction (QAction *action)
	{
		ICH_->handleCTCPRequest (QStringList () << Nick_
				<< action->property ("ctcp_type").toString ());
	}

}
}
}
