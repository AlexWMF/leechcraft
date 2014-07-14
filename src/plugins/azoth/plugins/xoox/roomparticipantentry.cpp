/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "roomparticipantentry.h"
#include <QAction>
#include <QtDebug>
#include <QXmppMucManager.h>
#include "glooxaccount.h"
#include "roompublicmessage.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "roomclentry.h"
#include "core.h"
#include "avatarsstorage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomParticipantEntry::RoomParticipantEntry (const QString& nick,
			RoomHandler *rh, GlooxAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	, RoomHandler_ (rh)
	, ID_ (rh->GetRoomJID () + "/" + nick)
	, Affiliation_ (QXmppMucItem::UnspecifiedAffiliation)
	, Role_ (QXmppMucItem::UnspecifiedRole)
	{
	}

	QObject* RoomParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* RoomParticipantEntry::GetParentCLEntry () const
	{
		return RoomHandler_->GetCLEntry ();
	}

	ICLEntry::Features RoomParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType RoomParticipantEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString RoomParticipantEntry::GetEntryName () const
	{
		return Nick_;
	}

	void RoomParticipantEntry::SetEntryName (const QString& nick)
	{
		Nick_ = nick;
		emit nameChanged (Nick_);
	}

	QString RoomParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + '_' + ID_;
	}

	QString RoomParticipantEntry::GetHumanReadableID () const
	{
		return ID_;
	}

	QStringList RoomParticipantEntry::Groups () const
	{
		return QStringList (RoomHandler_->GetCLEntry ()->GetGroupName ());
	}

	void RoomParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QStringList RoomParticipantEntry::Variants () const
	{
		return { {} };
	}

	QObject* RoomParticipantEntry::CreateMessage (IMessage::MessageType type,
			const QString&, const QString& body)
	{
		GlooxMessage *msg = RoomHandler_->CreateMessage (type, Nick_, body);
		AllMessages_ << msg;
		return msg;
	}

	QString RoomParticipantEntry::GetJID () const
	{
		return RoomHandler_->GetRoomJID () + "/" + Nick_;
	}

	QString RoomParticipantEntry::GetRealJID () const
	{
		return RoomHandler_->GetRoom ()->
				participantPresence (GetJID ()).mucItem ().jid ();
	}

	QString RoomParticipantEntry::GetNick () const
	{
		return Nick_;
	}

	void RoomParticipantEntry::StealMessagesFrom (RoomParticipantEntry *other)
	{
		if (other->AllMessages_.isEmpty ())
			return;

		for (auto msg : other->AllMessages_)
			qobject_cast<GlooxMessage*> (msg)->SetVariant (Nick_);

		QList<QObject*> messages;
		std::merge (AllMessages_.begin (), AllMessages_.end (),
				other->AllMessages_.begin (), other->AllMessages_.end (),
				std::back_inserter (messages),
				[] (QObject *msgObj1, QObject *msgObj2)
				{
					const auto msg1 = qobject_cast<IMessage*> (msgObj1);
					const auto msg2 = qobject_cast<IMessage*> (msgObj2);
					return msg1->GetDateTime () < msg2->GetDateTime ();
				});

		other->AllMessages_.clear ();
		AllMessages_ = messages;

		// unread messages are skept intentionally
	}

	void RoomParticipantEntry::SetPhotoHash (const QByteArray& hash)
	{
		VCardPhotoHash_ = hash;
		if (hash.isEmpty ())
			Avatar_ = QImage ();
		else
		{
			Avatar_ = Core::Instance ().GetAvatarsStorage ()->GetAvatar (hash.toHex ());
			if (Avatar_.isNull ())
				VCardPhotoHash_.clear ();
		}
		emit avatarChanged (GetAvatar ());
	}

	QXmppMucItem::Affiliation RoomParticipantEntry::GetAffiliation () const
	{
		return Affiliation_;
	}

	void RoomParticipantEntry::SetAffiliation (QXmppMucItem::Affiliation aff)
	{
		Affiliation_ = aff;
		emit permsChanged ();
	}

	QXmppMucItem::Role RoomParticipantEntry::GetRole () const
	{
		return Role_;
	}

	void RoomParticipantEntry::SetRole (QXmppMucItem::Role role)
	{
		Role_ = role;
		emit permsChanged ();
	}
}
}
}
