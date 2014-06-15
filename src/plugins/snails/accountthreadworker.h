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
#include <vmime/net/session.hpp>
#include <vmime/net/message.hpp>
#include <vmime/net/folder.hpp>
#include <vmime/net/store.hpp>
#include <vmime/security/cert/defaultCertificateVerifier.hpp>
#include <interfaces/structures.h>
#include "progresslistener.h"
#include "message.h"
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	class Account;

	class AccountThreadWorker : public QObject
	{
		Q_OBJECT

		Account * const A_;

		const bool IsListening_;

		vmime::shared_ptr<vmime::net::session> Session_;
		vmime::shared_ptr<vmime::net::store> CachedStore_;
		QHash<QStringList, vmime::shared_ptr<vmime::net::folder>> CachedFolders_;

		const vmime::shared_ptr<vmime::security::cert::defaultCertificateVerifier> CertVerifier_;
	public:
		AccountThreadWorker (bool, Account*);
	private:
		vmime::shared_ptr<vmime::net::store> MakeStore ();
		vmime::shared_ptr<vmime::net::transport> MakeTransport ();

		vmime::shared_ptr<vmime::net::folder> GetFolder (const QStringList& folder, int mode);

		Message_ptr FromHeaders (const vmime::shared_ptr<vmime::net::message>&) const;
		void FetchMessagesPOP3 (Account::FetchFlags);
		void FetchMessagesIMAP (Account::FetchFlags, const QList<QStringList>&, vmime::shared_ptr<vmime::net::store>);
		void FetchMessagesInFolder (const QStringList&, vmime::shared_ptr<vmime::net::folder>);
		void SyncIMAPFolders (vmime::shared_ptr<vmime::net::store>);
		QList<Message_ptr> FetchFullMessages (const std::vector<vmime::shared_ptr<vmime::net::message>>&);
		ProgressListener* MkPgListener (const QString&);
	public slots:
		void synchronize (LeechCraft::Snails::Account::FetchFlags, const QList<QStringList>&);
		void markAsUnread (const QList<QByteArray>& ids, const QStringList& folder);
		void fetchWholeMessage (LeechCraft::Snails::Message_ptr);
		void fetchAttachment (LeechCraft::Snails::Message_ptr, const QString&, const QString&);
		void sendMessage (LeechCraft::Snails::Message_ptr);
	signals:
		void error (const QString&);
		void gotEntity (const LeechCraft::Entity&);
		void gotProgressListener (ProgressListener_g_ptr);
		void gotMsgHeaders (QList<Message_ptr>, QStringList);
		void messageBodyFetched (Message_ptr);
		void gotUpdatedMessages (QList<Message_ptr>, QStringList);
		void gotOtherMessages (QList<QByteArray>, QStringList);
		void gotFolders (QList<QStringList>);
	};
}
}
