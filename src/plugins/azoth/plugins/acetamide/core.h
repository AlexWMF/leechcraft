/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "socketmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcProtocol;
	class IrcAccount;
	class IrcClient;
	
	struct ServerOptions
	{
		QString NetworkName_;
		QString ServerName_;
		QString ServerEncoding_;
		QString ServerPassword_;
		QStringList ServerNicknames_;
		QString ServerRealName_;
		int ServerPort_;
		bool SSL_;
	};
	
	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;
		QString ChannelNickname_;
	};
	
	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		boost::shared_ptr<IrcProtocol> IrcProtocol_;
		boost::shared_ptr<SocketManager> SocketManager_;
		QObject *PluginProxy_;
		IrcAccount *DefaultAccount_;
		QString DefaultUserName_;
		Core ();
	public:
		static Core& Instance ();

		void SecondInit ();
		void Release ();
		QList<QObject*> GetProtocols () const;

		void SetPluginProxy (QObject*);
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		void SetDefaultIrcAcoount (IrcAccount *account);
		IrcAccount* GetDefaultIrcAccount ();
		void SendEntity (const Entity&);
		QString GetDefaultUserName () const;
		boost::shared_ptr<SocketManager> GetSocketManager () const;
	private:
		void CreateDefaultAccount ();
	private slots:
		void handleItemsAdded (const QList<QObject*>&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
