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

#include "storage.h"
#include <stdexcept>
#include <QFile>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <util/db/dblock.h>
#include <util/sys/paths.h>
#include "xmlsettingsmanager.h"
#include "account.h"
#include "accountdatabase.h"

namespace LeechCraft
{
namespace Snails
{
	namespace
	{
		template<typename T>
		QByteArray Serialize (const T& t)
		{
			QByteArray result;
			QDataStream stream (&result, QIODevice::WriteOnly);
			stream << t;
			return result;
		}
	}

	Storage::Storage (QObject *parent)
	: QObject (parent)
	, Settings_ (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Snails_Storage")
	{
		SDir_ = Util::CreateIfNotExists ("snails/storage");
	}

	namespace
	{
		QList<Message_ptr> MessageSaverProc (QList<Message_ptr> msgs, const QDir dir)
		{
			for (const auto& msg : msgs)
			{
				if (msg->GetFolderID ().isEmpty ())
					continue;

				const QString dirName = msg->GetFolderID ().toHex ().right (3);

				QDir msgDir = dir;
				if (!dir.exists (dirName))
					msgDir.mkdir (dirName);
				if (!msgDir.cd (dirName))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cd into"
							<< msgDir.filePath (dirName);
					continue;
				}

				QFile file (msgDir.filePath (msg->GetFolderID ().toHex ()));
				file.open (QIODevice::WriteOnly);
				file.write (qCompress (msg->Serialize (), 9));
			}

			return msgs;
		}
	}

	void Storage::SaveMessages (Account *acc, const QStringList& folder, const QList<Message_ptr>& msgs)
	{
		auto dir = DirForAccount (acc);
		for (const auto& elem : folder)
		{
			const auto& subdir = elem.toUtf8 ().toHex ();
			if (!dir.exists (subdir))
				dir.mkdir (subdir);

			if (!dir.cd (subdir))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< dir.filePath (subdir);
				throw std::runtime_error ("Unable to cd to the directory");
			}
		}

		for (const auto& msg : msgs)
			PendingSaveMessages_ [acc] [msg->GetFolderID ()] = msg;

