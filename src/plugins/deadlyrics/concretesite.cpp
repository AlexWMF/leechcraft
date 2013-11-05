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

#include "concretesite.h"
#include <stdexcept>
#include <QDomElement>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LeechCraft
{
namespace DeadLyrics
{
	class MatcherBase
	{
	public:
		enum class Mode
		{
			Return,
			Exclude
		};
	protected:
		const Mode Mode_;

		MatcherBase (Mode mode)
		: Mode_ (mode)
		{
		}
	public:
		virtual ~MatcherBase () {}

		static std::shared_ptr<MatcherBase> MakeMatcher (Mode mode, const QDomElement& item);

		virtual QString operator() (const QString&) const = 0;
	};

	class RangeMatcher : public MatcherBase
	{
		const QString From_;
		const QString To_;
	public:
		RangeMatcher (const QString& from, const QString& to, Mode mode)
		: MatcherBase (mode)
		, From_ (from)
		, To_ (to)
		{
		}

		QString operator() (const QString& string) const
		{
			int fromPos = string.indexOf (From_);
			const int toPos = string.indexOf (To_, fromPos + From_.size ());
			if (fromPos == -1 || toPos == -1)
				return Mode_ == Mode::Exclude ? string : QString ();

			if (Mode_ == Mode::Return)
			{
				fromPos += From_.size ();
				return string.mid (fromPos, toPos - fromPos);
			}
			else
				return string.left (fromPos) + string.mid (toPos + To_.size ());
		}
	};

	class TagMatcher : public MatcherBase
	{
		const QString Tag_;
		QString Name_;
	public:
		TagMatcher (const QString& tag, Mode mode)
		: MatcherBase (mode)
		, Tag_ (tag)
		{
			const int space = tag.indexOf (' ');
			if (space == -1)
				Name_ = tag;
			else
				Name_ = tag.left (space);
			Name_.remove ('<');
			Name_.remove ('>');
		}

		QString operator() (const QString& str) const
		{
			RangeMatcher rm (Tag_, "</" + Name_ + ">", Mode_);
			return rm (str);
		}
	};

	MatcherBase_ptr MatcherBase::MakeMatcher (Mode mode, const QDomElement& item)
	{
		if (item.hasAttribute ("begin") && item.hasAttribute ("end"))
			return MatcherBase_ptr (new RangeMatcher (item.attribute ("begin"), item.attribute ("end"), mode));
		else if (item.hasAttribute ("tag"))
			return MatcherBase_ptr (new TagMatcher (item.attribute ("tag"), mode));
		else
			return MatcherBase_ptr ();
	}

	ConcreteSiteDesc::ConcreteSiteDesc (const QDomElement& elem)
	: Name_ (elem.attribute ("name"))
	, Charset_ (elem.attribute ("charset", "utf-8"))
	, URLTemplate_ (elem.attribute ("url"))
	{
		auto urlFormat = elem.firstChildElement ("urlFormat");
		while (!urlFormat.isNull ())
		{
			const auto& replace = urlFormat.attribute ("replace");
			const auto& with = urlFormat.attribute ("with");
			Q_FOREACH (const auto c, replace)
				Replacements_ [c] = with;

			urlFormat = urlFormat.nextSiblingElement ("urlFormat");
		}

		auto fillMatchers = [&elem] (const QString& name, MatcherBase::Mode mode) -> QList<MatcherBase_ptr>
		{
			QList<MatcherBase_ptr> result;

			auto extract = elem.firstChildElement (name);
			while (!extract.isNull ())
			{
				auto item = extract.firstChildElement ("item");
				while (!item.isNull ())
				{
					result << MatcherBase::MakeMatcher (mode, item);
					item = item.nextSiblingElement ("item");
				}

				extract = extract.nextSiblingElement (name);
			}
			result.removeAll (MatcherBase_ptr ());
			return result;
		};

		Matchers_ += fillMatchers ("extract", MatcherBase::Mode::Return);
		Matchers_ += fillMatchers ("exclude", MatcherBase::Mode::Exclude);
	}

	ConcreteSite::ConcreteSite (const Media::LyricsQuery& query,
			const ConcreteSiteDesc& desc, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Query_ (query)
	, Desc_ (desc)
	{
		auto replace = [this] (QString str) -> QString
		{
			Q_FOREACH (const QChar c, Desc_.Replacements_.keys ())
				str.replace (c, Desc_.Replacements_ [c]);
			return str;
		};

		const auto& artist = replace (query.Artist_.toLower ());
		const auto& album = replace (query.Album_.toLower ());
		const auto& title = replace (query.Title_.toLower ());

		auto urlStr = Desc_.URLTemplate_;
		urlStr.replace ("{artist}", artist);
		urlStr.replace ("{album}", album);
		urlStr.replace ("{title}", title);

		auto cap = [] (QString str) -> QString
		{
			if (!str.isEmpty ())
				str [0] = str [0].toUpper ();
			return str;
		};
		urlStr.replace ("{Artist}", cap (artist));
		urlStr.replace ("{Album}", cap (album));
		urlStr.replace ("{Title}", cap (title));

#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO
				<< "requesting"
				<< urlStr
				<< "from"
				<< Desc_.Name_
				<< "for"
				<< artist
				<< album
				<< title;
#endif

		auto nam = proxy->GetNetworkAccessManager ();

		QUrl url { urlStr };
		QNetworkRequest req { url };

		url.setPath ({});
		url.setQueryItems ({});
		req.setRawHeader ("Referer", url.toString ().toUtf8 ());

		auto reply = nam->get (req);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (deleteLater ()));
	}

	void ConcreteSite::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		const auto& data = reply->readAll ();
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO
				<< "got from"
				<< Desc_.Name_
				<< "the data:"
				<< data;
#endif
		auto str = QString::fromUtf8 (data.constData ());

		for (auto excluder : Desc_.Matchers_)
			str = (*excluder) (str);

		str = str.trimmed ();

		const auto& contentType = reply->header (QNetworkRequest::ContentTypeHeader);
		const bool isPlainText = contentType.toString ().toLower () == "text/plain";
		if (isPlainText)
		{
			str.replace ("\r\n", "<br/>");
			str.replace ("\r", "<br/>");
			str.replace ("\n", "<br/>");
		}

		if (str.size () >= 100)
			emit gotLyrics ({ Query_, { { Desc_.Name_, str } } });
	}
}
}
