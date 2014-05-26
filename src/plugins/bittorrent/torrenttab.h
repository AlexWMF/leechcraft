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
#include "ui_torrenttab.h"

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	class TorrentTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::TorrentTab Ui_;

		const TabClassInfo TC_;
		QObject *ParentMT_;

		QToolBar *Toolbar_;
		QAction *OpenTorrent_;
		QAction *AddMagnet_;
		QAction *RemoveTorrent_;
		QAction *Resume_;
		QAction *Stop_;
		QAction *CreateTorrent_;
		QAction *MoveUp_;
		QAction *MoveDown_;
		QAction *MoveToTop_;
		QAction *MoveToBottom_;
		QAction *ForceReannounce_;
		QAction *ForceRecheck_;
		QAction *OpenMultipleTorrents_;
		QAction *IPFilter_;
		QAction *MoveFiles_;
		QAction *ChangeTrackers_;
		QAction *MakeMagnetLink_;

		QSortFilterProxyModel *ViewFilter_;
	public:
		TorrentTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();
	private:
		int GetCurrentTorrent () const;
		QList<int> GetSelectedRows () const;
		QModelIndexList GetSelectedRowIndexes () const;
	private slots:
		void handleTorrentSelected (const QModelIndex&);
		void setActionsEnabled ();

		void on_TorrentsView__customContextMenuRequested (const QPoint&);

		void handleOpenTorrentTriggered ();
		void handleAddMagnetTriggered ();
		void handleOpenMultipleTorrentsTriggered ();
		void handleIPFilterTriggered ();
		void handleCreateTorrentTriggered ();
		void handleRemoveTorrentTriggered ();
		void handleResumeTriggered ();
		void handleStopTriggered ();
		void handleMoveUpTriggered ();
		void handleMoveDownTriggered ();
		void handleMoveToTopTriggered ();
		void handleMoveToBottomTriggered ();
		void handleForceReannounceTriggered ();
		void handleForceRecheckTriggered ();
		void handleChangeTrackersTriggered ();
		void handleMoveFilesTriggered ();
		void handleMakeMagnetLinkTriggered ();
	signals:
		void removeTab (QWidget*);
	};
}
}
}
