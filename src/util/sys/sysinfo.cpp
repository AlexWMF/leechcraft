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

#include "sysinfo.h"
#if !defined(Q_OS_WIN32)
#include <sys/utsname.h>
#endif

#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QSettings>

namespace LeechCraft
{
namespace Util
{
namespace SysInfo
{
	QString GetOSName ()
	{
		const auto& pair = GetOSNameSplit ();
		return pair.first + ' ' + pair.second;
	}

	typedef QPair<QString, QString> SplitInfo_t;

	namespace Linux
	{
		QString GetLSBName ()
		{
			QProcess proc;

			proc.start (QString ("/bin/sh"),
						QStringList ("-c") << "lsb_release -ds", QIODevice::ReadOnly);
			if (proc.waitForStarted ())
			{
				QTextStream stream (&proc);
				QString ret;
				while (proc.waitForReadyRead ())
					ret += stream.readAll ();
				proc.close ();
				if (!ret.isEmpty ())
					return ret.remove ('"').trimmed ();
			}

			return {};
		}

		QString GetEtcOsName ()
		{
			if (!QFile::exists ("/etc/os-release"))
				return {};

			QSettings relFile { "/etc/os-release", QSettings::IniFormat };
			relFile.setIniCodec ("UTF-8");

			const auto& prettyName = relFile.value ("PRETTY_NAME").toString ();
			const auto& name = relFile.value ("NAME").toString ();
			const auto& version = relFile.value ("VERSION").toString ();
			return !prettyName.isEmpty () ? prettyName : (name + " " + version);
		}

		QString GetEtcName ()
		{
			struct OsInfo_t
			{
				QString path;
				QString name;
			} OsInfo [] =
			{
				{ "/etc/mandrake-release", "Mandrake Linux" },
				{ "/etc/debian_version", "Debian GNU/Linux" },
				{ "/etc/gentoo-release", "Gentoo Linux" },
				{ "/etc/exherbo-release", "Exherbo" },
				{ "/etc/arch-release", "Arch Linux" },
				{ "/etc/slackware-version", "Slackware Linux" },
				{ "/etc/pld-release", "" },
				{ "/etc/lfs-release", "LFS" },
				{ "/etc/SuSE-release", "SuSE linux" },
				{ "/etc/conectiva-release", "Connectiva" },
				{ "/etc/.installed", "" },
				{ "/etc/redhat-release", "" },
				{ "", "" }
			};
			OsInfo_t *osptr = OsInfo;
			while (!osptr->path.isEmpty ())
			{
				QFileInfo fi (osptr->path);
				if (fi.exists ())
				{
					QFile f (osptr->path);
					f.open (QIODevice::ReadOnly);
					QString data = QString (f.read (1024)).trimmed ();
					if (osptr->name.isEmpty ())
						return data;
					else
						return QString ("%1 (%2)")
								.arg (osptr->name)
								.arg (data);
				}
				++osptr;
			}

			return {};
		}
	}

	namespace
	{
#ifndef Q_OS_MAC
		void Normalize (QString& osName)
		{
			auto trimQuotes = [&osName]
			{
				if (osName.startsWith ('"') && osName.endsWith ('"'))
					osName = osName.mid (1, osName.size () - 1);
			};

			trimQuotes ();

			const QString nameMarker ("NAME=");
			if (osName.startsWith (nameMarker))
				osName = osName.mid (nameMarker.size ());

			trimQuotes ();
		}
#endif
	}

	QPair<QString, QString> GetOSNameSplit ()
	{
#if defined(Q_OS_MAC)
		QSysInfo::MacVersion v = QSysInfo::MacintoshVersion;
		switch (v)
		{
		case QSysInfo::MV_10_3:
			return { "Mac OS X", "10.3" };
		case QSysInfo::MV_10_4:
			return { "Mac OS X", "10.4" };
		case QSysInfo::MV_10_5:
			return { "Mac OS X", "10.5" };
		case QSysInfo::MV_10_6:
			return { "Mac OS X", "10.6" };
		case QSysInfo::MV_10_7:
			return { "Mac OS X", "10.7" };
		case QSysInfo::MV_10_8:
			return { "Mac OS X", "10.8" };
		case QSysInfo::MV_10_9:
			return { "Mac OS X", "10.9" };
		default:
			return { "Max OS X", "Unknown version" };
		}
#elif defined(Q_OS_WIN32)
		QSysInfo::WinVersion v = QSysInfo::WindowsVersion;
		if (v == QSysInfo::WV_95)
			return SplitInfo_t ("Windows", "95");
		else if (v == QSysInfo::WV_98)
			return SplitInfo_t ("Windows", "98");
		else if (v == QSysInfo::WV_Me)
			return SplitInfo_t ("Windows", "Me");
		else if (v == QSysInfo::WV_DOS_based)
			return SplitInfo_t ("Windows", "9x/Me");
		else if (v == QSysInfo::WV_NT)
			return SplitInfo_t ("Windows", "NT 4.x");
		else if (v == QSysInfo::WV_2000)
			return SplitInfo_t ("Windows", "2000");
		else if (v == QSysInfo::WV_XP)
			return SplitInfo_t ("Windows", "XP");
		else if (v == QSysInfo::WV_2003)
			return SplitInfo_t ("Windows", "2003");
		else if (v == QSysInfo::WV_VISTA)
			return SplitInfo_t ("Windows", "Vista");
		else if (v == QSysInfo::WV_WINDOWS7)
			return SplitInfo_t ("Windows", "7");
		else if (v == 0x00a0)
			return SplitInfo_t ("Windows", "8");
		else if (v == QSysInfo::WV_NT_based)
			return SplitInfo_t ("Windows", "NT-based");
#else
		auto osName = Linux::GetEtcOsName ();

		if (osName.isEmpty ())
			osName = Linux::GetEtcName ();

		if (osName.isEmpty ())
			osName = Linux::GetLSBName ();

		Normalize (osName);

		utsname u;
		uname (&u);

		return qMakePair (osName.isEmpty () ? QString (u.sysname) : osName,
				QString ("%1 %2 %3").arg (u.machine, u.release, u.version));
#endif

		return qMakePair (QString ("Unknown OS"), QString ("Unknown version"));
	}
}
}
}
