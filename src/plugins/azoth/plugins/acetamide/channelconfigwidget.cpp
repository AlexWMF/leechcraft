/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "channelconfigwidget.h"
#include <QStandardItemModel>
#include "sortfilterproxymodel.h"
#include "channelclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelConfigWidget::ChannelConfigWidget (ChannelCLEntry *clentry, QWidget *parent) 
	: QWidget (parent)
	, ChannelEntry_ (clentry)
	, IsWidgetRequest_ (false)
	{
		Ui_.setupUi (this);

		BanModel_ = new QStandardItemModel (this);
		ExceptModel_ = new QStandardItemModel (this);
		InviteModel_ = new QStandardItemModel (this);
		BanFilterModel_ = new SortFilterProxyModel (this);
		ExceptFilterModel_ = new SortFilterProxyModel (this);
		InviteFilterModel_ = new SortFilterProxyModel (this);

		BanModel_->setColumnCount (3);
		BanModel_->setHorizontalHeaderLabels (QStringList () << tr ("Ban mask")
				<< tr ("Set by")
				<< tr ("Date"));

		ExceptModel_->setColumnCount (3);
		ExceptModel_->setHorizontalHeaderLabels (QStringList () << tr ("Except mask")
				<< tr ("Set by")
				<< tr ("Date"));

		InviteModel_->setColumnCount (3);
		InviteModel_->setHorizontalHeaderLabels (QStringList () << tr ("Invite mask")
				<< tr ("Set by")
				<< tr ("Date"));

		Ui_.BanList_->horizontalHeader ()->setResizeMode (QHeaderView::Stretch);
		Ui_.BanList_->setModel (BanFilterModel_);
		Ui_.ExceptList_->horizontalHeader ()->setResizeMode (QHeaderView::Stretch);
		Ui_.ExceptList_->setModel (ExceptFilterModel_);
		Ui_.InviteList_->horizontalHeader ()->setResizeMode (QHeaderView::Stretch);
		Ui_.InviteList_->setModel (InviteFilterModel_);
		BanFilterModel_->setSourceModel (BanModel_);
		ExceptFilterModel_->setSourceModel (ExceptModel_);
		InviteFilterModel_->setSourceModel (InviteModel_);

		ChannelMode_ = ChannelEntry_->GetChannelModes ();
		SetModesUi ();

		connect (ChannelEntry_,
				SIGNAL (gotBanListItem (const QString&, const QString&, const QDateTime&)),
				this,
				SLOT (addBanListItem (const QString&, const QString&, const QDateTime&)));
		connect (ChannelEntry_,
				SIGNAL (gotExceptListItem (const QString&, const QString&, const QDateTime&)),
				this,
				SLOT (addExceptListItem (const QString&, const QString&, const QDateTime&)));
		connect (ChannelEntry_,
				SIGNAL (gotInviteListItem (const QString&, const QString&, const QDateTime&)),
				this,
				SLOT (addInviteListItem (const QString&, const QString&, const QDateTime&)));
	}

	void ChannelConfigWidget::SetModesUi ()
	{
		Ui_.OpTopic_->setChecked (ChannelMode_.OnlyOpChangeTopicMode_);
		Ui_.BlockOutMessage_->setChecked (ChannelMode_.BlockOutsideMessageMode_);
		Ui_.SecretChannel_->setChecked (ChannelMode_.SecretMode_);
		Ui_.PrivateChannel_->setChecked (ChannelMode_.PrivateMode_);
		Ui_.InvitesOnly_->setChecked (ChannelMode_.InviteMode_);
		Ui_.ModerateChannel_->setChecked (ChannelMode_.ModerateMode_);
		Ui_.ReOp_->setChecked (ChannelMode_.ReOpMode_);
		Ui_.UserLimit_->setChecked (ChannelMode_.UserLimit_.first);
		Ui_.Limit_->setValue (ChannelMode_.UserLimit_.second);
		Ui_.Password_->setChecked (ChannelMode_.ChannelKey_.first);
		Ui_.Key_->setText (ChannelMode_.ChannelKey_.second);
	}

	void ChannelConfigWidget::accept ()
	{
	}

	void ChannelConfigWidget::on_BanSearch__textChanged (const QString& text)
	{
		BanFilterModel_->setFilterRegExp (QRegExp(text, Qt::CaseInsensitive,
				QRegExp::FixedString));
		BanFilterModel_->setFilterKeyColumn (1);
	}

	void ChannelConfigWidget::on_ExceptSearch__textChanged (const QString& text)
	{
		ExceptFilterModel_->setFilterRegExp (QRegExp(text, Qt::CaseInsensitive,
				QRegExp::FixedString));
		ExceptFilterModel_->setFilterKeyColumn (1);
	}

	void ChannelConfigWidget::on_InviteSearch__textChanged (const QString& text)
	{
		InviteFilterModel_->setFilterRegExp (QRegExp(text, Qt::CaseInsensitive,
				QRegExp::FixedString));
		InviteFilterModel_->setFilterKeyColumn (1);
	}

	void ChannelConfigWidget::on_tabWidget_currentChanged (int index)
	{
		switch (index)
		{
		case 1:
			ChannelEntry_->RequestBanList ();
			IsWidgetRequest_ = true;
			break;
		case 2:
			ChannelEntry_->RequestExceptList ();
			IsWidgetRequest_ = true;
			break;
		case 3:
			ChannelEntry_->RequestInviteList ();
			IsWidgetRequest_ = true;
			break;
		default:
			IsWidgetRequest_ = false;
		}
		ChannelEntry_->SetIsWidgetRequest (IsWidgetRequest_);
	}

	void ChannelConfigWidget::addBanListItem (const QString& mask, 
			const QString& nick, const QDateTime& date)
	{
		QStandardItem *itemMask = new QStandardItem (mask);
		itemMask->setEditable (false);
		QStandardItem *itemNick = new QStandardItem (nick);
		itemNick->setEditable (false);
		QStandardItem *itemDate = new QStandardItem (date.toString ("dd.MM.yyyy hh:mm:ss"));
		itemDate->setEditable (false);

		BanModel_->appendRow (QList<QStandardItem*> () << itemMask
				<< itemNick
				<< itemDate);
	}

	void ChannelConfigWidget::addExceptListItem (const QString& mask, 
			const QString& nick, const QDateTime& date)
	{
		QStandardItem *itemMask = new QStandardItem (mask);
		itemMask->setEditable (false);
		QStandardItem *itemNick = new QStandardItem (nick);
		itemNick->setEditable (false);
		QStandardItem *itemDate = new QStandardItem (date.toString ("dd.MM.yyyy hh:mm:ss"));
		itemDate->setEditable (false);

		ExceptModel_->appendRow (QList<QStandardItem*> () << itemMask
				<< itemNick
				<< itemDate);
	}

	void ChannelConfigWidget::addInviteListItem (const QString& mask, 
			const QString& nick, const QDateTime& date)
	{
		QStandardItem *itemMask = new QStandardItem (mask);
		itemMask->setEditable (false);
		QStandardItem *itemNick = new QStandardItem (nick);
		itemNick->setEditable (false);
		QStandardItem *itemDate = new QStandardItem (date.toString ("dd.MM.yyyy hh:mm:ss"));
		itemDate->setEditable (false);

		InviteModel_->appendRow (QList<QStandardItem*> () << itemMask
				<< itemNick
				<< itemDate);
	}

}
}
}