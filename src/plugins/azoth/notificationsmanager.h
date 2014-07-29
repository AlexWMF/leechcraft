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
#include <QVariantMap>
#include <interfaces/core/ihookproxy.h>

class IEntityManager;

namespace LeechCraft
{
namespace Azoth
{
	class IMessage;
	class ICLEntry;
	class IAccount;
	struct EntryStatus;

	class NotificationsManager : public QObject
	{
		Q_OBJECT

		IEntityManager * const EntityMgr_;
		QHash<ICLEntry*, int> UnreadCounts_;

		QHash<IAccount*, QDateTime> LastAccountStatusChange_;
	public:
		NotificationsManager (IEntityManager*, QObject* = nullptr);

		void AddAccount (QObject*);
		void RemoveAccount (QObject*);

		void AddCLEntry (QObject*);
		void RemoveCLEntry (QObject*);

		void HandleMessage (IMessage*);
	private:
		void NotifyWithReason (QObject*, const QString&,
				const char*, const QString&,
				const QString&, const QString&);
	public slots:
		void handleClearUnreadMsgCount (QObject*);
	private slots:
		void handleItemSubscribed (QObject*, const QString&);
		void handleItemUnsubscribed (QObject*, const QString&);
		void handleItemUnsubscribed (const QString&, const QString&);
		void handleItemCancelledSubscription (QObject*, const QString&);
		void handleItemGrantedSubscription (QObject*, const QString&);

		void handleAccountStatusChanged (const EntryStatus&);
		void handleStatusChanged (const EntryStatus&, const QString&);

		void handleAttentionDrawn (const QString&, const QString&);
		void handleAuthorizationRequested (QObject*, const QString&);

		void handleMUCInvitation (const QVariantMap&, const QString&, const QString&);
	signals:
		void hookGotAuthRequest (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);
	};
}
}
