/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "poshuku.h"
#include <stdexcept>
#include <QMessageBox>
#include <qwebsettings.h>
#include <QHeaderView>
#include <QToolBar>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QInputDialog>
#include <QMenu>
#include <QMainWindow>
#include <QtDebug>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include <util/tagscompletionmodel.h>
#include <util/backendselector.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "browserwidget.h"
#include "cookieseditdialog.h"

namespace LeechCraft
{
namespace Poshuku
{
	using LeechCraft::Util::TagsCompletionModel;

	Poshuku::~Poshuku ()
	{
		Core::Instance ().setParent (0);
	}

	void Poshuku::Init (ICoreProxy_ptr coreProxy)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku"));

		BrowserWidget::SetParentMultiTabs (this);
		Core::Instance ().setParent (this);
		Core::Instance ().SetProxy (coreProxy);

		try
		{
			QWebSettings::setIconDatabasePath (
					Util::CreateIfNotExists ("poshuku/favicons")
							.absolutePath ()
					);
		}
		catch (const std::runtime_error& e)
		{
			QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
					e.what ());
		}

		try
		{
			QWebSettings::setOfflineStoragePath (
					Util::CreateIfNotExists ("poshuku/offlinestorage")
							.absolutePath ()
					);
		}
		catch (const std::runtime_error& e)
		{
			QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
					e.what ());
		}

		try
		{
			QWebSettings::setOfflineWebApplicationCachePath (
					Util::CreateIfNotExists ("poshuku/offlinewebappcache")
						.absolutePath ()
					);
		}
		catch (const std::runtime_error& e)
		{
			QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
					e.what ());
		}

		XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukusettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
				new Util::BackendSelector (XmlSettingsManager::Instance ()));

		InitConnections ();

		ImportXbel_ = new QAction (tr ("Import XBEL..."),
				this);
		ImportXbel_->setProperty ("ActionIcon", "poshuku_importxbel");

		ExportXbel_ = new QAction (tr ("Export XBEL..."),
				this);
		ExportXbel_->setProperty ("ActionIcon", "poshuku_exportxbel");

		CheckFavorites_ = new QAction (tr ("Check favorites..."),
				this);
		CheckFavorites_->setProperty ("ActionIcon", "poshuku_checkfavorites");

		ReloadAll_ = new QAction (tr ("Reload all pages"),
				this);
		ReloadAll_->setProperty ("ActionIcon", "poshuku_reloadall");


		try
		{
			Core::Instance ().Init ();
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
					tr ("Poshuku failed to initialize properly. "
						"Check logs and talk with the developers. "
						"Or, at least, check the storage backend "
						"settings and restart LeechCraft."));
			throw;
		}

		RegisterSettings ();

		connect (Core::Instance ().GetFavoritesModel (),
				SIGNAL (error (const QString&)),
				this,
				SLOT (handleError (const QString&)));

		connect (ImportXbel_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (importXbel ()));
		connect (ExportXbel_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (exportXbel ()));
		connect (CheckFavorites_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCheckFavorites ()));
		connect (ReloadAll_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReloadAll ()));

		IShortcutProxy *proxy = coreProxy->GetShortcutProxy ();
		ImportXbel_->setShortcuts (proxy->GetShortcuts (this, "EAImportXbel_"));
		ExportXbel_->setShortcuts (proxy->GetShortcuts (this, "EAExportXbel_"));
		CheckFavorites_->setShortcuts (proxy->GetShortcuts (this, "EACheckFavorites_"));
		ReloadAll_->setShortcuts (proxy->GetShortcuts (this, "EAReloadAll_"));

		ToolMenu_ = new QMenu ("Poshuku",
				Core::Instance ().GetProxy ()->GetMainWindow ());
		ToolMenu_->setIcon (GetIcon ());
		ToolMenu_->addAction (ImportXbel_);
		ToolMenu_->addAction (ExportXbel_);
	}

	void Poshuku::SecondInit ()
	{
		QTimer::singleShot (1000,
				this,
				SLOT (createTabFirstTime ()));
	}

	void Poshuku::Release ()
	{
		Core::Instance ().setParent (0);
		Core::Instance ().Release ();
		XmlSettingsDialog_.reset ();
	}

	QByteArray Poshuku::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku";
	}

	QString Poshuku::GetName () const
	{
		return tr ("Poshuku Browser");
	}

	QString Poshuku::GetInfo () const
	{
		return tr ("Simple yet functional web browser");
	}

	QStringList Poshuku::Provides () const
	{
		return QStringList ("webbrowser");
	}

	QStringList Poshuku::Needs () const
	{
		return QStringList ();
	}

	QStringList Poshuku::Uses () const
	{
		return QStringList ();
	}

	void Poshuku::SetProvider (QObject*, const QString&)
	{
	}

	QIcon Poshuku::GetIcon () const
	{
		return QIcon (":/resources/images/poshuku.svg");
	}
	
	TabClasses_t Poshuku::GetTabClasses () const
	{
		TabClasses_t result;
		result << Core::Instance ().GetTabClass ();
		return result;
	}
	
	void Poshuku::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Poshuku")
			Core::Instance ().NewURL ("", true);
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	QSet<QByteArray> Poshuku::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Poshuku::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Poshuku::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	EntityTestHandleResult Poshuku::CouldHandle (const LeechCraft::Entity& e) const
	{
		return Core::Instance ().CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Poshuku::Handle (LeechCraft::Entity e)
	{
		Core::Instance ().Handle (e);
	}

	void Poshuku::Open (const QString& link)
	{
		Core::Instance ().NewURL (link);
	}

	IWebWidget* Poshuku::GetWidget () const
	{
		return Core::Instance ().GetWidget ();
	}

	QWebView* Poshuku::CreateWindow ()
	{
		return Core::Instance ().MakeWebView ();
	}

	void Poshuku::SetShortcut (const QString& name, const QKeySequences_t& sequences)
	{
		if (name.startsWith ("Browser"))
			Core::Instance ().SetShortcut (name, sequences);
		else
		{
			QAction *act = 0;
			if (name == "EAImportXbel_")
				act = ImportXbel_;
			else if (name == "EAExportXbel_")
				act = ExportXbel_;
			else if (name == "EACheckFavorites_")
				act = CheckFavorites_;
			if (act)
				act->setShortcuts (sequences);
		}
	}

	QMap<QString, ActionInfo> Poshuku::GetActionInfo () const
	{
		BrowserWidget bw;
		QMap<QString, ActionInfo> result = bw.GetActionInfo ();
		result ["EAImportXbel_"] = ActionInfo (ImportXbel_->text (),
				QKeySequence (), ImportXbel_->icon ());
		result ["EAExportXbel_"] = ActionInfo (ExportXbel_->text (),
				QKeySequence (), ExportXbel_->icon ());
		result ["EACheckFavorites_"] = ActionInfo (CheckFavorites_->text (),
				QKeySequence (), CheckFavorites_->icon ());
		return result;
	}

	QList<QAction*> Poshuku::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		switch (place)
		{
		case AEPToolsMenu:
			result << CheckFavorites_;
			result << ToolMenu_->menuAction ();
			break;
		case AEPCommonContextMenu:
			result << ReloadAll_;
			break;
		default:
			break;
		}

		return result;
	}

	void Poshuku::InitConnections ()
	{
		connect (XmlSettingsDialog_.get (),
				SIGNAL (pushButtonClicked (const QString&)),
				this,
				SLOT (handleSettingsClicked (const QString&)));

		connect (&Core::Instance (),
				SIGNAL (addNewTab (const QString&, QWidget*)),
				this,
				SIGNAL (addNewTab (const QString&, QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (&Core::Instance (),
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
		connect (&Core::Instance (),
				SIGNAL (changeTooltip (QWidget*, QWidget*)),
				this,
				SIGNAL (changeTooltip (QWidget*, QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (statusBarChanged (QWidget*, const QString&)),
				this,
				SIGNAL (statusBarChanged (QWidget*, const QString&)));
		connect (&Core::Instance (),
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		connect (&Core::Instance (),
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
				this,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
		connect (&Core::Instance (),
				SIGNAL (error (const QString&)),
				this,
				SLOT (handleError (const QString&)));
	}

	void Poshuku::RegisterSettings ()
	{
		QList<QByteArray> viewerSettings;
		viewerSettings << "StandardFont"
			<< "FixedFont"
			<< "SerifFont"
			<< "SansSerifFont"
			<< "CursiveFont"
			<< "FantasyFont"
			<< "MinimumFontSize"
			<< "DefaultFontSize"
			<< "DefaultFixedFontSize"
			<< "AutoLoadImages"
			<< "DNSPrefetchEnabled"
			<< "AllowJavascript"
			<< "AllowJava"
			<< "AllowPlugins"
			<< "JavascriptCanOpenWindows"
			<< "JavascriptCanAccessClipboard"
			<< "UserStyleSheet"
			<< "OfflineStorageDB"
			<< "LocalStorageDB"
			<< "OfflineWebApplicationCache";
		XmlSettingsManager::Instance ()->RegisterObject (viewerSettings,
				this, "viewerSettingsChanged");

		XmlSettingsManager::Instance ()->RegisterObject ("DeveloperExtrasEnabled",
				this, "developerExtrasChanged");

		viewerSettingsChanged ();
		developerExtrasChanged ();

		QList<QByteArray> cacheSettings;
		cacheSettings << "MaximumPagesInCache"
			<< "MinDeadCapacity"
			<< "MaxDeadCapacity"
			<< "TotalCapacity"
			<< "OfflineStorageQuota";
		XmlSettingsManager::Instance ()->RegisterObject (cacheSettings,
				this, "cacheSettingsChanged");

		cacheSettingsChanged ();
	}

	void Poshuku::createTabFirstTime ()
	{
		bool firstTime = XmlSettingsManager::Instance ()->
				Property ("FirstTimeRun", true).toBool ();
		bool startWithHome = XmlSettingsManager::Instance ()->
				property ("StartWithHomeTab").toBool ();
		if (firstTime || startWithHome)
			Core::Instance ().NewURL ("about:home", true);
		XmlSettingsManager::Instance ()->
				setProperty ("FirstTimeRun", false);
	}

	void Poshuku::viewerSettingsChanged ()
	{
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::StandardFont,
				XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FixedFont,
				XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SerifFont,
				XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SansSerifFont,
				XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::CursiveFont,
				XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FantasyFont,
				XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());
		QWebSettings::globalSettings ()->setFontSize (QWebSettings::MinimumFontSize,
				XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
		QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFontSize,
				XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
		QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
				XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::AutoLoadImages,
				XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::DnsPrefetchEnabled,
				XmlSettingsManager::Instance ()->property ("DNSPrefetchEnabled").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptEnabled,
				XmlSettingsManager::Instance ()->property ("AllowJavascript").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavaEnabled,
				XmlSettingsManager::Instance ()->property ("AllowJava").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::PluginsEnabled,
				XmlSettingsManager::Instance ()->property ("AllowPlugins").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptCanOpenWindows,
				XmlSettingsManager::Instance ()->property ("JavascriptCanOpenWindows").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptCanAccessClipboard,
				XmlSettingsManager::Instance ()->property ("JavascriptCanAccessClipboard").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::OfflineStorageDatabaseEnabled,
				XmlSettingsManager::Instance ()->property ("OfflineStorageDB").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::OfflineWebApplicationCacheEnabled,
				XmlSettingsManager::Instance ()->property ("OfflineWebApplicationCache").toBool ());
		QWebSettings::globalSettings ()->setAttribute (QWebSettings::LocalStorageEnabled,
				XmlSettingsManager::Instance ()->property ("LocalStorageDB").toBool ());
		QWebSettings::globalSettings ()->setUserStyleSheetUrl (QUrl (XmlSettingsManager::
					Instance ()->property ("UserStyleSheet").toString ()));
	}

	void Poshuku::developerExtrasChanged ()
	{
		bool enabled = XmlSettingsManager::Instance ()->
				property ("DeveloperExtrasEnabled").toBool ();
		QWebSettings::globalSettings ()->
				setAttribute (QWebSettings::DeveloperExtrasEnabled, enabled);
		if (enabled && sender ())
			QMessageBox::information (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
					tr ("Please note that Developer Extras would work correctly "
						"only for pages that are loaded after enabling."));
	}

	void Poshuku::cacheSettingsChanged ()
	{
		QWebSettings::setMaximumPagesInCache (XmlSettingsManager::Instance ()->
				property ("MaximumPagesInCache").toInt ());
		QWebSettings::setObjectCacheCapacities (
				XmlSettingsManager::Instance ()->property ("MinDeadCapacity").toDouble () * 1024 * 1024,
				XmlSettingsManager::Instance ()->property ("MaxDeadCapacity").toDouble () * 1024 * 1024,
				XmlSettingsManager::Instance ()->property ("TotalCapacity").toDouble () * 1024 * 1024
				);
		QWebSettings::setOfflineStorageDefaultQuota (XmlSettingsManager::Instance ()->
				property ("OfflineStorageQuota").toInt () * 1024);
	}

	void Poshuku::handleError (const QString& msg)
	{
		emit gotEntity (Util::MakeNotification ("Poshuku", msg, PWarning_));
	}

	void Poshuku::handleNewTab ()
	{
		Core::Instance ().NewURL ("", true);
	}

	void Poshuku::handleSettingsClicked (const QString& name)
	{
		if (name == "CookiesEdit")
		{
			CookiesEditDialog *dia =
					new CookiesEditDialog (Core::Instance ()
							.GetProxy ()->GetMainWindow ());
			dia->show ();
		}
		else if (name == "ClearIconDatabase")
			QWebSettings::clearIconDatabase ();
		else if (name == "ClearMemoryCaches")
			QWebSettings::clearMemoryCaches ();
		else
			qWarning () << Q_FUNC_INFO
				<< "unknown name"
				<< name;
	}

	void Poshuku::handleCheckFavorites ()
	{
		Core::Instance ().CheckFavorites ();
	}

	void Poshuku::handleReloadAll ()
	{
		Core::Instance ().ReloadAll ();
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, LeechCraft::Poshuku::Poshuku);
