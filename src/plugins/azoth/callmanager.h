/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_CALLMANAGER_H
#define PLUGINS_AZOTH_CALLMANAGER_H
#include <QObject>
#include <QHash>
#include "interfaces/imediacall.h"
#include "interfaces/isupportmediacalls.h"

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	class CallManager : public QObject
	{
		Q_OBJECT
		
		QHash<QString, QObjectList> Entry2Calls_;
	public:
		CallManager (QObject* = 0);
		
		void AddAccount (QObject*);
		QObject* Call (ICLEntry*, const QString&);
		QObjectList GetCallsForEntry (const QString&) const;
	private:
		void HandleCall (QObject*);
	private slots:
		void handleIncomingCall (QObject*);
		void handleStateChanged (LeechCraft::Azoth::IMediaCall::State);
	signals:
		void gotCall (QObject*);
	};
}
}

#endif
