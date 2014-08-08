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

#include "chatmessage.h"
#include "toxcontact.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	ChatMessage::ChatMessage (const QString& body, IMessage::Direction dir, ToxContact *contact)
	: QObject { contact }
	, Contact_ { contact }
	, Dir_ { dir }
	, Body_ { body }
	{
	}

	QObject* ChatMessage::GetQObject ()
	{
		return this;
	}

	void ChatMessage::Send ()
	{
		Contact_->SendMessage (this);
		Store ();
	}

	void ChatMessage::Store ()
	{
		Contact_->HandleMessage (this);
	}

	IMessage::Direction ChatMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type ChatMessage::GetMessageType () const
	{
		return Type::ChatMessage;
	}

	IMessage::SubType ChatMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* ChatMessage::OtherPart () const
	{
		return Contact_;
	}

	QString ChatMessage::GetOtherVariant () const
	{
		return "";
	}

	QString ChatMessage::GetBody () const
	{
		return Body_;
	}

	void ChatMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime ChatMessage::GetDateTime () const
	{
		return TS_;
	}

	void ChatMessage::SetDateTime (const QDateTime& timestamp)
	{
		TS_ = timestamp;
	}

	bool ChatMessage::IsDelivered () const
	{
		return IsDelivered_;
	}

	void ChatMessage::SetDelivered ()
	{
		if (IsDelivered_)
			return;

		IsDelivered_ = true;
		emit messageDelivered ();
	}
}
}
}
