/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

class QWebView;
namespace LeechCraft
{
namespace Blogique
{
	class Plugin : public QObject
				, public IInfo
				, public IHaveTabs
				, public IHaveSettings
				, public IPluginReady
				, public IActionsExporter
				, public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings IPluginReady IActionsExporter
				IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Blogique")

		TabClasses_t TabClasses_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		QAction *ExportAction_;

	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject* plugin);

		QList<QAction*> GetActions (ActionsEmbedPlace area) const;

		void RecoverTabs (const QList<TabRecoverInfo>& infos);
	private:
		void CreateTab ();

	signals:
		void addNewTab (const QString& name, QWidget *tabContents);
		void removeTab (QWidget *tabContents);
		void changeTabName (QWidget *tabContents, const QString& name);
		void changeTabIcon (QWidget *tabContents, const QIcon& icon);
		void statusBarChanged (QWidget *tabContents, const QString& text);
		void raiseTab (QWidget *tabContents);

		void gotEntity (const LeechCraft::Entity& e);
		void delegateEntity (const LeechCraft::Entity& e, int *id, QObject **obj);

		void gotActions (QList<QAction*> actions, LeechCraft::ActionsEmbedPlace area);
	};
}
}
