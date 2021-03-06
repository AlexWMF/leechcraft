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

#include "urleditbuttonsmanager.h"
#include <QToolButton>
#include <QMenu>
#include <qwebframe.h>
#include <qwebelement.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include "customwebview.h"
#include "core.h"
#include "progresslineedit.h"
#include "webpagesslwatcher.h"
#include "sslstatedialog.h"

namespace LeechCraft
{
namespace Poshuku
{
	UrlEditButtonsManager::UrlEditButtonsManager (CustomWebView *view,
			ProgressLineEdit *edit, WebPageSslWatcher *watcher, QAction *add2favs)
	: QObject { view }
	, View_ { view }
	, LineEdit_ { edit }
	, SslWatcher_ { watcher }
	, Add2Favorites_ { add2favs }
	, SslStateAction_ { new QAction { this } }
	, ExternalLinks_ { new QMenu { } }
	, ExternalLinksAction_ { new QAction { this } }
	{
		ExternalLinks_->menuAction ()->setText (tr ("External links"));

		ExternalLinksAction_->setText ("External links");
		ExternalLinksAction_->setProperty ("ActionIcon", "application-rss+xml");

		connect (&Core::Instance (),
				SIGNAL (bookmarkAdded (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (&Core::Instance (),
				SIGNAL (bookmarkRemoved (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (LineEdit_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (View_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (updateBookmarksState ()));
		connect (View_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (checkLinkRels ()));

		LineEdit_->InsertAction (Add2Favorites_, 0, true);

		connect (SslWatcher_,
				SIGNAL (sslStateChanged (WebPageSslWatcher*)),
				this,
				SLOT (handleSslState ()));

		LineEdit_->InsertAction (SslStateAction_, 0, false);
		LineEdit_->SetVisible (SslStateAction_, false);

		connect (SslStateAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (showSslDialog ()));
	}

	void UrlEditButtonsManager::handleSslState ()
	{
		QString iconName;
		QString title;
		switch (SslWatcher_->GetPageState ())
		{
		case WebPageSslWatcher::State::NoSsl:
			LineEdit_->SetVisible (SslStateAction_, false);
			return;
		case WebPageSslWatcher::State::SslErrors:
			iconName = "security-low";
			title = tr ("Some SSL errors where encountered.");
			break;
		case WebPageSslWatcher::State::UnencryptedElems:
			iconName = "security-medium";
			title = tr ("Some elements were loaded via unencrypted connection.");
			break;
		case WebPageSslWatcher::State::FullSsl:
			iconName = "security-high";
			title = tr ("Everything is secure!");
			break;
		}

		const auto iconMgr = Core::Instance ().GetProxy ()->GetIconThemeManager ();
		SslStateAction_->setIcon (iconMgr->GetIcon (iconName));

		LineEdit_->SetVisible (SslStateAction_, true);
	}

	void UrlEditButtonsManager::checkPageAsFavorite (const QString& url)
	{
		if (url != View_->url ().toString () &&
				url != LineEdit_->text ())
			return;

		if (Core::Instance ().IsUrlInFavourites (url))
		{
			Add2Favorites_->setProperty ("ActionIcon", "list-remove");
			Add2Favorites_->setText (tr ("Remove bookmark"));
			Add2Favorites_->setToolTip (tr ("Remove bookmark"));

			if (auto btn = LineEdit_->GetButtonFromAction (Add2Favorites_))
				btn->setIcon (Core::Instance ().GetProxy ()->
						GetIconThemeManager ()->GetIcon ("list-remove"));
		}
		else
		{
			Add2Favorites_->setProperty ("ActionIcon", "bookmark-new");
			Add2Favorites_->setText (tr ("Add bookmark"));
			Add2Favorites_->setToolTip (tr ("Add bookmark"));

			if (auto btn = LineEdit_->GetButtonFromAction (Add2Favorites_))
				btn->setIcon (Core::Instance ().GetProxy ()->
						GetIconThemeManager ()->GetIcon ("bookmark-new"));
		}
	}

	void UrlEditButtonsManager::checkLinkRels ()
	{
		LineEdit_->RemoveAction (ExternalLinksAction_);

		ExternalLinks_->clear ();

		const auto entityMgr = Core::Instance ().GetProxy ()->GetEntityManager ();

		const auto& links = View_->page ()->mainFrame ()->findAllElements ("link");
		const auto& mainFrameURL = View_->page ()->mainFrame ()->url ();
		bool inserted = false;
		for (const auto& link : links)
		{
			if (link.attribute ("type") == "")
				continue;

			LeechCraft::Entity e;
			e.Mime_ = link.attribute ("type");

			QString entity = link.attribute ("title");
			if (entity.isEmpty ())
			{
				entity = e.Mime_;
				entity.remove ("application/");
				entity.remove ("+xml");
				entity = entity.toUpper ();
			}

			auto entityUrl = mainFrameURL.resolved (QUrl (link.attribute ("href")));
			e.Entity_ = entityUrl;
			e.Additional_ ["SourceURL"] = entityUrl;
			e.Parameters_ = LeechCraft::FromUserInitiated |
				LeechCraft::OnlyHandle;
			e.Additional_ ["UserVisibleName"] = entity;
			e.Additional_ ["LinkRel"] = link.attribute ("rel");
			e.Additional_ ["IgnorePlugins"] = QStringList ("org.LeechCraft.Poshuku");

			if (entityMgr->CouldHandle (e))
			{
				QString mime = e.Mime_;
				mime.replace ('/', '_');
				auto act = ExternalLinks_->addAction (QIcon (QString (":/resources/images/%1.png")
							.arg (mime)),
						entity);

				new Util::SlotClosure<Util::NoDeletePolicy>
				{
					[entityMgr, e] { entityMgr->HandleEntity (e); },
					act,
					SIGNAL (triggered ()),
					act
				};

				if (!inserted)
				{
					auto btn = LineEdit_->InsertAction (ExternalLinksAction_);
					LineEdit_->SetVisible (ExternalLinksAction_, true);
					btn->setMenu (ExternalLinks_.get ());
					btn->setArrowType (Qt::NoArrow);
					btn->setPopupMode (QToolButton::InstantPopup);
					const QString newStyle ("::menu-indicator { image: "
							"url(data:image/gif;base64,R0lGODlhAQABAPABAP///"
							"wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw==);}");
					btn->setStyleSheet (btn->styleSheet () + newStyle);

					connect (ExternalLinks_->menuAction (),
							SIGNAL (triggered ()),
							this,
							SLOT (showSendersMenu ()),
							Qt::UniqueConnection);
					inserted = true;
				}
			}
		}
	}

	void UrlEditButtonsManager::showSendersMenu ()
	{
		auto action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction"
				<< sender ();
			return;
		}

		auto menu = action->menu ();
		menu->exec (QCursor::pos ());
	}

	void UrlEditButtonsManager::showSslDialog ()
	{
		const auto dia = new SslStateDialog { SslWatcher_ };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	void UrlEditButtonsManager::updateBookmarksState ()
	{
		checkPageAsFavorite (View_->url ().toString ());
	}
}
}
