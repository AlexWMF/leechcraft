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

#include <QStringList>
#include <QAbstractItemModel>
#include <QList>
#include "message.h"

namespace LeechCraft
{
namespace Snails
{
	class MailModel : public QAbstractItemModel
	{
		const QStringList Headers_;

		QStringList Folder_;

		struct TreeNode;
		typedef std::shared_ptr<TreeNode> TreeNode_ptr;
		typedef std::weak_ptr<TreeNode> TreeNode_wptr;
		const TreeNode_ptr Root_;

		QList<Message_ptr> Messages_;
		QHash<QByteArray, QList<TreeNode_ptr>> FolderId2Nodes_;
		QHash<QByteArray, QByteArray> MsgId2FolderId_;
	public:
		enum class Column
		{
			From,
			UnreadChildren,
			AttachIcon,
			StatusIcon,
			Subject,
			Date,
			Size
		};

		enum MailRole
		{
			ID = Qt::UserRole + 1,
			Sort,
			IsRead,
			UnreadChildrenCount
		};

		MailModel (QObject* = 0);

		QVariant headerData (int, Qt::Orientation, int) const;
		int columnCount (const QModelIndex& = {}) const;
		QVariant data (const QModelIndex&, int) const;
		QModelIndex index (int, int, const QModelIndex& = {}) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = {}) const;

		void SetFolder (const QStringList&);
		QStringList GetCurrentFolder () const;

		Message_ptr GetMessage (const QByteArray&) const;

		void Clear ();

		void Append (QList<Message_ptr>);

		bool Update (const Message_ptr&);
		bool Remove (const QByteArray&);
	private:
		void UpdateParentReadCount (const QByteArray&, bool);

		void RemoveNode (const TreeNode_ptr&);
		bool AppendStructured (const Message_ptr&);

		QList<QModelIndex> GetIndexes (const QByteArray& folderId, int column) const;
		QList<QList<QModelIndex>> GetIndexes (const QByteArray& folderId, const QList<int>& columns) const;
		Message_ptr GetMessageByFolderId (const QByteArray&) const;
	};
}
}
