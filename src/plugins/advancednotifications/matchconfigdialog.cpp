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

#include "matchconfigdialog.h"
#include <util/xpc/stdanfields.h>
#include "core.h"
#include "typedmatchers.h"
#include "fieldmatch.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	MatchConfigDialog::MatchConfigDialog (const QMap<QObject*, QList<ANFieldData>>& map,
			QWidget *parent)
	: QDialog (parent)
	, FieldsMap_ (map)
	{
		Ui_.setupUi (this);

		if (!FieldsMap_ [nullptr].isEmpty ())
			Ui_.SourcePlugin_->addItem (tr ("Standard fields"));

		for (auto i = FieldsMap_.begin (); i != FieldsMap_.end (); ++i)
		{
			if (!i.key ())
				continue;

			auto ii = qobject_cast<IInfo*> (i.key ());
			Ui_.SourcePlugin_->addItem (ii->GetIcon (),
					ii->GetName (), QVariant::fromValue (i.key ()));
		}

		if (Ui_.SourcePlugin_->count ())
			on_SourcePlugin__activated (0);
	}

	FieldMatch MatchConfigDialog::GetFieldMatch () const
	{
		const int fieldIdx = Ui_.FieldName_->currentIndex ();
		const int sourceIdx = Ui_.SourcePlugin_->currentIndex ();
		if (fieldIdx == -1 || sourceIdx == -1)
			return FieldMatch ();

		CurrentMatcher_->SyncToWidget ();

		const auto& data = Ui_.FieldName_->itemData (fieldIdx).value<ANFieldData> ();

		const auto plugin = Ui_.SourcePlugin_->itemData (sourceIdx).value<QObject*> ();

		FieldMatch result (data.Type_, CurrentMatcher_);
		if (plugin)
			result.SetPluginID (qobject_cast<IInfo*> (plugin)->GetUniqueID ());
		result.SetFieldName (data.ID_);

		return result;
	}

	void MatchConfigDialog::SetFieldMatch (const FieldMatch& match)
	{
		if (!match.GetMatcher ())
			qWarning () << Q_FUNC_INFO
					<< "no matcher for"
					<< match.GetPluginID ()
					<< match.GetFieldName ();

		const int fieldIdx = SelectPlugin (match.GetPluginID ().toLatin1 (), match.GetFieldName ());
		if (fieldIdx == -1)
			return;

		Ui_.FieldName_->setCurrentIndex (fieldIdx);
		on_FieldName__activated (fieldIdx);

		if (CurrentMatcher_)
		{
			CurrentMatcher_->SetValue (match.GetMatcher ()->GetValue ());
			CurrentMatcher_->SyncWidgetTo ();
		}
	}

	int MatchConfigDialog::SelectPlugin (const QByteArray& pluginId, const QString& fieldId)
	{
		int plugIdx = -1;
		if (!pluginId.isEmpty ())
			for (int i = 0; i < Ui_.SourcePlugin_->count (); ++i)
			{
				const auto plugin = Ui_.SourcePlugin_->itemData (i).value<QObject*> ();
				if (plugin && qobject_cast<IInfo*> (plugin)->GetUniqueID () == pluginId)
				{
					plugIdx = i;
					break;
				}
			}

		auto tryIdx = [this, &fieldId] (int idx) -> int
		{
			const auto pObj = Ui_.SourcePlugin_->itemData (idx).value<QObject*> ();
			const auto& fields = FieldsMap_ [pObj];

			for (int i = 0; i < fields.size (); ++i)
				if (fields.at (i).ID_ == fieldId)
				{
					Ui_.SourcePlugin_->setCurrentIndex (idx);
					on_SourcePlugin__activated (idx);
					return i;
				}

			return -1;
		};

		if (plugIdx != -1)
		{
			const auto idx = tryIdx (plugIdx);
			if (idx != -1)
				return idx;
		}

		return tryIdx (0);
	}

	void MatchConfigDialog::AddFields (const QList<ANFieldData>& fields)
	{
		for (const auto& data : fields)
			Ui_.FieldName_->addItem (data.Name_, QVariant::fromValue (data));
	}

	void MatchConfigDialog::on_SourcePlugin__activated (int idx)
	{
		Ui_.FieldName_->clear ();

		const auto pObj = Ui_.SourcePlugin_->itemData (idx).value<QObject*> ();
		AddFields (FieldsMap_ [pObj]);

		if (Ui_.FieldName_->count ())
			on_FieldName__activated (0);
	}

	void MatchConfigDialog::on_FieldName__activated (int idx)
	{
		const auto& data = Ui_.FieldName_->itemData (idx).value<ANFieldData> ();
		Ui_.DescriptionLabel_->setText (data.Description_);

		QLayout *lay = Ui_.ConfigWidget_->layout ();
		QLayoutItem *oldItem = 0;
		while ((oldItem = lay->takeAt (0)) != 0)
		{
			delete oldItem->widget ();
			delete oldItem;
		}

		CurrentMatcher_ = TypedMatcherBase::Create (data.Type_, data);
		if (CurrentMatcher_)
			lay->addWidget (CurrentMatcher_->GetConfigWidget ());
		else
			lay->addWidget (new QLabel (tr ("Invalid matcher type %1.")
						.arg (QVariant::typeToName (data.Type_))));
	}
}
}
