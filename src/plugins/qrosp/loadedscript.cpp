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

#include "loadedscript.h"
#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif
#include <QtDebug>
#include <qross/core/action.h>
#include <qross/core/script.h>

namespace LeechCraft
{
namespace Qrosp
{
	LoadedScript::LoadedScript (const QString& path,
			const QString& interp, QObject *parent)
	: QObject (parent)
	, ScriptAction_ (new Qross::Action (this, QUrl::fromLocalFile (path)))
	{
		if (!interp.isEmpty ())
			ScriptAction_->setInterpreter (interp);

		ScriptAction_->trigger ();
#ifndef QROSP_NO_QTSCRIPT
		if (interp == "qtscript")
		{
			QObject *scriptEngineObject = 0;
			QMetaObject::invokeMethod (ScriptAction_->script (),
					"engine", Q_RETURN_ARG (QObject*, scriptEngineObject));
			QScriptEngine *engine = qobject_cast<QScriptEngine*> (scriptEngineObject);
			if (!engine)
				qWarning () << Q_FUNC_INFO
						<< "unable to obtain script engine from the"
						<< "script action though we are Qt Script";
			else
				for (const auto& req : { "qt", "qt.core", "qt.gui", "qt.network" })
					engine->importExtension (req);
		}
#endif
	}

	QObject* LoadedScript::GetQObject ()
	{
		return this;
	}
	
	QVariant LoadedScript::InvokeMethod (const QString& name, const QVariantList& args)
	{
		if (!ScriptAction_->functionNames ().contains (name))
			return QVariant ();

		return ScriptAction_->callFunction (name, args);
	}

	void LoadedScript::AddQObject (QObject *object, const QString& name)
	{
		ScriptAction_->addQObject (object, name);
	}
}
}
