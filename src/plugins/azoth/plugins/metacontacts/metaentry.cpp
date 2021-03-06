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

#include "metaentry.h"
#include <algorithm>
#include <QDateTime>
#include <QVariant>
#include <QImage>
#include <QAction>
#include <QtDebug>
#include <boost/concept_check.hpp>
#include <util/util.h>
#include "metaaccount.h"
#include "metamessage.h"
#include "managecontactsdialog.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	MetaEntry::MetaEntry (const QString& id, MetaAccount *account)
	: QObject (account)
	, Account_ (account)
	, ID_ (id)
	{
		ActionMCSep_ = Util::CreateSeparator (this);
		ActionManageContacts_ = new QAction (tr ("Manage contacts..."),
				this);
		connect (ActionManageContacts_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleManageContacts ()));
	}

	QObjectList MetaEntry::GetAvailEntryObjs () const
	{
		return AvailableRealEntries_;
	}

	QStringList MetaEntry::GetRealEntries () const
	{
		QStringList result = UnavailableRealEntries_;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
			result << qobject_cast<ICLEntry*> (entryObj)->GetEntryID ();
		return result;
	}

	void MetaEntry::SetRealEntries (const QStringList& ids)
	{
		UnavailableRealEntries_ = ids;
	}

	void MetaEntry::AddRealObject (ICLEntry *entry)
	{
		QObject *entryObj = entry->GetQObject ();

		AvailableRealEntries_ << entryObj;
		UnavailableRealEntries_.removeAll (entry->GetEntryID ());

		handleRealVariantsChanged (entry->Variants (), entryObj);
		Q_FOREACH (QObject *object, entry->GetAllMessages ())
			handleRealGotMessage (object);

		emit statusChanged (GetStatus (QString ()), QString ());

		ConnectStandardSignals (entryObj);
		if (qobject_cast<IAdvancedCLEntry*> (entryObj))
			ConnectAdvancedSiganls (entryObj);
	}

	QString MetaEntry::GetMetaVariant (QObject *entry, const QString& realVar) const
	{
		QPair<QObject*, QString> pair = qMakePair (entry, realVar);
		return Variant2RealVariant_.key (pair);
	}

	QObject* MetaEntry::GetQObject ()
	{
		return this;
	}

	QObject* MetaEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features MetaEntry::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsGrouping | FSupportsRenames;
	}

	ICLEntry::EntryType MetaEntry::GetEntryType () const
	{
		return EntryType::Chat;
	}

	QString MetaEntry::GetEntryName () const
	{
		return Name_;
	}

	void MetaEntry::SetEntryName (const QString& name)
	{
		Name_ = name;
		emit nameChanged (name);
	}

	QString MetaEntry::GetEntryID () const
	{
		return ID_;
	}

	QString MetaEntry::GetHumanReadableID () const
	{
		return GetEntryName () + "@metacontact";
	}

	QStringList MetaEntry::Groups () const
	{
		return Groups_;
	}

	void MetaEntry::SetGroups (const QStringList& groups)
	{
		Groups_ = groups;
		emit groupsChanged (groups);
	}

	QStringList MetaEntry::Variants () const
	{
		QStringList result;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

			const QString& name = entry->GetEntryName ();
			QStringList variants = entry->Variants ();
			if (!variants.contains (QString ()))
				variants.prepend (QString ());
			Q_FOREACH (const QString& var, variants)
			{
				const QString& full = name + '/' + var;
				if (!Variant2RealVariant_.contains (full))
				{
					qWarning () << Q_FUNC_INFO
							<< "skipping out-of-sync with variant map:"
							<< name
							<< var
							<< Variant2RealVariant_;
					continue;
				}

				result << full;
			}
		}
		return result;
	}

	QObject* MetaEntry::CreateMessage (IMessage::Type type, const QString& variant, const QString& body)
	{
		auto f = [type, body] (ICLEntry *e, const QString& v)
				{ return e->CreateMessage (type, v, body); };
		return ActWithVariant<QObject*, ICLEntry*> (f, variant);
	}

	QList<QObject*> MetaEntry::GetAllMessages () const
	{
		return Messages_;
	}

	void MetaEntry::PurgeMessages (const QDateTime& from)
	{
		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			qobject_cast<ICLEntry*> (obj)->PurgeMessages (from);
	}

	void MetaEntry::SetChatPartState (ChatPartState state, const QString& variant)
	{
		auto f = [state] (ICLEntry *e, const QString& v) { e->SetChatPartState (state, v); };
		ActWithVariant<void, ICLEntry*> (f, variant);
	}

	EntryStatus MetaEntry::GetStatus (const QString& variant) const
	{
		auto f = [] (ICLEntry *e, const QString& v) { return e->GetStatus (v); };
		return ActWithVariant<EntryStatus, ICLEntry*> (f, variant);
	}

	QImage MetaEntry::GetAvatar () const
	{
		return QImage ();
	}

	void MetaEntry::ShowInfo ()
	{
	}

	QList<QAction*> MetaEntry::GetActions () const
	{
		QList<QAction*> result;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
			result << qobject_cast<ICLEntry*> (entryObj)->GetActions ();

		if (!result.isEmpty ())
			result << ActionMCSep_;

		result << ActionManageContacts_;

		return result;
	}

	QMap<QString, QVariant> MetaEntry::GetClientInfo (const QString& variant) const
	{
		auto f = [] (ICLEntry *e, const QString& v) { return e->GetClientInfo (v); };
		return ActWithVariant<QMap<QString, QVariant>, ICLEntry*> (f, 	variant);
	}

	void MetaEntry::MarkMsgsRead ()
	{
	}

	void MetaEntry::ChatTabClosed ()
	{
	}

	IAdvancedCLEntry::AdvancedFeatures MetaEntry::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void MetaEntry::DrawAttention (const QString& text, const QString& variant)
	{
		auto f = [text] (IAdvancedCLEntry *e, const QString& v) { e->DrawAttention (text, v); };
		ActWithVariant<void, IAdvancedCLEntry*> (f, variant);
	}

	template<typename T, typename U>
	T MetaEntry::ActWithVariant (boost::function<T (U, const QString&)> func, const QString& variant) const
	{
		if (variant.isEmpty ())
		{
			if (AvailableRealEntries_.size ())
				return func (qobject_cast<U> (AvailableRealEntries_.first ()), QString ());
			else
				return T ();
		}

		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return T ();
		}

		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		return func (qobject_cast<U> (pair.first), pair.second);
	}

	void MetaEntry::ConnectStandardSignals (QObject *entryObj)
	{
		connect (entryObj,
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleRealGotMessage (QObject*)));
		connect (entryObj,
				SIGNAL (statusChanged (const EntryStatus&, const QString&)),
				this,
				SLOT (handleRealStatusChanged (const EntryStatus&, const QString&)));
		connect (entryObj,
				SIGNAL (availableVariantsChanged (const QStringList&)),
				this,
				SLOT (handleRealVariantsChanged (const QStringList&)));
		connect (entryObj,
				SIGNAL (nameChanged (const QString&)),
				this,
				SLOT (handleRealNameChanged (const QString&)));
		connect (entryObj,
				SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
				this,
				SLOT (handleRealCPSChanged (const ChatPartState&, const QString&)));
		connect (entryObj,
				SIGNAL (entryGenerallyChanged ()),
				this,
				SIGNAL (entryGenerallyChanged ()));

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		connect (entry->GetParentAccount (),
				SIGNAL (removedCLItems (QList<QObject*>)),
				this,
				SLOT (checkRemovedCLItems (QList<QObject*>)));

		/*
		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		connect (account->GetParentProtocol (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)),
				Qt::QueuedConnection);
				*/
	}

	void MetaEntry::ConnectAdvancedSiganls (QObject *entryObj)
	{
		connect (entryObj,
				SIGNAL (attentionDrawn (const QString&, const QString&)),
				this,
				SLOT (handleRealAttentionDrawn (const QString&, const QString&)));
		connect (entryObj,
				SIGNAL (moodChanged (const QString&)),
				this,
				SLOT (handleRealMoodChanged (const QString&)));
		connect (entryObj,
				SIGNAL (activityChanged (const QString&)),
				this,
				SLOT (handleRealActivityChanged (const QString&)));
		connect (entryObj,
				SIGNAL (tuneChanged (const QString&)),
				this,
				SLOT (handleRealTuneChanged (const QString&)));
		connect (entryObj,
				SIGNAL (locationChanged (const QString&)),
				this,
				SLOT (handleRealLocationChanged (const QString&)));
	}

	void MetaEntry::PerformRemoval (QObject *entryObj)
	{
		QObjectList::iterator i = Messages_.begin ();
		while (i < Messages_.end ())
		{
			MetaMessage *metaMsg = qobject_cast<MetaMessage*> (*i);
			IMessage *origMsg = metaMsg->GetOriginalMessage ();

			if (origMsg->OtherPart () == entryObj)
				i = Messages_.erase (i);
			else
				++i;
		}

		Q_FOREACH (const QString& var, Variant2RealVariant_.keys ())
		{
			const QPair<QObject*, QString>& pair = Variant2RealVariant_ [var];
			if (pair.first == entryObj)
			{
				Variant2RealVariant_.remove (var);
				emit statusChanged (EntryStatus (SOffline, QString ()), var);
			}
		}

		emit availableVariantsChanged (Variants ());
	}

	void MetaEntry::SetNewEntryList (const QList<QObject*>& newList, bool readdRemoved)
	{
		if (newList == AvailableRealEntries_)
			return;

		QList<QObject*> removedContacts;

		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			if (!newList.contains (obj))
				removedContacts << obj;

		AvailableRealEntries_ = newList;

		Q_FOREACH (QObject *entryObj, removedContacts)
			PerformRemoval (entryObj);

		Core::Instance ().HandleEntriesRemoved (removedContacts, readdRemoved);

		if (AvailableRealEntries_.isEmpty () &&
				UnavailableRealEntries_.isEmpty ())
		{
			emit shouldRemoveThis ();
			return;
		}

		emit availableVariantsChanged (Variants ());
		emit statusChanged (GetStatus (QString ()), QString ());
	}

	void MetaEntry::handleRealGotMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		MetaMessage *message = new MetaMessage (msgObj, this);

		const bool shouldSort = !Messages_.isEmpty () &&
				qobject_cast<IMessage*> (Messages_.last ())->GetDateTime () > msg->GetDateTime ();

		Messages_ << message;
		if (shouldSort)
			std::stable_sort (Messages_.begin (), Messages_.end (),
					[] (QObject *lObj, QObject *rObj)
					{
						return qobject_cast<IMessage*> (lObj)->GetDateTime () <
								qobject_cast<IMessage*> (rObj)->GetDateTime ();
					});

		emit gotMessage (message);
	}

	void MetaEntry::handleRealStatusChanged (const EntryStatus& status, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit statusChanged (status, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealVariantsChanged (QStringList variants, QObject *passedObj)
	{
		QObject *obj = passedObj ? passedObj : sender ();
		Q_FOREACH (const QString& var, Variant2RealVariant_.keys ())
		{
			const QPair<QObject*, QString>& pair = Variant2RealVariant_ [var];
			if (pair.first == obj)
				Variant2RealVariant_.remove (var);
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

		if (!variants.contains (QString ()))
			variants.prepend (QString ());

		Q_FOREACH (const QString& var, variants)
			Variant2RealVariant_ [entry->GetEntryName () + '/' + var] =
					qMakePair (obj, var);

		emit availableVariantsChanged (Variants ());

		Q_FOREACH (const QString& var, variants)
		{
			const QString& str = entry->GetEntryName () + '/' + var;
			emit statusChanged (GetStatus (str), str);
		}
	}

	void MetaEntry::handleRealNameChanged (const QString&)
	{
		QObject *obj = sender ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

		handleRealVariantsChanged (entry->Variants (), obj);
	}

	void MetaEntry::handleRealCPSChanged (const ChatPartState& cps, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit chatPartStateChanged (cps, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealAttentionDrawn (const QString& text, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit attentionDrawn (text, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealMoodChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit moodChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealActivityChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit activityChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealTuneChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit tuneChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealLocationChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit locationChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::checkRemovedCLItems (const QList<QObject*>& objs)
	{
		QList<QObject*> leave = AvailableRealEntries_;
		Q_FOREACH (QObject *obj, objs)
			leave.removeAll (obj);

		if (leave.size () != AvailableRealEntries_.size ())
			SetNewEntryList (leave, false);
	}

	void MetaEntry::handleManageContacts ()
	{
		ManageContactsDialog dia (AvailableRealEntries_);
		if (dia.exec () == QDialog::Rejected)
			return;

		SetNewEntryList (dia.GetObjects (), true);
	}
}
}
}
