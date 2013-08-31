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

#include "rootpathsettingsmanager.h"
#include <QStandardItemModel>
#include <QFile>
#include <QMessageBox>
#include <xmlsettingsdialog/datasourceroles.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	RootPathSettingsManager::RootPathSettingsManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		QStandardItem *item = new QStandardItem (tr ("Path"));
		item->setData (DataSources::DataFieldType::LocalPath,
				DataSources::DataSourceRole::FieldType);
		Model_->setHorizontalHeaderItem (0, item);

		auto collection = Core::Instance ().GetLocalCollection ();
		connect (collection,
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (handleRootPathsChanged ()));
		handleRootPathsChanged ();
	}

	QAbstractItemModel* RootPathSettingsManager::GetModel () const
	{
		return Model_;
	}

	void RootPathSettingsManager::addRequested (const QString&, const QVariantList& list)
	{
		if (!XmlSettingsManager::Instance ().Property ("HasAskedAboutAAFetch", false).toBool ())
		{
			XmlSettingsManager::Instance ().setProperty ("HasAskedAboutAAFetch", true);
			const auto fetch = QMessageBox::question (nullptr,
					"LeechCraft",
					tr ("Do you want LMP to automatically fetch missing album art? It is done in "
						"the background and won't disturb you, but can consume quite some traffic "
						"and local storage space, especially if you have a lot of albums in your "
						"collection.<br/><br/>You can always toggle this option later in LMP "
						"settings"),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

			XmlSettingsManager::Instance ().setProperty ("AutoFetchAlbumArt", fetch);
		}

		const QString& str = list.value (0).toString ();
		if (QFile::exists (str))
			Core::Instance ().GetLocalCollection ()->Scan (str);
	}

	void RootPathSettingsManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		QStringList paths;
		Q_FOREACH (const auto& idx, indexes)
			paths << idx.data ().toString ();

		auto coll = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const auto& path, paths)
			coll->Unscan (path);
	}

	void RootPathSettingsManager::handleRootPathsChanged ()
	{
		while (Model_->rowCount ())
			Model_->removeRow (0);

		const auto& dirs = Core::Instance ().GetLocalCollection ()->GetDirs ();
		Q_FOREACH (const auto& dir, dirs)
			Model_->appendRow (new QStandardItem (dir));
	}
}
}
