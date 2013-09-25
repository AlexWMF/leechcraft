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

#include "groupsmanager.h"
#include <QTimer>
#include "vkconnection.h"
#include "structures.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	GroupsManager::GroupsManager (VkConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
	{
		connect (Conn_,
				SIGNAL (gotLists (QList<ListInfo>)),
				this,
				SLOT (handleLists (QList<ListInfo>)));
		connect (Conn_,
				SIGNAL (addedLists (QList<ListInfo>)),
				this,
				SLOT (handleAddedLists (QList<ListInfo>)));
		connect (Conn_,
				SIGNAL (gotUsers (QList<UserInfo>)),
				this,
				SLOT (handleUsers (QList<UserInfo>)));
	}

	ListInfo GroupsManager::GetListInfo (qulonglong id) const
	{
		return ID2ListInfo_ [id];
	}

	ListInfo GroupsManager::GetListInfo (const QString& name) const
	{
		const auto pos = std::find_if (ID2ListInfo_.begin (), ID2ListInfo_.end (),
				[&name] (const ListInfo& li) { return li.Name_ == name; });
		return pos == ID2ListInfo_.end () ? ListInfo () : *pos;
	}

	void GroupsManager::UpdateGroups (const QStringList& oldGroups,
			const QStringList& newGroups, qulonglong id)
	{
		for (const auto& newItem : newGroups)
		{
			if (oldGroups.contains (newItem))
				continue;

			const auto listPos = std::find_if (ID2ListInfo_.begin (), ID2ListInfo_.end (),
					[&newItem] (const ListInfo& li) { return li.Name_ == newItem; });
			if (listPos != ID2ListInfo_.end ())
			{
				const auto list = listPos->ID_;
				List2IDs_ [list] << id;

				ModifiedLists_ << list;
			}
			else
				NewLists_ [newItem] << id;
		}

		for (const auto& oldItem : oldGroups)
			if (!newGroups.contains (oldItem))
			{
				auto list = GetListInfo (oldItem).ID_;
				List2IDs_ [list].remove (id);

				ModifiedLists_ << list;
			}

		if (!IsApplyScheduled_)
		{
			IsApplyScheduled_ = true;
			QTimer::singleShot (1000,
					this,
					SLOT (applyChanges ()));
		}
	}

	void GroupsManager::applyChanges ()
	{
		for (auto list : ModifiedLists_)
			Conn_->ModifyFriendList (GetListInfo (list), List2IDs_ [list].toList ());

		for (auto i = NewLists_.begin (), end = NewLists_.end (); i != end; ++i)
			Conn_->AddFriendList (i.key (), i.value ().toList ());

		ModifiedLists_.clear ();

		IsApplyScheduled_ = false;
	}

	void GroupsManager::handleLists (const QList<ListInfo>& lists)
	{
		ID2ListInfo_.clear ();
		for (const auto& list : lists)
			ID2ListInfo_ [list.ID_] = list;
	}

	void GroupsManager::handleAddedLists (const QList<ListInfo>& lists)
	{
		for (const auto& list : lists)
		{
			ID2ListInfo_ [list.ID_] = list;

			List2IDs_ [list.ID_] += NewLists_.take (list.Name_);
		}
	}

	void GroupsManager::handleUsers (const QList<UserInfo>& infos)
	{
		for (const auto& info : infos)
			for (const auto& listId : info.Lists_)
				List2IDs_ [listId] << info.ID_;
	}
}
}
}