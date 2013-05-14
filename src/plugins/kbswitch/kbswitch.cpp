/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "kbswitch.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "keyboardlayoutswitcher.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace KBSwitch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"kbswitchsettings.xml");

		KBLayoutSwitcher_ = new KeyboardLayoutSwitcher (this);

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
		connect (rootWM->GetQObject (),
				SIGNAL (currentWindowChanged (int, int)),
				this,
				SLOT(handleCurrentWindowChanged (int, int)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.KBSwitch";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "KBSwitch";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides plugin- or tab-grained keyboard layout control.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/kbswitch/resources/images/kbswitch.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::handleCurrentChanged (int index)
	{
		if (KBLayoutSwitcher_->IsGlobalPolicy ())
			return;

		auto ictw = qobject_cast<ICoreTabWidget*> (sender ());
		QWidget *currentWidget = ictw->Widget (index);
		QWidget *prevWidget = ictw->GetPreviousWidget ();
		KBLayoutSwitcher_->updateKBLayouts (currentWidget, prevWidget);
	}

	void Plugin::handleCurrentWindowChanged (int from, int to)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();

		auto currentTW = rootWM->GetTabWidget (to);
		auto prevTW = rootWM->GetTabWidget (from);
		auto currentWidget = currentTW->Widget (currentTW->CurrentIndex ());
		auto prevWidget = prevTW->Widget (prevTW->CurrentIndex ());
		KBLayoutSwitcher_->updateKBLayouts (currentWidget, prevWidget);
	}

	void Plugin::handleWindow (int index)
	{
		auto tabWidget = Proxy_->GetRootWindowsManager ()->GetTabWidget (index);
		connect (tabWidget->GetQObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentChanged (int)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_kbswitch, LeechCraft::KBSwitch::Plugin);
