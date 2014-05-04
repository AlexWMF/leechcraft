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

#include "core.h"
#include <util/sys/resourceloader.h>
#include "notificationruleswidget.h"
#include "typedmatchers.h"
#include "rulesmanager.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	Core::Core ()
	: RulesManager_ (new RulesManager (this))
	, NRW_ (0)
	, AudioThemeLoader_ (new Util::ResourceLoader ("sounds/"))
	{
		AudioThemeLoader_->AddLocalPrefix ();
		AudioThemeLoader_->AddGlobalPrefix ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		AudioThemeLoader_.reset ();
		delete RulesManager_;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	RulesManager* Core::GetRulesManager () const
	{
		return RulesManager_;
	}

	NotificationRulesWidget* Core::GetNRW ()
	{
		if (!NRW_)
			NRW_ = new NotificationRulesWidget (RulesManager_);
		return NRW_;
	}

	std::shared_ptr<Util::ResourceLoader> Core::GetAudioThemeLoader () const
	{
		return AudioThemeLoader_;
	}

	QList<NotificationRule> Core::GetRules (const Entity& e) const
	{
		const QString& type = e.Additional_ ["org.LC.AdvNotifications.EventType"].toString ();

		QList<NotificationRule> result;

		for (const NotificationRule& rule : RulesManager_->GetRulesList ())
		{
			if (!rule.IsEnabled ())
				continue;

			if (!rule.GetTypes ().contains (type))
				continue;

			bool fieldsMatch = true;
			for (const auto& match : rule.GetFieldMatches ())
			{
				const QString& fieldName = match.GetFieldName ();
				const auto& matcher = match.GetMatcher ();
				if (!matcher->Match (e.Additional_ [fieldName]))
				{
					fieldsMatch = false;
					break;
				}
			}

			if (!fieldsMatch)
				continue;

			if (rule.IsSingleShot ())
				RulesManager_->SetRuleEnabled (rule, false);

			result << rule;
			break;
		}

		return result;
	}

	QString Core::GetAbsoluteAudioPath (const QString& fname) const
	{
		if (fname.contains ('/'))
			return fname;

		const QString& option = XmlSettingsManager::Instance ()
				.property ("AudioTheme").toString ();
		const QString& base = option + '/' + fname;

		QStringList pathVariants;
		pathVariants << base + ".ogg"
				<< base + ".wav"
				<< base + ".flac"
				<< base + ".mp3";

		return GetAudioThemeLoader ()->GetPath (pathVariants);
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}
}
}
