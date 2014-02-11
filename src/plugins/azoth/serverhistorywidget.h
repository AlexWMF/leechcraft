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

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "interfaces/azoth/ihaveserverhistory.h"
#include "ui_serverhistorywidget.h"

namespace LeechCraft
{
namespace Azoth
{
	class IHaveServerHistory;

	class ServerHistoryWidget : public QWidget
							  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		QObject *PluginObj_;
		TabClassInfo TC_;

		Ui::ServerHistoryWidget Ui_;

		QToolBar * const Toolbar_;

		QObject * const AccObj_;
		IHaveServerHistory * const IHSH_;

		int CurrentOffset_ = 0;
		int FirstMsgCount_ = -1;
	public:
		ServerHistoryWidget (QObject*, QWidget* = nullptr);

		void SetTabInfo (QObject*, const TabClassInfo&);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		int GetReqMsgCount () const;
	private slots:
		void handleFetched (const QModelIndex&, int, const SrvHistMessages_t&);
		void on_ContactsView__activated (const QModelIndex&);
		void navigatePrevious ();
		void navigateNext ();
	signals:
		void removeTab (QWidget*);
	};
}
}
