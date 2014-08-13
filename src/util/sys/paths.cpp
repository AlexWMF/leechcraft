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

#include "paths.h"
#include <stdexcept>
#include <QFile>
#include <QTemporaryFile>
#if defined (Q_OS_WIN32) || defined (Q_OS_MAC)
#include <QApplication>
#endif
#include <QtDebug>
#include <QDir>
#include <QUrl>

#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

#include <util/util.h>

namespace LeechCraft
{
namespace Util
{
	QStringList GetPathCandidates (SysPath path, QString suffix)
	{
		if (!suffix.isEmpty () && suffix.at (suffix.size () - 1) != '/')
			suffix += '/';

		QStringList candidates;
		switch (path)
		{
		case SysPath::QML:
#if QT_VERSION < 0x050000
			return GetPathCandidates (SysPath::Share, "qml/" + suffix);
#else
			return GetPathCandidates (SysPath::Share, "qml5/" + suffix);
#endif
		case SysPath::Share:
#ifdef Q_OS_WIN32
			candidates << QApplication::applicationDirPath () + "/share/" + suffix;
#elif defined (Q_OS_MAC)
			if (QApplication::arguments ().contains ("-nobundle"))
				candidates << "/usr/local/share/leechcraft/" + suffix;
			else
				candidates << QApplication::applicationDirPath () + "/../Resources/share/" + suffix;
#else
			candidates << "/usr/local/share/leechcraft/" + suffix
					<< "/usr/share/leechcraft/" + suffix;
#endif
			return candidates;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown system path"
				<< static_cast<int> (path);
		return QStringList ();
	}

	QString GetSysPath (SysPath path, const QString& suffix, const QString& filename)
	{
		for (const QString& cand : GetPathCandidates (path, suffix))
			if (QFile::exists (cand + filename))
				return cand + filename;

		qWarning () << Q_FUNC_INFO
				<< "unable to find"
				<< suffix
				<< filename;
		return QString ();
	}

	QUrl GetSysPathUrl (SysPath path, const QString& subfolder, const QString& filename)
	{
		return QUrl::fromLocalFile (GetSysPath (path, subfolder, filename));
	}

	QStringList GetSystemPaths ()
	{
		return QString (qgetenv ("PATH")).split (":", QString::SkipEmptyParts);
	}

	QString FindInSystemPath (const QString& name, const QStringList& paths,
			const std::function<bool (QFileInfo)>& filter)
	{
		for (const auto& dir : paths)
		{
			const QFileInfo fi (dir + '/' + name);
			if (!fi.exists ())
				continue;

			if (filter && !filter (fi))
				continue;

			return fi.absoluteFilePath ();
		}

		return {};
	}

	QDir GetUserDir (UserDir dir, const QString& subpath)
	{
		QString path;
		switch (dir)
		{
		case UserDir::Cache:
#if QT_VERSION < 0x050000
			path = QDesktopServices::storageLocation (QDesktopServices::CacheLocation);
#else
			path = QStandardPaths::writableLocation (QStandardPaths::CacheLocation);
#endif
			break;
		case UserDir::LC:
			path = QDir::home ().path () + "/.leechcraft/";
			break;
		}

		if (path.isEmpty ())
			throw std::runtime_error ("cannot get root path");

		if (!path.endsWith ('/'))
			path += '/';
		if (dir != UserDir::LC)
			path += "leechcraft/";
		path += subpath;

		if (!QDir {}.exists (path) &&
				!QDir {}.mkpath (path))
			throw std::runtime_error ("cannot create path " + path.toStdString ());

		return { path };
	}

	QDir CreateIfNotExists (QString path)
	{
		auto home = QDir::home ();
		path.prepend (".leechcraft/");

		if (!home.exists (path) &&
				!home.mkpath (path))
			throw std::runtime_error (qPrintable (QObject::tr ("Could not create %1")
						.arg (QDir::toNativeSeparators (home.filePath (path)))));

		if (home.cd (path))
			return home;
		else
			throw std::runtime_error (qPrintable (QObject::tr ("Could not cd into %1")
						.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}

	QString GetTemporaryName (const QString& pattern)
	{
		QTemporaryFile file (QDir::tempPath () + "/" + pattern);
		file.open ();
		QString name = file.fileName ();
		file.close ();
		file.remove ();
		return name;
	}
}
}