		auto watcher = new QFutureWatcher<QList<Message_ptr>> ();
		FutureWatcher2Account_ [watcher] = acc;

		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleMessagesSaved ()));
		auto future = QtConcurrent::run (MessageSaverProc, msgs, dir);
		watcher->setFuture (future);

		for (const auto& msg : msgs)
		{
			if (msg->GetFolderID ().isEmpty ())
				continue;

			AddMessage (msg, acc);
			UpdateCaches (msg);
		}
	}

	MessageSet Storage::LoadMessages (Account *acc)
	{
		MessageSet result;

		const QDir& dir = DirForAccount (acc);
		for (const auto& str : dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
		{
			QDir subdir = dir;
			if (!subdir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< str;
				continue;
			}

			for (const auto& str : subdir.entryList (QDir::NoDotAndDotDot | QDir::Files))
			{
				QFile file (subdir.filePath (str));
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to open"
							<< str
							<< file.errorString ();
					continue;
				}

				const auto& msg = std::make_shared<Message> ();
				try
				{
					msg->Deserialize (qUncompress (file.readAll ()));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error deserializing the message from"
							<< file.fileName ()
							<< e.what ();
					continue;
				}
				result << msg;
				UpdateCaches (msg);
			}
		}

		for (const auto& msg : PendingSaveMessages_ [acc])
		{
			result << msg;
			UpdateCaches (msg);
		}

		return result;
	}

	Message_ptr Storage::LoadMessage (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		if (PendingSaveMessages_ [acc].contains (id))
			return PendingSaveMessages_ [acc] [id];

		auto dir = DirForAccount (acc);
		for (const auto& elem : folder)
		{
			const auto& subdir = elem.toUtf8 ().toHex ();
			if (!dir.cd (subdir))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< dir.filePath (subdir);
				throw std::runtime_error ("Unable to cd to the directory");
			}
		}

		if (!dir.cd (id.toHex ().right (3)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd to"
					<< dir.filePath (id.toHex ().right (3));
			throw std::runtime_error ("Unable to cd to the directory");
		}

		QFile file (dir.filePath (id.toHex ()));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< file.fileName ()
					<< file.errorString ();
			throw std::runtime_error ("Unable to open the message file");
		}

		Message_ptr msg (new Message);
		try
		{
			msg->Deserialize (qUncompress (file.readAll ()));
			UpdateCaches (msg);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error deserializing the message from"
					<< file.fileName ()
					<< e.what ();
			throw;
		}

		return msg;
	}

	QList<QByteArray> Storage::LoadIDs (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetIDs (folder);
	}

	void Storage::RemoveMessage (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		PendingSaveMessages_ [acc].remove (id);

		BaseForAccount (acc)->RemoveMessage (id, folder,
				[this, acc, folder, id] { RemoveMessageFile (acc, folder, id); });
	}

	int Storage::GetNumMessages (Account *acc) const
	{
		int result = 0;

		const QDir& dir = DirForAccount (acc);
		for (const auto& str : dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
		{
			QDir subdir = dir;
			if (!subdir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< str;
				continue;
			}

			result += subdir.entryList (QDir::NoDotAndDotDot | QDir::Files).size ();
		}

		return result;
	}

	int Storage::GetNumMessages (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetMessageCount (folder);
	}

	int Storage::GetNumUnread (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetUnreadMessageCount (folder);
	}

	bool Storage::HasMessagesIn (Account *acc) const
	{
		return GetNumMessages (acc);
	}

	bool Storage::IsMessageRead (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		if (IsMessageRead_.contains (id))
			return IsMessageRead_ [id];

		return LoadMessage (acc, folder, id)->IsRead ();
	}

	void Storage::RemoveMessageFile (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		auto dir = DirForAccount (acc);
		for (const auto& elem : folder)
		{
			const auto& subdir = elem.toUtf8 ().toHex ();
			if (!dir.cd (subdir))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< dir.filePath (subdir);
				throw std::runtime_error ("Unable to cd to the directory");
			}
		}

		if (!dir.cd (id.toHex ().right (3)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd to"
					<< dir.filePath (id.toHex ().right (3));
			throw std::runtime_error ("Unable to cd to the directory");
		}

		QFile file (dir.filePath (id.toHex ()));
		if (!file.exists ())
			return;

		qDebug () << "removing" << file.fileName () << file.size ();
		if (!file.remove ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to remove the file:"
					<< file.errorString ();
			throw std::runtime_error ("Unable to remove the file");
		}
	}

	QDir Storage::DirForAccount (Account *acc) const
	{
		const QByteArray& id = acc->GetID ().toHex ();

		QDir dir = SDir_;
		if (!dir.exists (id))
			dir.mkdir (id);
		if (!dir.cd (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd into"
					<< dir.filePath (id);
			throw std::runtime_error ("Unable to cd to the dir");
		}

		return dir;
	}

	AccountDatabase_ptr Storage::BaseForAccount (Account *acc)
	{
		if (AccountBases_.contains (acc))
			return AccountBases_ [acc];

		const auto& dir = DirForAccount (acc);
		const auto& base = std::make_shared<AccountDatabase> (dir, acc);
		AccountBases_ [acc] = base;
		return base;
	}

	void Storage::AddMessage (Message_ptr msg, Account *acc)
	{
		const auto& base = BaseForAccount (acc);
		base->AddMessage (msg);
	}

	void Storage::UpdateCaches (Message_ptr msg)
	{
		IsMessageRead_ [msg->GetFolderID ()] = msg->IsRead ();
	}

	void Storage::handleMessagesSaved ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QList<Message_ptr>>*> (sender ());
		watcher->deleteLater ();

		auto acc = FutureWatcher2Account_.take (watcher);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "no account for future watcher"
					<< watcher;
			return;
		}

		auto& hash = PendingSaveMessages_ [acc];

		auto messages = watcher->result ();
		for (const auto& msg : messages)
			hash.remove (msg->GetFolderID ());
	}
}
}
