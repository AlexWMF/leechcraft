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

#include "progressmanager.h"
#include <QStandardItemModel>
#include <interfaces/ijobholder.h>
#include "cuesplitter.h"

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setColumnCount (3);
	}

	QAbstractItemModel* ProgressManager::GetModel () const
	{
		return Model_;
	}

	void ProgressManager::handleTagsFetch (int fetched, int total, QObject *obj)
	{
		if (!TagsFetchObj2Row_.contains (obj))
		{
			auto nameItem = new QStandardItem (tr ("Fetching tags..."));
			auto statusItem = new QStandardItem (tr ("Fetching..."));
			auto progressItem = new QStandardItem ();

			const QList<QStandardItem*> row
			{
				nameItem,
				statusItem,
				progressItem
			};
			auto item = row.at (JobHolderColumn::JobProgress);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);

			TagsFetchObj2Row_ [obj] = row;
			Model_->appendRow (row);
		}

		if (fetched == total)
		{
			const auto& list = TagsFetchObj2Row_.take (obj);
			Model_->removeRow (list.first ()->row ());
			return;
		}

		const auto& list = TagsFetchObj2Row_ [obj];

		auto item = list.at (JobHolderColumn::JobProgress);
		item->setText (tr ("%1 of %2").arg (fetched).arg (total));
		item->setData (fetched, ProcessState::Done);
		item->setData (total, ProcessState::Total);
	}

	void ProgressManager::handleCueSplitter (CueSplitter *splitter)
	{
		const QList<QStandardItem*> row
		{
			new QStandardItem (tr ("Splitting CUE %1...").arg (splitter->GetCueFile ())),
			new QStandardItem (tr ("Splitting...")),
			new QStandardItem ()
		};

		auto item = row.at (JobHolderColumn::JobProgress);
		item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
				CustomDataRoles::RoleJobHolderRow);

		Splitter2Row_ [splitter] = row;
		Model_->appendRow (row);

		connect (splitter,
				SIGNAL (splitProgress (int, int, CueSplitter*)),
				this,
				SLOT (handleSplitProgress (int, int, CueSplitter*)));
		connect (splitter,
				SIGNAL (finished (CueSplitter*)),
				this,
				SLOT (handleSplitFinished (CueSplitter*)));
	}

	void ProgressManager::handleSplitProgress (int done, int total, CueSplitter *splitter)
	{
		if (!Splitter2Row_.contains (splitter))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown splitter";
			return;
		}

		if (done == total)
			return;

		const auto& list = Splitter2Row_ [splitter];

		auto item = list.at (JobHolderColumn::JobProgress);
		item->setText (tr ("%1 of %2").arg (done).arg (total));
		item->setData (done, ProcessState::Done);
		item->setData (total, ProcessState::Total);
	}

	void ProgressManager::handleSplitFinished (CueSplitter *splitter)
	{
		if (!Splitter2Row_.contains (splitter))
			return;

		Model_->removeRow (Splitter2Row_.take (splitter).first ()->row ());
	}
}
}
}
