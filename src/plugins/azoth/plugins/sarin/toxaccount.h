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

#include <memory>
#include <QObject>
#include <interfaces/azoth/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	class ToxProtocol;
	class ToxThread;
	class ToxContact;
	class ChatMessage;
	class MessagesManager;

	class ToxAccount : public QObject
					 , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount)

		ToxProtocol * const Proto_;
		const QByteArray UID_;

		QString Name_;
		QString Nick_;

		QByteArray ToxState_;

		QAction * const ActionGetToxId_;

		std::shared_ptr<ToxThread> Thread_;

		MessagesManager * const MsgsMgr_;

		QHash<QByteArray, ToxContact*> Contacts_;

		ToxAccount (const QByteArray&, const QString& name, ToxProtocol*);
	public:
		ToxAccount (const QString& name, ToxProtocol*);

		QByteArray Serialize ();
		static ToxAccount* Deserialize (const QByteArray&, ToxProtocol*);

		void SetNickname (const QString&);

		QObject* GetQObject () override;
		QObject* GetParentProtocol () const override;
		AccountFeatures GetAccountFeatures () const override;

		QList<QObject*> GetCLEntries () override;

		QString GetAccountName () const override;
		QString GetOurNick () const override;
		void RenameAccount (const QString& name) override;
		QByteArray GetAccountID () const override;

		QList<QAction*> GetActions () const override;
		void OpenConfigurationDialog () override;
		EntryStatus GetState () const override;
		void ChangeState (const EntryStatus&) override;

		void Authorize (QObject*) override;
		void DenyAuth (QObject*) override;
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&) override;
		void RemoveEntry (QObject*) override;

		QObject* GetTransferManager () const override;

		void SendMessage (const QByteArray& pkey, ChatMessage *msg);
	private:
		void InitThread (const EntryStatus&);
		void InitEntry (const QByteArray&);
	private slots:
		void handleToxIdRequested ();
		void handleToxStateChanged (const QByteArray&);

		void handleGotFriend (qint32);
		void handleGotFriendRequest (const QByteArray&, const QString&);
		void handleFriendNameChanged (const QByteArray&, const QString&);
		void handleFriendStatusChanged (const QByteArray&, const EntryStatus&);

		void handleInMessage (const QByteArray&, const QString&);
	signals:
		void accountRenamed (const QString&) override;
		void authorizationRequested (QObject*, const QString&) override;
		void gotCLItems (const QList<QObject*>&) override;
		void itemCancelledSubscription (QObject*, const QString&) override;
		void itemGrantedSubscription (QObject*, const QString&) override;
		void itemSubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (const QString& entryID, const QString&) override;
		void itemUnsubscribed (QObject*, const QString&) override;
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&) override;
		void removedCLItems (const QList< QObject* >&) override;
		void statusChanged (const EntryStatus&) override;

		void accountChanged (ToxAccount*);

		void threadChanged (const std::shared_ptr<ToxThread>&);
	};
}
}
}
