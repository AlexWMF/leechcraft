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

#include "accountslistwidget.h"
#include <QMenu>
#include <QWizard>
#include <QMessageBox>
#include <QComboBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iprotocol.h"
#ifdef ENABLE_CRYPT
#include "interfaces/azoth/isupportpgp.h"
#include "pgpkeyselectiondialog.h"
#include "cryptomanager.h"
#endif
#include "core.h"
#include "util.h"
#include "customchatstylemanager.h"
#include "chatstyleoptionmanager.h"

Q_DECLARE_METATYPE (LeechCraft::Azoth::ChatStyleOptionManager*)

namespace LeechCraft
{
namespace Azoth
{
	namespace
	{
		class AccountListDelegate : public QStyledItemDelegate
		{
		public:
			QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
			void setEditorData (QWidget*, const QModelIndex&) const;
			void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const;
		};

		QWidget* AccountListDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			auto getStyler = [&index] (AccountsListWidget::Role role)
				{ return index.data (role).value<ChatStyleOptionManager*> (); };

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::ChatStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetStyleModel ());
				connect (box,
						SIGNAL (currentIndexChanged (QString)),
						chatStyler,
						SLOT (handleChatStyleSelected (QString)));
				return box;
			}
			case AccountsListWidget::Column::ChatVariant:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::ChatStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetVariantModel ());
				return box;
			}
			case AccountsListWidget::Column::MUCStyle:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::MUCStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetStyleModel ());
				connect (box,
						SIGNAL (currentIndexChanged (QString)),
						chatStyler,
						SLOT (handleChatStyleSelected (QString)));
				return box;
			}
			case AccountsListWidget::Column::MUCVariant:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::MUCStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetVariantModel ());
				return box;
			}
			default:
				return QStyledItemDelegate::createEditor (parent, option, index);
			}
		}

		void AccountListDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
		{
			auto box = qobject_cast<QComboBox*> (editor);
			const auto& text = index.data ().toString ();

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			case AccountsListWidget::Column::ChatVariant:
			case AccountsListWidget::Column::MUCStyle:
			case AccountsListWidget::Column::MUCVariant:
				box->setCurrentIndex (box->findText (text));
				break;
			default:
				QStyledItemDelegate::setEditorData (editor, index);
				return;
			}
		}

		void AccountListDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
		{
			auto box = qobject_cast<QComboBox*> (editor);

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			case AccountsListWidget::Column::ChatVariant:
			case AccountsListWidget::Column::MUCStyle:
			case AccountsListWidget::Column::MUCVariant:
				model->setData (index, box->currentText ());
				break;
			default:
				QStyledItemDelegate::setModelData (editor, model, index);
				return;
			}
		}
	}

	AccountsListWidget::AccountsListWidget (QWidget* parent)
	: QWidget (parent)
	, AccModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.Accounts_->setItemDelegate (new AccountListDelegate);

		AccModel_->setHorizontalHeaderLabels ({ tr ("Show"), tr ("Name"),
				tr ("Chat style"), tr ("Variant"), tr ("MUC style"), tr ("MUC variant") });
		connect (AccModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));

#ifdef ENABLE_CRYPT
		Ui_.PGP_->setEnabled (true);
