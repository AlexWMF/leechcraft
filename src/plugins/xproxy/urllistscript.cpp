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

#include "urllistscript.h"
#include <QUrl>
#include <QtDebug>
#include <QSettings>
#include <QCoreApplication>

namespace LeechCraft
{
namespace XProxy
{
	bool operator== (const HostInfo& left, const HostInfo& right)
	{
		return left.Port_ == right.Port_ &&
				left.Host_ == right.Host_ &&
				left.Scheme_ == right.Scheme_;
	}

	uint qHash (const HostInfo& info)
	{
		return qHash (info.Host_ + info.Scheme_) + info.Port_;
	}

	UrlListScript::UrlListScript (const IScript_ptr& script, QObject *parent)
	: QObject { parent }
	, Script_ { script }
	{
		script->AddQObject (this, "xproxy");
		ListName_ = Script_->InvokeMethod ("getListName").toString ();

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy_SavedScripts" };
		settings.beginGroup (GetListId ());
		SetUrlsImpl (settings.value ("Urls").toStringList ());
		LastUpdate_ = settings.value ("LastUpdate").toDateTime ();
		settings.endGroup ();
	}

	QByteArray UrlListScript::GetListId () const
	{
		return ListName_.toUtf8 ();
	}

	QString UrlListScript::GetListName () const
	{
		return ListName_;
	}

	void UrlListScript::SetEnabled (bool enabled)
	{
		if (enabled == IsEnabled_)
			return;

		IsEnabled_ = enabled;
		if (!IsEnabled_)
			return;

		if (!LastUpdate_.isValid () || LastUpdate_.secsTo (QDateTime::currentDateTime ()) > 60 * 60)
		{
			refresh ();
			LastUpdate_ = QDateTime::currentDateTime ();
		}
	}

	bool UrlListScript::Accepts (const QString& host, int port, const QString& proto)
	{
		return Hosts_.contains ({ host, port, proto }) ||
				Hosts_.contains ({ host, -1, proto });
	}

	void UrlListScript::setUrls (const QStringList& urls)
	{
		qDebug () << Q_FUNC_INFO << GetListId () << urls.size () << "; was" << Hosts_.size ();

		SetUrlsImpl (urls);

		LastUpdate_ = QDateTime::currentDateTime ();

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy_SavedScripts" };
		settings.beginGroup (GetListId ());
		settings.setValue ("Urls", urls);
		settings.setValue ("LastUpdate", LastUpdate_);
		settings.endGroup ();
	}

	void UrlListScript::SetUrlsImpl (const QStringList& urls)
	{
		Hosts_.clear ();
		for (const auto& urlStr : urls)
		{
			const auto& url = QUrl::fromEncoded (urlStr.toUtf8 ());
			Hosts_.insert ({ url.host (), url.port (), url.scheme () });
		}
	}

	void UrlListScript::refresh ()
	{
		qDebug () << Q_FUNC_INFO << GetListId ();
		Script_->InvokeMethod ("refresh");
	}
}
}
