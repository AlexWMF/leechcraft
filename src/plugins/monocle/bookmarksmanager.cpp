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

#include "bookmarksmanager.h"
#include <QUrl>
#include <QFileInfo>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>
#include "bookmark.h"

namespace LeechCraft
{
namespace Monocle
{
	BookmarksManager::BookmarksManager (QObject *parent)
	: QObject (parent)
	{
		qRegisterMetaType<Bookmark> ("LeechCraft::Monocle::Bookmark");
		qRegisterMetaTypeStreamOperators<Bookmark> ("LeechCraft::Monocle::Bookmark");

		Load ();
	}

	namespace
	{
		QString GetDocID (IDocument_ptr doc)
		{
			const auto& info = doc->GetDocumentInfo ();
			if (!info.Title_.trimmed ().isEmpty ())
				return info.Title_;

			return QFileInfo (doc->GetDocURL ().path ()).fileName ();
		}
	}

	void BookmarksManager::AddBookmark (IDocument_ptr doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto elem = BookmarksDOM_.createElement ("bm");
		bm.ToXML (elem, BookmarksDOM_);
		fileElem.appendChild (elem);

		Save ();
	}

	void BookmarksManager::RemoveBookmark (IDocument_ptr doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			auto next = bmElem.nextSiblingElement ("bm");
			if (Bookmark::FromXML (bmElem) == bm)
				fileElem.removeChild (bmElem);
			bmElem = next;
		}

		Save ();
	}

	QList<Bookmark> BookmarksManager::GetBookmarks (IDocument_ptr doc) const
	{
		QList<Bookmark> result;

		auto fileElem = GetDocElem (GetDocID (doc));
		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			result << Bookmark::FromXML (bmElem);
			bmElem = bmElem.nextSiblingElement ("bm");
		}

		return result;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id)
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				break;

			fileElem = fileElem.nextSiblingElement ("doc");
		}

		if (fileElem.isNull ())
		{
			fileElem = BookmarksDOM_.createElement ("doc");
			fileElem.setAttribute ("id", id);
			BookmarksDOM_.documentElement ().appendChild (fileElem);
		}
		return fileElem;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id) const
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				return fileElem;

			fileElem = fileElem.nextSiblingElement ("doc");
		}
		return QDomElement ();
	}

	void BookmarksManager::Load ()
	{
		if (LoadSaved ())
			return;

		auto docElem = BookmarksDOM_.createElement ("bookmarks");
		docElem.setTagName ("bookmarks");
		docElem.setAttribute ("version", "1");
		BookmarksDOM_.appendChild (docElem);
	}

	bool BookmarksManager::LoadSaved ()
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		if (!dir.exists ("bookmarks.xml"))
			return false;

		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return false;
		}

		if (!BookmarksDOM_.setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing file"
					<< file.fileName ();
			return false;
		}

		return true;
	}

	void BookmarksManager::Save () const
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		file.write (BookmarksDOM_.toByteArray (2));
	}
}
}
