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

#include "customcookiejar.h"
#include <memory>
#include <QNetworkCookie>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	CustomCookieJar::CustomCookieJar (QObject *parent)
	: QNetworkCookieJar (parent)
	, FilterTrackingCookies_ (false)
	, Enabled_ (true)
	, MatchDomainExactly_ (false)
	{
	}

	CustomCookieJar::~CustomCookieJar ()
	{
	}

	void CustomCookieJar::SetFilterTrackingCookies (bool filter)
	{
		FilterTrackingCookies_ = filter;
	}

	void CustomCookieJar::SetEnabled (bool enabled)
	{
		Enabled_ = enabled;
	}

	void CustomCookieJar::SetExactDomainMatch (bool enabled)
	{
		MatchDomainExactly_ = enabled;
	}

	void CustomCookieJar::SetWhitelist (const QList<QRegExp>& list)
	{
		WL_ = list;
	}

	void CustomCookieJar::SetBlacklist (const QList<QRegExp>& list)
	{
		BL_ = list;
	}

	QByteArray CustomCookieJar::Save () const
	{
		QList<QNetworkCookie> cookies = allCookies ();
		QByteArray result;
		for (const auto& cookie : cookies)
		{
			if (cookie.isSessionCookie ())
				continue;

			result += cookie.toRawForm ();
			result += "\n";
		}
		return result;
	}

	void CustomCookieJar::Load (const QByteArray& data)
	{
		QList<QByteArray> spcookies = data.split ('\n');

		QList<QNetworkCookie> cookies, filteredCookies;
		for (const auto& ba : spcookies)
			cookies += QNetworkCookie::parseCookies (ba);

		for (const auto& cookie : cookies)
			if (!(FilterTrackingCookies_ &&
						cookie.name ().startsWith ("__utm")))
				filteredCookies << cookie;
		setAllCookies (filteredCookies);
	}

	void CustomCookieJar::CollectGarbage ()
	{
		QList<QNetworkCookie> cookies = allCookies ();
		QList<QNetworkCookie> result;
		for (const auto& cookie : allCookies ())
			if (!result.contains (cookie))
				result << cookie;
		qDebug () << Q_FUNC_INFO << cookies.size () << result.size ();
		setAllCookies (result);
	}

	QList<QNetworkCookie> CustomCookieJar::cookiesForUrl (const QUrl& url) const
	{
		if (!Enabled_)
			return {};

		QList<QNetworkCookie> filtered;
		for (const auto& cookie : QNetworkCookieJar::cookiesForUrl (url))
			if (!filtered.contains (cookie))
				filtered << cookie;
		return filtered;
	}

	namespace
	{
		bool MatchDomain (QString domain, QString cookieDomain)
		{
			auto normalize = [] (QString& s)
			{
				if (s.startsWith ('.'))
					s = s.mid (1);
			};
			normalize (domain);
			normalize (cookieDomain);

			if (domain == cookieDomain)
				return true;

			const auto idx = domain.indexOf (cookieDomain);
			return idx > 0 && domain.at (idx - 1) == '.';
		}

		bool Check (const QList<QRegExp>& list, const QString& str)
		{
			for (auto& rx : list)
				if (str == rx.pattern () || rx.exactMatch (str))
					return true;

			return false;
		}
	}

	bool CustomCookieJar::setCookiesFromUrl (const QList<QNetworkCookie>& cookieList, const QUrl& url)
	{
		if (!Enabled_)
			return false;

		QList<QNetworkCookie> filtered;
		filtered.reserve (cookieList.size ());
		for (auto cookie : cookieList)
		{
			if (cookie.domain ().isEmpty ())
				cookie.setDomain (url.host ());

			bool checkWhitelist = false;
			std::shared_ptr<void> wlGuard (nullptr, [&] (void*)
					{
						if (checkWhitelist && Check (WL_, cookie.domain ()))
							filtered << cookie;
					});

			if (MatchDomainExactly_ && !MatchDomain (url.host (), cookie.domain ()))
			{
				checkWhitelist = true;
				continue;
			}

			if (FilterTrackingCookies_ &&
					cookie.name ().startsWith ("__utm"))
			{
				checkWhitelist = true;
				continue;
			}

			if (!Check (BL_, cookie.domain ()))
				filtered << cookie;
		}

		return QNetworkCookieJar::setCookiesFromUrl (filtered, url);
	}
}
}