#endif

		const auto& fm = fontMetrics ();

		connect (&Core::Instance (),
				SIGNAL (accountAdded (IAccount*)),
				this,
				SLOT (addAccount (IAccount*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (IAccount*)),
				this,
				SLOT (handleAccountRemoved (IAccount*)));

		for (IAccount *acc : Core::Instance ().GetAccounts ())
			addAccount (acc);

		Ui_.Accounts_->setModel (AccModel_);

		Ui_.Accounts_->setColumnWidth (0, 32);
		Ui_.Accounts_->setColumnWidth (1, fm.width ("Some typical very long account name"));
		Ui_.Accounts_->setColumnWidth (2, fm.width ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (3, fm.width ("Some typical style variant (alternate)"));
		Ui_.Accounts_->setColumnWidth (4, fm.width ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (5, fm.width ("Some typical style variant (alternate)"));
	}

	void AccountsListWidget::addAccount (IAccount *acc)
	{
		auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		auto show = new QStandardItem ();
		show->setCheckable (true);
		show->setCheckState (acc->IsShownInRoster () ? Qt::Checked : Qt::Unchecked);
		show->setEditable (false);

		auto name = new QStandardItem (acc->GetAccountName ());
		name->setIcon (proto ? proto->GetProtocolIcon () : QIcon ());
		name->setEditable (false);

		const auto& stylePair = Core::Instance ()
				.GetCustomChatStyleManager ()->GetStyleForAccount (acc);
		auto style = new QStandardItem ();
		style->setText (stylePair.first);
		auto variant = new QStandardItem ();
		variant->setText (stylePair.second);

		const auto& mucPair = Core::Instance ()
				.GetCustomChatStyleManager ()->GetMUCStyleForAccount (acc);
		auto mucStyle = new QStandardItem ();
		mucStyle->setText (mucPair.first);
		auto mucVariant = new QStandardItem ();
		mucVariant->setText (mucPair.second);

		auto chatStyler = new ChatStyleOptionManager (QByteArray (), acc->GetQObject ());
		auto mucStyler = new ChatStyleOptionManager (QByteArray (), acc->GetQObject ());

		const QList<QStandardItem*> row { show, name, style, variant, mucStyle, mucVariant };
		for (auto item : row)
		{
			item->setData (QVariant::fromValue<IAccount*> (acc), Role::AccObj);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (chatStyler), Role::ChatStyleManager);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (mucStyler), Role::MUCStyleManager);
		}
		AccModel_->appendRow (row);

		for (auto item : { style, variant, mucStyle, mucVariant })
			Ui_.Accounts_->openPersistentEditor (item->index ());

		Account2Item_ [acc] = name;
	}

	void AccountsListWidget::on_Add__released ()
	{
		InitiateAccountAddition (this);
	}

	void AccountsListWidget::on_Modify__released ()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		index.data (Role::AccObj).value<IAccount*> ()->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_PGP__released ()
	{
#ifdef ENABLE_CRYPT
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		auto acc = index.data (Role::AccObj).value<IAccount*> ();
		auto pgpAcc = qobject_cast<ISupportPGP*> (acc->GetQObject ());
		if (!pgpAcc)
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("The account %1 doesn't support encryption.")
						.arg (acc->GetAccountName ()));
			return;
		}

		const QString& str = tr ("Please select new PGP key for the account %1.")
				.arg (acc->GetAccountName ());
		PGPKeySelectionDialog dia (str,
				PGPKeySelectionDialog::TPrivate, pgpAcc->GetPrivateKey (), this);
		if (dia.exec () != QDialog::Accepted)
			return;

		pgpAcc->SetPrivateKey (dia.GetSelectedKey ());
		CryptoManager::Instance ().AssociatePrivateKey (acc, dia.GetSelectedKey ());
#endif
	}

	void AccountsListWidget::on_Delete__released()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		auto acc = index.data (Role::AccObj).value<IAccount*> ();

		if (QMessageBox::question (this,
					"LeechCraft",
					tr ("Are you sure you want to remove the account %1?")
						.arg (acc->GetAccountName ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		auto protoObj = acc->GetParentProtocol ();
		auto proto = qobject_cast<IProtocol*> (protoObj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent protocol for"
					<< acc->GetAccountID ()
					<< "doesn't implement IProtocol";
			return;
		}
		proto->RemoveAccount (acc->GetQObject ());
	}

	void AccountsListWidget::on_ResetStyles__released ()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		const auto row = index.row ();
		for (auto col : { Column::ChatStyle, Column::ChatVariant, Column::MUCStyle, Column::MUCVariant })
			AccModel_->item (row, col)->setText ({});
	}

	void AccountsListWidget::handleItemChanged (QStandardItem *item)
	{
		auto acc = item->data (Role::AccObj).value<IAccount*> ();

		const auto& styleMgr = Core::Instance ().GetCustomChatStyleManager ();
		const auto& text = item->text ();

		switch (item->column ())
		{
		case Column::ShowInRoster:
			acc->SetShownInRoster (item->checkState () == Qt::Checked);

			if (!acc->IsShownInRoster () && acc->GetState ().State_ != SOffline)
				acc->ChangeState (EntryStatus (SOffline, QString ()));

			emit accountVisibilityChanged (acc);

			break;
		case Column::ChatStyle:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::ChatStyle, text);
			break;
		case Column::ChatVariant:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::ChatVariant, text);
			break;
		case Column::MUCStyle:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::MUCStyle, text);
			break;
		case Column::MUCVariant:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::MUCVariant, text);
			break;
		default:
			break;
		}
	}

	void AccountsListWidget::handleAccountRemoved (IAccount *acc)
	{
		if (!Account2Item_.contains (acc))
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetAccountName ()
					<< acc->GetQObject ()
					<< "from"
					<< sender ()
					<< "not found here";
			return;
		}

		AccModel_->removeRow (Account2Item_ [acc]->row ());
		Account2Item_.remove (acc);
	}
}
}
