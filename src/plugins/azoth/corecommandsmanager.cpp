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

#include "corecommandsmanager.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "proxyobject.h"
#include "chattabsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	namespace
	{
		bool HelpAll (ICLEntry *entry, const QString&)
		{
			QStringList commands;

			auto cmdProvs = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<IProvideCommands*> ();
			cmdProvs << Core::Instance ().GetCoreCommandsManager ();
			for (const auto prov : cmdProvs)
				for (const auto& cmd : prov->GetStaticCommands (entry))
				{
					auto cmdLine = cmd.Names_.join ("; ");
					if (!cmd.Description_.isEmpty ())
						cmdLine += QString::fromUtf8 (" — ") + cmd.Description_;
					commands << cmdLine;
				}

			auto body = CoreCommandsManager::tr ("The following commands are available:") +
					"<ul><li>" + commands.join ("</li><li>") + "</li></ul>";
			body += CoreCommandsManager::tr ("Type %1 in chat to get help on a particular command.")
					.arg ("<code>/help command</code>");

			const auto entryObj = entry->GetQObject ();
			const auto msgObj = ProxyObject {}.CreateCoreMessage (body,
					QDateTime::currentDateTime (),
					IMessage::Type::ServiceMessage,
					IMessage::Direction::In,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();

			return true;
		}

		bool HelpCommand (ICLEntry *entry, const StaticCommand& cmd)
		{
			auto message = CoreCommandsManager::tr ("Help on command %1:")
					.arg ("<code>" + cmd.Names_.first () + "</code>");

			if (!cmd.Description_.isEmpty ())
				message += "<br/>" + cmd.Description_;
			if (cmd.Names_.size () > 1)
				message += "<br/>" + CoreCommandsManager::tr ("Aliases: %1.")
						.arg (cmd.Names_.join ("; "));
			if (!cmd.Help_.isEmpty ())
				message += "<br/>" + cmd.Help_;

			const auto entryObj = entry->GetQObject ();
			const auto msgObj = ProxyObject {}.CreateCoreMessage (message,
					QDateTime::currentDateTime (),
					IMessage::Type::ServiceMessage,
					IMessage::Direction::In,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();

			return true;
		}

		bool HelpCommand (ICLEntry *entry, const QString& name)
		{
			auto cmdProvs = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<IProvideCommands*> ();
			cmdProvs << Core::Instance ().GetCoreCommandsManager ();
			for (const auto prov : cmdProvs)
				for (const auto& cmd : prov->GetStaticCommands (entry))
					if (cmd.Names_.contains (name) || cmd.Names_.contains ('/' + name))
					{
						HelpCommand (entry, cmd);
						return true;
					}

			const auto& body = CoreCommandsManager::tr ("Unknown command %1.")
					.arg ("<code>" + name + "</code>");

			const auto entryObj = entry->GetQObject ();
			const auto msgObj = ProxyObject {}.CreateCoreMessage (body,
					QDateTime::currentDateTime (),
					IMessage::Type::ServiceMessage,
					IMessage::Direction::In,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();

			return false;
		}

		bool Help (ICLEntry *entry, const QString& text)
		{
			const auto& commands = text.section (' ', 1).split (' ', QString::SkipEmptyParts);
			if (commands.isEmpty ())
				return HelpAll (entry, text);

			for (const auto& name : commands)
				HelpCommand (entry, name);

			return true;
		}

		bool Clear (ICLEntry *entry, const QString&)
		{
			const auto tab = Core::Instance ().GetChatTabsManager ()->
					GetChatTab (entry->GetEntryID ());
			if (tab)
				tab->clearChat ();

			return true;
		}
	}

	CoreCommandsManager::CoreCommandsManager (QObject* parent)
	: QObject { parent }
	, Help_
	{
		{ "/help" },
		&Help,
		tr ("Show the list of all commands or get help for a specific command."),
		tr ("Usage: ") + "<em>/help " + tr ("[command1] [command2] ...") + "</em><br/>" +
				tr ("Shows the list of all available commands with their respective short "
					"descriptions if called without parameters, otherwise shows help for "
					"the passed commands.") +
				"<br/><br/>" +
				tr ("Commands' variable inputs are typically shown in <em>italics</em>, "
					"while the parts that are to be entered as is are shown in "
					"<code>monospace</code> font. Optional arguments are typically in "
					"[square brackets], while different options are shown like this:")
					+ " &lt;A1|B2|C3>."
	}
	, Clear_
	{
		{ "/clear" },
		&Clear,
		tr ("Clear chat window."),
		tr ("Usage: ") + "<code>/clear</code>"
	}
	{
	}

	StaticCommands_t CoreCommandsManager::GetStaticCommands (ICLEntry*)
	{
		return { Help_, Clear_ };
	}
}
}
