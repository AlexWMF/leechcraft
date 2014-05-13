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

#include "advancednotifications.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/iplugin2.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sys/resourceloader.h>
#include <util/util.h>
#include <util/sys/paths.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "notificationruleswidget.h"
#include "core.h"
#include "rulesmanager.h"
#include "quarkproxy.h"
#include "interfaces/advancednotifications/inotificationbackendplugin.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("advancednotifications");

		Proxy_ = proxy;
		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"advancednotificationssettings.xml");
		SettingsDialog_->SetCustomWidget ("RulesWidget",
				Core::Instance ().GetNRW ());
		SettingsDialog_->SetDataSource ("AudioTheme",
				Core::Instance ().GetAudioThemeLoader ()->GetSubElemModel ());

		GeneralHandler_.reset (new GeneralHandler (proxy));
		connect (GeneralHandler_.get (),
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));

		Component_.reset (new QuarkComponent ("advancednotifications", "ANQuark.qml"));
		Component_->StaticProps_.push_back ({ "AN_quarkTooltip", tr ("Toggle Advanced Notifications rules...") });
		Component_->DynamicProps_.push_back ({ "AN_rulesManager", Core::Instance ().GetRulesManager () });
		Component_->DynamicProps_.push_back ({ "AN_proxy", new QuarkProxy });
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AdvancedNotifications";
	}

	void Plugin::Release ()
	{
		GeneralHandler_.reset ();
		Core::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "Advanced Notifications";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Module for the advanced notifications framework.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/advancednotifications/resources/images/advancednotifications.svg");
		return icon;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_.startsWith ("x-leechcraft/notification") &&
				e.Additional_.contains ("org.LC.AdvNotifications.SenderID") &&
				e.Additional_.contains ("org.LC.AdvNotifications.EventID") &&
				e.Additional_.contains ("org.LC.AdvNotifications.EventCategory");

		if (!can)
			return EntityTestHandleResult ();

		EntityTestHandleResult result (EntityTestHandleResult::PIdeal);
		result.CancelOthers_ = true;
		return result;
	}

	void Plugin::Handle (Entity e)
	{
		GeneralHandler_->Handle (e);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { Component_ };
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << GetUniqueID () + ".NotificationsBackend";
		return result;
	}

	void Plugin::AddPlugin (QObject *obj)
	{
		const auto ip2 = qobject_cast<IPlugin2*> (obj);
		const auto& classes = ip2->GetPluginClasses ();

		if (classes.contains (GetUniqueID () + ".NotificationsBackend"))
		{
			const auto inbp = qobject_cast<INotificationBackendPlugin*> (obj);
			for (const auto& handler : inbp->GetNotificationHandlers ())
				GeneralHandler_->RegisterHandler (handler);
		}
	}

	QList<Entity> Plugin::GetAllRules (const QString& category) const
	{
		return Core::Instance ().GetRulesManager ()->GetAllRules (category);
	}

	void Plugin::RequestRuleConfiguration (const Entity& rule)
	{
		Core::Instance ().GetRulesManager ()->SuggestRuleConfiguration (rule);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_advancednotifications, LeechCraft::AdvancedNotifications::Plugin);
