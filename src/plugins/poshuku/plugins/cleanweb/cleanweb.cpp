/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "cleanweb.h"
#include <typeinfo>
#include <QIcon>
#include <QTextCodec>
#include <QtDebug>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "subscriptionsmanager.h"
#include "flashonclickplugin.h"
#include "flashonclickwhitelist.h"
#include "userfilters.h"
#include "wizardgenerator.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	void CleanWeb::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku_cleanweb"));

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukucleanwebsettings.xml");

		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&,
						int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&,
						int*, QObject**)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));

		Core::Instance ().SetProxy (proxy);

		SettingsDialog_->SetCustomWidget ("SubscriptionsManager",
				new SubscriptionsManager ());
		SettingsDialog_->SetCustomWidget ("UserFilters",
				new UserFilters ());
		SettingsDialog_->SetCustomWidget ("FlashOnClickWhitelist",
				Core::Instance ().GetFlashOnClickWhitelist ());
	}

	void CleanWeb::SecondInit ()
	{
	}

	void CleanWeb::Release ()
	{
	}

	QByteArray CleanWeb::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.CLeanWeb";
	}

	QString CleanWeb::GetName () const
	{
		return "Poshuku CleanWeb";
	}

	QString CleanWeb::GetInfo () const
	{
		return tr ("Blocks unwanted ads.");
	}

	QIcon CleanWeb::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/poshuku/plugins/cleanweb/resources/images/poshuku_cleanweb.svg");
		return icon;
	}

	QStringList CleanWeb::Provides () const
	{
		return QStringList ();
	}

	QStringList CleanWeb::Needs () const
	{
		return QStringList ("http");
	}

	QStringList CleanWeb::Uses () const
	{
		return QStringList ();
	}

	void CleanWeb::SetProvider (QObject*, const QString&)
	{
	}

	Util::XmlSettingsDialog_ptr CleanWeb::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	EntityTestHandleResult CleanWeb::CouldHandle (const Entity& e) const
	{
		return Core::Instance ().CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void CleanWeb::Handle (Entity e)
	{
		Core::Instance ().Handle (e);
	}

	QList<QWizardPage*> CleanWeb::GetWizardPages () const
	{
		std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
		return wg->GetPages ();
	}

	QSet<QByteArray> CleanWeb::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void CleanWeb::hookWebPluginFactoryReload (LeechCraft::IHookProxy_ptr,
			QList<LeechCraft::Poshuku::IWebPlugin*>& plugins)
	{
		plugins << Core::Instance ().GetFlashOnClick ();
	}

	void CleanWeb::hookInitialLayoutCompleted (IHookProxy_ptr, QWebPage *page, QWebFrame *frame)
	{
		Core::Instance ().HandleInitialLayout (page, frame);
	}

	void CleanWeb::hookNAMCreateRequest (IHookProxy_ptr proxy,
			QNetworkAccessManager *manager,
			QNetworkAccessManager::Operation *op,
			QIODevice **dev)
	{
		Core::Instance ().Hook (proxy, manager, op, dev);
	}

	void CleanWeb::hookExtension (LeechCraft::IHookProxy_ptr proxy,
			QWebPage *page,
			QWebPage::Extension ext,
			const QWebPage::ExtensionOption *opt,
			QWebPage::ExtensionReturn *ret)
	{
		Core::Instance ().HandleExtension (proxy, page, ext, opt, ret);
	}

	void CleanWeb::hookWebViewContextMenu (IHookProxy_ptr,
			QWebView *view,
			QContextMenuEvent*,
			const QWebHitTestResult& r,
			QMenu *menu,
			WebViewCtxMenuStage stage)
	{
		Core::Instance ().HandleContextMenu (r, view, menu, stage);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_cleanweb, LeechCraft::Poshuku::CleanWeb::CleanWeb);
