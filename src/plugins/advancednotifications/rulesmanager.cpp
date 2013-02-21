/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "rulesmanager.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/an/constants.h>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	QDataStream& operator<< (QDataStream& out, const NotificationRule& r)
	{
		r.Save (out);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, NotificationRule& r)
	{
		r.Load (in);
		return in;
	}

	namespace
	{
		class RulesModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				RuleName = Qt::UserRole + 1,
				IsRuleEnabled
			};

			RulesModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::RuleName] = "ruleName";
				roleNames [Roles::IsRuleEnabled] = "isRuleEnabled";
				setRoleNames (roleNames);
			}
		};
	}

	RulesManager::RulesManager (QObject *parent)
	: QObject (parent)
	, RulesModel_ (new RulesModel (this))
	{
		qRegisterMetaType<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaType<QList<NotificationRule>> ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");
		qRegisterMetaTypeStreamOperators<QList<NotificationRule>> ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");

		Cat2HR_ [AN::CatIM] = tr ("Instant messaging");
		Type2HR_ [AN::TypeIMAttention] = tr ("Attention request");
		Type2HR_ [AN::TypeIMIncFile] = tr ("Incoming file transfer request");
		Type2HR_ [AN::TypeIMIncMsg] = tr ("Incoming chat message");
		Type2HR_ [AN::TypeIMMUCHighlight] = tr ("MUC highlight");
		Type2HR_ [AN::TypeIMMUCInvite] = tr ("MUC invitation");
		Type2HR_ [AN::TypeIMMUCMsg] = tr ("General MUC message");
		Type2HR_ [AN::TypeIMStatusChange] = tr ("Contact status change");
		Type2HR_ [AN::TypeIMSubscrGrant] = tr ("Authorization granted");
		Type2HR_ [AN::TypeIMSubscrRevoke] = tr ("Authorization revoked");
		Type2HR_ [AN::TypeIMSubscrRequest] = tr ("Authorization requested");
		Type2HR_ [AN::TypeIMSubscrSub] = tr ("Contact subscribed");
		Type2HR_ [AN::TypeIMSubscrUnsub] = tr ("Contact unsubscribed");

		Cat2HR_ [AN::CatOrganizer] = tr ("Organizer");
		Type2HR_ [AN::TypeOrganizerEventDue] = tr ("Event is due");

		Cat2HR_ [AN::CatDownloads] = tr ("Downloads");
		Type2HR_ [AN::TypeDownloadError] = tr ("Download error");
		Type2HR_ [AN::TypeDownloadFinished] = tr ("Download finished");

		LoadSettings ();

		connect (RulesModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
	}

	QAbstractItemModel* RulesManager::GetRulesModel () const
	{
		return RulesModel_;
	}

	QList<NotificationRule> RulesManager::GetRulesList () const
	{
		return Rules_;
	}

	const QMap<QString, QString>& RulesManager::GetCategory2HR () const
	{
		return Cat2HR_;
	}

	const QMap<QString, QString>& RulesManager::GetType2HR () const
	{
		return Type2HR_;
	}

	void RulesManager::SetRuleEnabled (const NotificationRule& rule, bool enabled)
	{
		const int idx = Rules_.indexOf (rule);
		if (idx == -1)
			return;

		setRuleEnabled (idx, enabled);
	}

	void RulesManager::UpdateRule (const QModelIndex& index, const NotificationRule& rule)
	{
		if (rule.IsNull ())
			return;

		const int row = index.row ();
		Rules_ [row] = rule;
		int i = 0;
		for (QStandardItem *item : RuleToRow (rule))
			RulesModel_->setItem (row, i++, item);

		SaveSettings ();
	}

	void RulesManager::LoadDefaultRules (int version)
	{
		if (version <= 0)
		{
			NotificationRule chatMsg (tr ("Incoming chat messages"), AN::CatIM,
					QStringList (AN::TypeIMIncMsg));
			chatMsg.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			chatMsg.SetAudioParams (AudioParams ("im-incoming-message"));
			Rules_ << chatMsg;

			NotificationRule mucHigh (tr ("MUC highlights"), AN::CatIM,
					QStringList (AN::TypeIMMUCHighlight));
			mucHigh.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			mucHigh.SetAudioParams (AudioParams ("im-muc-highlight"));
			Rules_ << mucHigh;

			NotificationRule mucInv (tr ("MUC invitations"), AN::CatIM,
					QStringList (AN::TypeIMMUCInvite));
			mucInv.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			mucInv.SetAudioParams (AudioParams ("im-attention"));
			Rules_ << mucInv;

			NotificationRule incFile (tr ("Incoming file transfers"), AN::CatIM,
					QStringList (AN::TypeIMIncFile));
			incFile.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			Rules_ << incFile;

			NotificationRule subscrReq (tr ("Subscription requests"), AN::CatIM,
					QStringList (AN::TypeIMSubscrRequest));
			subscrReq.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			subscrReq.SetAudioParams (AudioParams ("im-auth-requested"));
			Rules_ << subscrReq;

			NotificationRule subscrChanges (tr ("Subscription changes"), AN::CatIM,
					QStringList (AN::TypeIMSubscrRevoke)
						<< AN::TypeIMSubscrGrant
						<< AN::TypeIMSubscrSub
						<< AN::TypeIMSubscrUnsub);
			subscrChanges.SetMethods (NMVisual | NMTray);
			Rules_ << subscrChanges;

			NotificationRule attentionDrawn (tr ("Attention requests"), AN::CatIM,
					QStringList (AN::TypeIMAttention));
			attentionDrawn.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint);
			attentionDrawn.SetAudioParams (AudioParams ("im-attention"));
			Rules_ << attentionDrawn;
		}

		if (version == -1 || version == 1)
		{
			NotificationRule eventDue (tr ("Event is due"), AN::CatOrganizer,
					QStringList (AN::TypeOrganizerEventDue));
			eventDue.SetMethods (NMVisual | NMTray | NMAudio);
			eventDue.SetAudioParams (AudioParams ("org-event-due"));
			Rules_ << eventDue;
		}

		if (version == -1 || version == 2)
		{
			NotificationRule downloadFinished (tr ("Download finished"), AN::CatDownloads,
					QStringList (AN::TypeDownloadFinished));
			downloadFinished.SetMethods (NMVisual | NMTray | NMAudio);
			Rules_ << downloadFinished;

			NotificationRule downloadError (tr ("Download error"), AN::CatDownloads,
					QStringList (AN::TypeDownloadError));
			downloadError.SetMethods (NMVisual | NMTray | NMAudio);
			downloadError.SetAudioParams (AudioParams ("error"));
			Rules_ << downloadError;
		}
	}

	void RulesManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		Rules_ = settings.value ("RulesList").value<QList<NotificationRule>> ();
		int rulesVersion = settings.value ("DefaultRulesVersion", 1).toInt ();

		const int currentDefVersion = 3;
		if (Rules_.isEmpty ())
			LoadDefaultRules (0);

		const bool shouldSave = rulesVersion < currentDefVersion;
		while (rulesVersion < currentDefVersion)
			LoadDefaultRules (rulesVersion++);
		if (shouldSave)
			SaveSettings ();

		settings.setValue ("DefaultRulesVersion", currentDefVersion);
		settings.endGroup ();

		ResetModel ();
	}

	void RulesManager::ResetModel ()
	{
		RulesModel_->clear ();
		RulesModel_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
				<< tr ("Category")
				<< tr ("Type"));

		for (const auto& rule : Rules_)
			RulesModel_->appendRow (RuleToRow (rule));
	}

	void RulesManager::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		settings.setValue ("RulesList", QVariant::fromValue<QList<NotificationRule>> (Rules_));
		settings.endGroup ();
	}

	QList<QStandardItem*> RulesManager::RuleToRow (const NotificationRule& rule) const
	{
		QStringList hrTypes;
		for (const QString& type : rule.GetTypes ())
			hrTypes << Type2HR_ [type];

		QList<QStandardItem*> items;
		items << new QStandardItem (rule.GetName ());
		items << new QStandardItem (Cat2HR_ [rule.GetCategory ()]);
		items << new QStandardItem (hrTypes.join ("; "));

		items.first ()->setCheckable (true);
		items.first ()->setCheckState (rule.IsEnabled () ? Qt::Checked : Qt::Unchecked);

		for (auto item : items)
		{
			item->setData (rule.GetName (), RulesModel::Roles::RuleName);
			item->setData (rule.IsEnabled (), RulesModel::Roles::IsRuleEnabled);
		}

		return items;
	}

	void RulesManager::prependRule ()
	{
		Rules_.prepend (NotificationRule ());
		RulesModel_->insertRow (0, RuleToRow (NotificationRule ()));
	}

	void RulesManager::removeRule (const QModelIndex& index)
	{
		RulesModel_->removeRow (index.row ());
		Rules_.removeAt (index.row ());

		SaveSettings ();
	}

	void RulesManager::moveUp (const QModelIndex& index)
	{
		const int row = index.row ();
		if (row < 1)
			return;

		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row, RulesModel_->takeRow (row - 1));

		SaveSettings ();
	}

	void RulesManager::moveDown (const QModelIndex& index)
	{
		const int row = index.row () + 1;
		if (row < 0 || row >= RulesModel_->rowCount ())
			return;

		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row - 1, RulesModel_->takeRow (row));

		SaveSettings ();
	}

	void RulesManager::setRuleEnabled (int idx, bool enabled)
	{
		Rules_ [idx].SetEnabled (enabled);
		if (auto item = RulesModel_->item (idx))
		{
			item->setData (enabled, RulesModel::Roles::IsRuleEnabled);
			item->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
		}
	}

	void RulesManager::reset ()
	{
		Rules_.clear ();
		RulesModel_->clear ();

		LoadDefaultRules ();
		ResetModel ();
		SaveSettings ();
	}

	QVariant RulesManager::getRulesModel () const
	{
		return QVariant::fromValue<QObject*> (RulesModel_);
	}

	void RulesManager::handleItemChanged (QStandardItem *item)
	{
		if (item->column ())
			return;

		const int idx = item->row ();
		const bool newState = item->checkState () == Qt::Checked;
		item->setData (newState, RulesModel::Roles::IsRuleEnabled);

		if (newState == Rules_.at (idx).IsEnabled () ||
				Rules_.at (idx).IsNull ())
			return;

		Rules_ [idx].SetEnabled (newState);

		SaveSettings ();
	}
}
}
