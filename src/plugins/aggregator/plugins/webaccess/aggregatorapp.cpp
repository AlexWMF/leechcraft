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

#include "aggregatorapp.h"
#include <QObject>
#include <QThread>
#include <QtDebug>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WBoxLayout>
#include <Wt/WCheckBox>
#include <Wt/WTreeView>
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WOverlayLoadingIndicator>
#include <util/util.h>
#include <interfaces/aggregator/iproxyobject.h>
#include <interfaces/aggregator/channel.h>
#include <interfaces/aggregator/iitemsmodel.h>
#include "readchannelsfilter.h"
#include "util.h"
#include "q2wproxymodel.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace WebAccess
{
	namespace
	{
		class WittyThread : public QThread
		{
			Wt::WApplication * const App_;
		public:
			WittyThread (Wt::WApplication *app)
			: App_ { app }
			{
			}
		protected:
			void run ()
			{
				App_->attachThread (true);
				QThread::run ();
				App_->attachThread (false);
			}
		};
	}

	AggregatorApp::AggregatorApp (IProxyObject *ap, ICoreProxy_ptr cp,
			const Wt::WEnvironment& environment)
	: WApplication (environment)
	, AP_ (ap)
	, CP_ (cp)
	, ObjsThread_ (new WittyThread (this))
	, ChannelsModel_ (new Q2WProxyModel (AP_->GetChannelsModel (), this))
	, ChannelsFilter_ (new ReadChannelsFilter (this))
	, SourceItemModel_ (AP_->CreateItemsModel ())
	, ItemsModel_ (new Q2WProxyModel (SourceItemModel_, this))
	{
		ChannelsModel_->SetRoleMappings (Util::MakeMap<int, int> ({
				{ ChannelRole::UnreadCount, Aggregator::ChannelRoles::UnreadCount },
				{ ChannelRole::CID, Aggregator::ChannelRoles::ChannelID }
			}));
		ItemsModel_->SetRoleMappings (Util::MakeMap<int, int> ({
				{ ItemRole::IID, Aggregator::IItemsModel::ItemRole::ItemId }
			}));

		auto initThread = [this] (QObject *obj) -> void
		{
			obj->moveToThread (ObjsThread_);
			QObject::connect (ObjsThread_,
					SIGNAL (finished ()),
					obj,
					SLOT (deleteLater ()));
		};
		initThread (ChannelsModel_);
		initThread (SourceItemModel_);
		initThread (ItemsModel_);

		ObjsThread_->start ();

		ChannelsFilter_->setSourceModel (ChannelsModel_);

		setTitle ("Aggregator WebAccess");
		setLoadingIndicator (new Wt::WOverlayLoadingIndicator ());

		SetupUI ();

		enableUpdates (true);
	}

	AggregatorApp::~AggregatorApp ()
	{
		delete ChannelsFilter_;

		ObjsThread_->quit ();
		ObjsThread_->wait (1000);
		if (!ObjsThread_->isFinished ())
		{
			qWarning () << Q_FUNC_INFO
					<< "objects thread hasn't finished yet, terminating...";
			ObjsThread_->terminate ();
		}

		delete ObjsThread_;
	}

	void AggregatorApp::HandleChannelClicked (const Wt::WModelIndex& idx)
	{
		ItemView_->setText ({});

		const auto cid = boost::any_cast<IDType_t> (idx.data (ChannelRole::CID));

		const auto iim = qobject_cast<IItemsModel*> (SourceItemModel_);
		iim->reset (cid);

		ItemsTable_->setColumnWidth (0, Wt::WLength (500, Wt::WLength::Pixel));
		ItemsTable_->setColumnWidth (1, Wt::WLength (180, Wt::WLength::Pixel));
	}

	void AggregatorApp::HandleItemClicked (const Wt::WModelIndex& idx)
	{
		if (!idx.isValid ())
			return;

		const auto itemId = boost::any_cast<IDType_t> (idx.data (ItemRole::IID));
		const auto& item = AP_->GetItem (itemId);
		if (!item)
			return;

		auto text = Wt::WString ("<div><a href='{1}' target='_blank'>{2}</a><br />{3}<br /><hr/>{4}</div>")
				.arg (ToW (item->Link_))
				.arg (ToW (item->Title_))
				.arg (ToW (item->PubDate_.toString ()))
				.arg (ToW (item->Description_));
		ItemView_->setText (text);
	}

	void AggregatorApp::SetupUI ()
	{
		setTheme (new Wt::WCssTheme ("polished"));
		auto rootLay = new Wt::WBoxLayout (Wt::WBoxLayout::LeftToRight);
		root ()->setLayout (rootLay);

		auto leftPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);
		rootLay->addLayout (leftPaneLay, 2);

		auto showReadChannels = new Wt::WCheckBox (ToW (QObject::tr ("Include read channels")));
		showReadChannels->setToolTip (ToW (QObject::tr ("Also display channels that have no unread items.")));
		showReadChannels->setChecked (false);
		showReadChannels->checked ().connect ([this] (Wt::NoClass) { ChannelsFilter_->SetHideRead (false); });
		showReadChannels->unChecked ().connect ([this] (Wt::NoClass) { ChannelsFilter_->SetHideRead (true); });
		leftPaneLay->addWidget (showReadChannels);

		auto channelsTree = new Wt::WTreeView ();
		channelsTree->setModel (ChannelsFilter_);
		channelsTree->clicked ().connect (this, &AggregatorApp::HandleChannelClicked);
		channelsTree->setAlternatingRowColors (true);
		leftPaneLay->addWidget (channelsTree, 1, Wt::AlignTop);

		auto rightPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);
		rootLay->addLayout (rightPaneLay, 7);

		ItemsTable_ = new Wt::WTableView ();
		ItemsTable_->setModel (ItemsModel_);
		ItemsTable_->clicked ().connect (this, &AggregatorApp::HandleItemClicked);
		ItemsTable_->setAlternatingRowColors (true);
		ItemsTable_->setWidth (Wt::WLength (100, Wt::WLength::Percentage));
		rightPaneLay->addWidget (ItemsTable_, 2, Wt::AlignJustify);

		ItemView_ = new Wt::WText ();
		ItemView_->setTextFormat (Wt::XHTMLUnsafeText);
		rightPaneLay->addWidget (ItemView_, 5);
	}
}
}
}
