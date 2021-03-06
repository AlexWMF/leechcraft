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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/ihavetabs.h>
#include "ui_mailtab.h"
#include "account.h"

class QStandardItemModel;
class QStandardItem;
class QSortFilterProxyModel;
class QToolButton;

namespace LeechCraft
{
namespace Snails
{
	class MailTab : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::MailTab Ui_;

		const ICoreProxy_ptr Proxy_;

		QToolBar * const TabToolbar_;
		QToolBar * const MsgToolbar_;

		QMenu *MsgCopy_;
		QMenu *MsgMove_;

		QMenu *MsgAttachments_;
		QToolButton *MsgAttachmentsButton_;

		TabClassInfo TabClass_;
		QObject *PMT_;

		std::shared_ptr<MailModel> MailModel_;
		QSortFilterProxyModel *MailSortFilterModel_;
		Account_ptr CurrAcc_;
		Message_ptr CurrMsg_;
	public:
		MailTab (const ICoreProxy_ptr&, const TabClassInfo&, QObject*, QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void FillCommonActions ();
		void FillMailActions ();
		void FillTabToolbarActions ();
		QList<QByteArray> GetSelectedIds () const;

		void SetMsgActionsEnabled (bool);
		QList<Folder> GetActualFolders () const;
	private slots:
		void handleCurrentAccountChanged (const QModelIndex&);
		void handleCurrentTagChanged (const QModelIndex&);
		void handleMailSelected ();

		void rebuildOpsToFolders ();

		void handleReply ();
		void handleCopyMultipleFolders ();
		void handleCopyMessages (QAction*);
		void handleMoveMultipleFolders ();
		void handleMoveMessages (QAction*);
		void handleMarkMsgUnread ();
		void handleRemoveMsgs ();
		void handleViewHeaders ();

		void handleAttachment ();
		void handleAttachment (const QByteArray&, const QStringList&, const QString&);
		void handleFetchNewMail ();
		void handleRefreshFolder ();

		void handleMessageBodyFetched (Message_ptr);
	signals:
		void removeTab (QWidget*);

		void mailActionsEnabledChanged (bool);
	};
}
}
