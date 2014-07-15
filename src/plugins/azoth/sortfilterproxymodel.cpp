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

#include "sortfilterproxymodel.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucperms.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, ShowOffline_ (true)
	, MUCMode_ (false)
	, OrderByStatus_ (true)
	, HideMUCParts_ (false)
	, ShowSelfContacts_ (true)
	, MUCEntry_ (0)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);

		XmlSettingsManager::Instance ().RegisterObject ("OrderByStatus",
				this, "handleStatusOrderingChanged");
		handleStatusOrderingChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("HideMUCPartsInWholeCL",
				this, "handleHideMUCPartsChanged");
		handleHideMUCPartsChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("ShowSelfContacts",
				this, "handleShowSelfContactsChanged");
		handleShowSelfContactsChanged ();
	}

	void SortFilterProxyModel::SetMUCMode (bool muc)
	{
		MUCMode_ = muc;
		invalidate ();

		if (muc)
		  emit mucMode ();
	}

	bool SortFilterProxyModel::IsMUCMode () const
	{
		return MUCMode_;
	}

	void SortFilterProxyModel::SetMUC (QObject *mucEntry)
	{
		if (MUCEntry_)
			disconnect (MUCEntry_,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleMUCDestroyed ()));

		MUCEntry_ = qobject_cast<IMUCEntry*> (mucEntry) ? mucEntry : 0;
		if (MUCEntry_)
			connect (MUCEntry_,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleMUCDestroyed ()));

		invalidateFilter ();
	}

	void SortFilterProxyModel::showOfflineContacts (bool show)
	{
		ShowOffline_ = show;
		invalidate ();
	}

	void SortFilterProxyModel::handleStatusOrderingChanged ()
	{
		OrderByStatus_ = XmlSettingsManager::Instance ()
				.property ("OrderByStatus").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleHideMUCPartsChanged ()
	{
		HideMUCParts_ = XmlSettingsManager::Instance ()
				.property ("HideMUCPartsInWholeCL").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleShowSelfContactsChanged ()
	{
		ShowSelfContacts_ = XmlSettingsManager::Instance ()
				.property ("ShowSelfContacts").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleMUCDestroyed ()
	{
		SetMUC (0);
		SetMUCMode (false);
		emit wholeMode ();
	}

	namespace
	{
		Core::CLEntryType GetType (const QModelIndex& idx)
		{
			return idx.data (Core::CLREntryType).value<Core::CLEntryType> ();
		}

		ICLEntry* GetEntry (const QModelIndex& idx)
		{
			return qobject_cast<ICLEntry*> (idx
						.data (Core::CLREntryObject).value<QObject*> ());
		}
	}

	bool SortFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (MUCMode_)
		{
			if (!MUCEntry_)
				return false;

			const QModelIndex& idx = sourceModel ()->index (row, 0, parent);
			switch (GetType (idx))
			{
			case Core::CLETAccount:
			{
				QObject *acc = qobject_cast<ICLEntry*> (MUCEntry_)->GetParentAccount ();
				return acc == idx.data (Core::CLRAccountObject).value<QObject*> ();
			}
			case Core::CLETCategory:
			{
				const QString& gName = idx.data ().toString ();
				return gName == qobject_cast<IMUCEntry*> (MUCEntry_)->GetGroupName () ||
						qobject_cast<ICLEntry*> (MUCEntry_)->Groups ().contains (gName);
			}
			default:
				break;
			}
		}
		else
		{
			const QModelIndex& idx = sourceModel ()->index (row, 0, parent);
			if (!filterRegExp ().isEmpty ())
				return GetType (idx) == Core::CLETContact ?
						idx.data ().toString ().contains (filterRegExp ()) :
						true;

			if (idx.data (Core::CLRUnreadMsgCount).toInt ())
				return true;

			const auto type = GetType (idx);

			if (type == Core::CLETContact)
			{
				ICLEntry *entry = GetEntry (idx);
				const State state = entry->GetStatus ().State_;

				if (!ShowOffline_ &&
						state == SOffline &&
						!idx.data (Core::CLRUnreadMsgCount).toInt ())
					return false;

				if (HideMUCParts_ &&
						entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
					return false;

				if (!ShowSelfContacts_ &&
						entry->GetEntryFeatures () & ICLEntry::FSelfContact)
					return false;
			}
			else if (type == Core::CLETCategory)
			{
				if (!ShowOffline_ &&
						!idx.data (Core::CLRNumOnline).toInt ())
					return false;

				for (int subRow = 0; subRow < sourceModel ()->rowCount (idx); ++subRow)
					if (filterAcceptsRow (subRow, idx))
						return true;

				return false;
			}
			else if (type == Core::CLETAccount)
			{
				const auto& accObj = idx.data (Core::CLRAccountObject).value<QObject*> ();
				auto acc = qobject_cast<IAccount*> (accObj);
				return acc->IsShownInRoster ();
			}
		}

		return QSortFilterProxyModel::filterAcceptsRow (row, parent);
	}

	bool SortFilterProxyModel::lessThan (const QModelIndex& right,
			const QModelIndex& left) const			// sort in reverse order ok
	{
		const auto leftType = GetType (left);
		if (leftType == Core::CLETAccount)
			return QSortFilterProxyModel::lessThan (left, right);
		else if (leftType == Core::CLETCategory)
		{
			const bool leftIsMuc = left.data (Core::CLRIsMUCCategory).toBool ();
			const bool rightIsMuc = right.data (Core::CLRIsMUCCategory).toBool ();
			if ((leftIsMuc && rightIsMuc) || (!leftIsMuc && !rightIsMuc))
				return QSortFilterProxyModel::lessThan (left, right);
			else
				return rightIsMuc;
		}

		ICLEntry *lE = GetEntry (left);
		ICLEntry *rE = GetEntry (right);

		if (lE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				rE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				lE->GetParentCLEntry () == rE->GetParentCLEntry ())
			if (IMUCPerms *lp = qobject_cast<IMUCPerms*> (lE->GetParentCLEntry ()))
			{
				bool less = lp->IsLessByPerm (lE->GetQObject (), rE->GetQObject ());
				bool more = lp->IsLessByPerm (rE->GetQObject (), lE->GetQObject ());
				if (less || more)
					return more;
			}

		State lState = lE->GetStatus ().State_;
		State rState = rE->GetStatus ().State_;
		if (lState == rState ||
				!OrderByStatus_)
			return lE->GetEntryName ().localeAwareCompare (rE->GetEntryName ()) < 0;
		else
			return IsLess (lState, rState);
	}
}
}
