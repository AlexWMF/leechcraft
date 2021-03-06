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

#include "xtazy.h"
#include <QIcon>
#include <QMessageBox>
#include <QUrl>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iwebfilestorage.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/media/icurrentsongkeeper.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iproxyobject.h>
#include "tracksharedialog.h"
#include "xmlsettingsmanager.h"
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_xtazy");

		Proxy_ = proxy;
		AzothProxy_ = 0;

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothxtazysettings.xml");

		Keeper_ = 0;
	}

	void Plugin::SecondInit ()
	{
		auto keeperObjs = Proxy_->GetPluginsManager ()->GetAllCastableRoots<decltype (Keeper_)> ();
		if (keeperObjs.isEmpty ())
			return;

		Keeper_ = qobject_cast<decltype (Keeper_)> (keeperObjs.at (0));
		connect (keeperObjs.at (0),
				SIGNAL (currentSongChanged (Media::AudioInfo)),
				this,
				SLOT (publish (Media::AudioInfo)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Xtazy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Xtazy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Publishes current tune.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/azoth/plugins/xtazy/resources/images/xtazy.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::HandleShare (LeechCraft::IHookProxy_ptr proxy, QObject *entryObj, const QString& variant, const QUrl& url)
	{
		proxy->CancelDefault ();
		if (!url.isValid ())
			return;

		if (url.scheme () != "file")
		{
			proxy->SetValue ("text", QString::fromUtf8 (url.toEncoded ()));
			return;
		}

		auto sharers = Proxy_->GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ();
		QMap<QString, QObject*> variants;
		Q_FOREACH (auto sharerObj, sharers)
		{
			auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
			Q_FOREACH (const auto& var, sharer->GetServiceVariants ())
				variants [var] = sharerObj;
		}

		if (sharers.isEmpty ())
		{
			QMessageBox::critical (0,
					"LeechCraft",
					tr ("No web share plugins are installed. Try installing NetStoreManager, for example."));
			return;
		}

		const auto& localPath = url.toLocalFile ();

		TrackShareDialog dia (localPath, variants.keys (), entryObj);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& selectedVar = dia.GetVariantName ();
		auto sharerObj = variants [selectedVar];

		auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
		sharer->UploadFile (localPath, selectedVar);

		PendingUploads_ [localPath] << UploadNotifee_t (entryObj, variant);

		connect (sharerObj,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)),
				Qt::UniqueConnection);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
			QObject *chatTab,
			QObject *entryObj,
			int,
			QString variant)
	{
		if (!Keeper_)
			return;

		if (!XmlSettingsManager::Instance ().property ("NPCmdEnabled").toBool ())
			return;

		const auto& song = Keeper_->GetCurrentSong ();

		auto text = proxy->GetValue ("text").toString ();
		if (text == "/np")
		{
			if (!song.Title_.isEmpty () ||
					!song.Artist_.isEmpty () ||
					!song.Album_.isEmpty ())
			{
				text = XmlSettingsManager::Instance ().property ("NPCmdSubst").toString ();
				text.replace ("$artist", song.Artist_);
				text.replace ("$album", song.Album_);
				text.replace ("$title", song.Title_);
			}
			else
				text = XmlSettingsManager::Instance ().property ("NPCmdNoPlaying").toString ();

			if (XmlSettingsManager::Instance ().property ("SendTextImmediately").toBool ())
				proxy->SetValue ("text", text);
			else
			{
				proxy->CancelDefault ();

				QMetaObject::invokeMethod (chatTab,
						"prepareMessageText",
						Qt::QueuedConnection,
						Q_ARG (QString, text));
			}
		}
		else if (text == "/sharesong" && song.Other_.contains ("URL"))
			HandleShare (proxy, entryObj, variant, song.Other_ ["URL"].toUrl ());
	}

	void Plugin::publish (const Media::AudioInfo& info)
	{
		QVariantMap map;
		map ["artist"] = info.Artist_;
		map ["source"] = info.Album_;
		map ["title"] = info.Title_;
		map ["length"] = info.Length_;
		map ["track"] = info.TrackNumber_;

		for (auto accObj : AzothProxy_->GetAllAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
				continue;
			if (acc->GetState ().State_ == SOffline)
				continue;

			if (auto tune = qobject_cast<ISupportTune*> (accObj))
				tune->PublishTune (map);
		}
	}

	void Plugin::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		if (!PendingUploads_.contains (filename))
			return;

		const auto& encoded = url.toEncoded ();

		const auto& notifees = PendingUploads_.take (filename);
		Q_FOREACH (const auto& notifee, notifees)
		{
			auto entry = qobject_cast<ICLEntry*> (notifee.first);
			if (!entry)
				continue;

			const auto msgType = entry->GetEntryType () == ICLEntry::EntryType::MUC ?
					IMessage::Type::MUCMessage :
					IMessage::Type::ChatMessage;
			auto msgObj = entry->CreateMessage (msgType, notifee.second, encoded);
			auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Send ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xtazy, LeechCraft::Azoth::Xtazy::Plugin);

