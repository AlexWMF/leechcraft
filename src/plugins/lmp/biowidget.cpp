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

#include "biowidget.h"

#if QT_VERSION < 0x050000
#include <QDeclarativeView>
#else
#include <QQuickWidget>
#endif

#include <QtDebug>
#include <util/util.h>
#include <util/qml/standardnamfactory.h>
#include <util/sys/paths.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iinfo.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "bioviewmanager.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	BioWidget::BioWidget (QWidget *parent)
	: QWidget (parent)
#if QT_VERSION < 0x050000
	, View_ (new QDeclarativeView)
#else
	, View_ (new QQuickWidget)
#endif
	{
		Ui_.setupUi (this);

#if QT_VERSION < 0x050000
		View_->setResizeMode (QDeclarativeView::SizeRootObjectToView);
#else
		View_->setResizeMode (QQuickWidget::SizeRootObjectToView);
#endif
		View_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		layout ()->addWidget (View_);

		new Util::StandardNAMFactory ("lmp/qml",
				[] { return 50 * 1024 * 1024; },
				View_->engine ());

		Manager_ = new BioViewManager (View_, this);
		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "lmp", "BioView.qml"));
		Manager_->InitWithSource ();

		const auto& lastProv = XmlSettingsManager::Instance ()
				.Property ("LastUsedBioProvider", QString ()).toString ();

		auto providerObjs = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IArtistBioFetcher*> ();
		for (auto providerObj : providerObjs)
		{
			const auto provider = qobject_cast<Media::IArtistBioFetcher*> (providerObj);

			Providers_ << provider;

			const auto& icon = qobject_cast<IInfo*> (providerObj)->GetIcon ();
			Ui_.Provider_->addItem (icon, provider->GetServiceName ());
			if (lastProv == provider->GetServiceName ())
				Ui_.Provider_->setCurrentIndex (Ui_.Provider_->count () - 1);
		}

		connect (Ui_.Provider_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (requestBiography ()));
		connect (Ui_.Provider_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (saveLastUsedProv ()));

		connect (Manager_,
				SIGNAL (gotArtistImage (QString, QUrl)),
				this,
				SIGNAL (gotArtistImage (QString, QUrl)));
	}

	void BioWidget::SetCurrentArtist (const QString& artist)
	{
		if (artist.isEmpty () || artist == CurrentArtist_)
			return;

		CurrentArtist_ = artist;
		requestBiography ();
	}

	void BioWidget::saveLastUsedProv ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		const auto& prov = idx >= 0 ?
				Providers_.value (idx)->GetServiceName () :
				QString ();

		XmlSettingsManager::Instance ().setProperty ("LastUsedBioProvider", prov);
	}

	void BioWidget::requestBiography ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		if (idx < 0 || CurrentArtist_.isEmpty ())
			return;

		Manager_->Request (Providers_ [idx], CurrentArtist_);
	}
}
}
