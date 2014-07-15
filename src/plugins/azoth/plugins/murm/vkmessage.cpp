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

#include "vkmessage.h"
#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	VkMessage::VkMessage (bool isOurs, Direction dir, Type type, EntryBase *parentEntry, EntryBase *other)
	: QObject (parentEntry)
	, OtherPart_ (other)
	, ParentCLEntry_ (parentEntry)
	, Type_ (type)
	, Dir_ (dir)
	, IsOurs_ (isOurs)
	{
	}

	QObject* VkMessage::GetQObject ()
	{
		return this;
	}

	void VkMessage::Send ()
	{
		ParentCLEntry_->Send (this);
		Store ();
	}

	void VkMessage::Store ()
	{
		ParentCLEntry_->Store (this);
	}

	qulonglong VkMessage::GetID () const
	{
		return ID_;
	}

	void VkMessage::SetID (qulonglong id)
	{
		ID_ = id;
		emit messageDelivered ();
	}

	bool VkMessage::IsRead () const
	{
		return IsRead_;
	}

	void VkMessage::SetRead ()
	{
		IsRead_ = true;
	}

	QString VkMessage::GetRawBody () const
	{
		return Body_;
	}

	IMessage::Direction VkMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type VkMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType VkMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* VkMessage::OtherPart () const
	{
		return OtherPart_ ? OtherPart_ : ParentCLEntry_;
	}

	QObject* VkMessage::ParentCLEntry() const
	{
		return ParentCLEntry_;
	}

	QString VkMessage::GetOtherVariant () const
	{
		return "";
	}

	QString VkMessage::GetBody () const
	{
		auto result = Body_;
		if (IsOurs_)
			result.replace ('<', "&lt;");
		return result;
	}

	void VkMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	IMessage::EscapePolicy VkMessage::GetEscapePolicy () const
	{
		return EscapePolicy::NoEscape;
	}

	QDateTime VkMessage::GetDateTime () const
	{
		return TS_;
	}

	void VkMessage::SetDateTime (const QDateTime& timestamp)
	{
		TS_ = timestamp;
	}

	bool VkMessage::IsDelivered () const
	{
		return ID_ != static_cast<qulonglong> (-1);
	}

	QString VkMessage::GetRichBody () const
	{
		return RichBody_;
	}

	void VkMessage::SetRichBody (const QString& body)
	{
		RichBody_ = body;
	}
}
}
}
