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

#include "viewgeometrymanager.h"
#include <QSettings>
#include <QToolBar>
#include <QApplication>
#include <QDeclarativeContext>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QtDebug>

#ifdef WITH_X11
#include <util/x11/xwrapper.h>
#endif

#include "viewmanager.h"
#include "sbview.h"

namespace LeechCraft
{
namespace SB2
{
	ViewGeometryManager::ViewGeometryManager (ViewManager *parent)
	: QObject (parent)
	, ViewMgr_ (parent)
	{
	}

	void ViewGeometryManager::Manage ()
	{
		auto settings = ViewMgr_->GetSettings ();
		settings->beginGroup ("Toolbars");
		const auto& posSettingName = "Pos_" + QString::number (ViewMgr_->GetWindowIndex ());
		auto pos = settings->value (posSettingName, static_cast<int> (Qt::LeftToolBarArea)).toInt ();
		settings->endGroup ();

		SetPosition (static_cast<Qt::ToolBarArea> (pos));
	}

	void ViewGeometryManager::SetPosition (Qt::ToolBarArea pos)
	{
		setOrientation ((pos == Qt::LeftToolBarArea || pos == Qt::RightToolBarArea) ? Qt::Vertical : Qt::Horizontal);

		auto toolbar = ViewMgr_->GetToolbar ();

		auto settings = ViewMgr_->GetSettings ();
		settings->beginGroup ("Toolbars");
		settings->setValue ("Pos_" + QString::number (ViewMgr_->GetWindowIndex ()), static_cast<int> (pos));
		settings->endGroup ();

#ifdef WITH_X11
		if (!ViewMgr_->IsDesktopMode ())
		{
#endif
			toolbar->setFloatable (false);
			ViewMgr_->GetManagedWindow ()->addToolBar (static_cast<Qt::ToolBarArea> (pos), toolbar);
#ifdef Q_OS_MAC
			// dunno WTF
			ViewMgr_->GetManagedWindow ()->show ();
#endif
#ifdef WITH_X11
		}
		else
		{
			toolbar->setParent (nullptr);
			toolbar->setWindowFlags (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

			toolbar->setFloatable (true);
			toolbar->setAllowedAreas (Qt::NoToolBarArea);

			toolbar->setAttribute (Qt::WA_X11NetWmWindowTypeDock);
			toolbar->setAttribute (Qt::WA_X11NetWmWindowTypeToolBar, false);
			toolbar->setAttribute (Qt::WA_AlwaysShowToolTips);

			toolbar->show ();

			updatePos ();

			Util::XWrapper::Instance ().MoveWindowToDesktop (toolbar->winId (), 0xffffffff);
		}
#endif
	}

	void ViewGeometryManager::updatePos ()
	{
#ifdef WITH_X11
		setOrientation (Qt::Horizontal);

		auto toolbar = ViewMgr_->GetToolbar ();

		const auto screenGeometry = QApplication::desktop ()->
				screenGeometry (ViewMgr_->GetManagedWindow ());

		const auto& minSize = toolbar->minimumSizeHint ();
		QRect rect { 0, 0, screenGeometry.width (), minSize.height () };
		rect.moveLeft (screenGeometry.left ());
		rect.moveBottom (screenGeometry.bottom ());

		toolbar->setGeometry (rect);
		ViewMgr_->GetView ()->setFixedSize (rect.size ());
		toolbar->setFixedSize (rect.size ());

		Util::XWrapper::Instance ().SetStrut (toolbar, Qt::BottomToolBarArea);
#endif
	}

	void ViewGeometryManager::setOrientation (Qt::Orientation orientation)
	{
		auto view = ViewMgr_->GetView ();
		const auto& size = view->sizeHint ();

		switch (orientation)
		{
		case Qt::Vertical:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
			view->rootContext ()->setContextProperty ("viewOrient", "vertical");
			break;
		case Qt::Horizontal:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			view->rootContext ()->setContextProperty ("viewOrient", "horizontal");
			break;
		}
	}
}
}
