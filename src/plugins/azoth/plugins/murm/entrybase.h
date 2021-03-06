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

#pragma once

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iupdatablechatentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkMessage;
	struct MessageInfo;

	class EntryBase : public QObject
					, public ICLEntry
					, public IUpdatableChatEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
				LeechCraft::Azoth::IUpdatableChatEntry)
	protected:
		VkAccount * const Account_;
		QList<VkMessage*> Messages_;
	public:
		EntryBase (VkAccount*);

		virtual void Send (VkMessage*) = 0;
		void Store (VkMessage*);

		QObject* GetQObject ();
		QObject* GetParentAccount () const;

		QObject* CreateMessage (IMessage::Type type, const QString& variant, const QString& body);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime& before);
	protected:
		void HandleAttaches (VkMessage*, const MessageInfo&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void performJS (const QString&);
	};
}
}
}
