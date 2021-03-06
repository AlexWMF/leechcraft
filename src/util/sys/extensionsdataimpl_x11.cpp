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

#include "extensionsdataimpl.h"
#include <QFile>
#include <QIcon>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	namespace
	{
		QHash<QString, QString> ParseMimeTypes ()
		{
			QFile file { "/etc/mime.types" };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open /etc/mime.types:"
						<< file.errorString ();
				return {};
			}

			QRegExp wsRx { "\\s+" };

			QHash<QString, QString> result;
			while (!file.atEnd ())
			{
				const auto& line = file.readLine ().trimmed ();
				const auto& elems = QString::fromLatin1 (line).split (wsRx);

				const auto& mime = elems.at (0);
				for (int i = 1; i < elems.size (); ++i)
					result [elems.at (i)] = mime;
			}
			return result;
		}

		QStringList GetMimeDirs ()
		{
			auto list = qgetenv ("XDG_DATA_HOME").split (':') +
					qgetenv ("XDG_DATA_DIRS").split (':');
			if (list.isEmpty ())
				list << "/usr/share";

			QStringList result;
			for (const auto& item : list)
				if (QFile::exists (item + "/mime"))
					result << item + "/mime/";
			return result;
		}

		void ParseIconsMappings (QHash<QString, QString>& result, const QString& filename)
		{
			QFile file { filename };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open"
						<< filename
						<< file.errorString ();
				return;
			}

			while (!file.atEnd ())
			{
				const auto& line = QString::fromLatin1 (file.readLine ().trimmed ());
				if (!line.indexOf (':'))
					continue;

				result [line.section (':', 0, 0)] = line.section (':', 1, 1);
			}
		}

		QHash<QString, QString> ParseIconsMappings ()
		{
			QHash<QString, QString> result;
			for (const auto& mimeDir : GetMimeDirs ())
			{
				ParseIconsMappings (result, mimeDir + "generic-icons");
				ParseIconsMappings (result, mimeDir + "icons");
			}
			return result;
		}
	}

	struct ExtensionsDataImpl::Details
	{
		QHash<QString, QString> MimeDatabase_;
		QHash<QString, QString> IconsMappings_;

		Details ();
	};

	ExtensionsDataImpl::Details::Details ()
	: MimeDatabase_ { ParseMimeTypes () }
	, IconsMappings_ { ParseIconsMappings () }
	{
	}

	ExtensionsDataImpl::ExtensionsDataImpl ()
	: Details_ { new Details }
	{
	}

	const QHash<QString, QString>& ExtensionsDataImpl::GetMimeDatabase () const
	{
		return Details_->MimeDatabase_;
	}

	QIcon ExtensionsDataImpl::GetExtIcon (const QString& extension) const
	{
		return GetMimeIcon (GetMimeDatabase ().value (extension));
	}

	QIcon ExtensionsDataImpl::GetMimeIcon (const QString& mime) const
	{
		auto iconName = Details_->IconsMappings_.value (mime);
		if (iconName.isEmpty ())
			iconName = mime.section ('/', 0, 0) + "-x-generic";

		auto result = QIcon::fromTheme (iconName);
		if (result.isNull ())
			result = QIcon::fromTheme (mime.section ('/', 0, 0) + "-x-generic");
		if (result.isNull ())
			result = QIcon::fromTheme ("unknown");
		return result;
	}
}
}
