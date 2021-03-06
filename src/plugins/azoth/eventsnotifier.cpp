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

#include "eventsnotifier.h"
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	EventsNotifier::EventsNotifier (QObject *parent)
	: QObject (parent)
	{
	}

	void EventsNotifier::RegisterEntry (ICLEntry *entry)
	{
		QObject *entryObj = entry->GetQObject ();

		connect (entryObj,
				SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
				this,
				SLOT (handleChatPartStateChanged (const ChatPartState&, const QString&)));
	}

	void EventsNotifier::handleChatPartStateChanged (const ChatPartState& state,
			const QString&)
	{
		if (state != CPSComposing)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLentry";
			return;
		}

		const QString& id = entry->GetEntryID ();
		if (!ShouldNotifyNext_.value (id, true))
			return;

		const QString& type = XmlSettingsManager::Instance ()
				.property ("NotifyIncomingComposing").toString ();
		if (type == "all" ||
			(type == "opened" &&
				Core::Instance ().GetChatTabsManager ()->IsOpenedChat (id)))
		{
			ShouldNotifyNext_ [id] = false;

			Entity e = Util::MakeNotification ("Azoth",
					tr ("%1 started composing a message to you.")
						.arg (entry->GetEntryName ()),
					PInfo_);
			e.Additional_ ["NotificationPixmap"] =
						QVariant::fromValue<QPixmap> (QPixmap::fromImage (entry->GetAvatar ()));
			Util::NotificationActionHandler *nh =
					new Util::NotificationActionHandler (e, this);
			nh->AddFunction (tr ("Open chat"),
					[entry] () { Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true); });
			nh->AddDependentObject (entry->GetQObject ());
			emit gotEntity (e);
		}
	}

	void EventsNotifier::handleEntryMadeCurrent (QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		ShouldNotifyNext_ [entry->GetEntryID ()] = true;
	}
}
}
