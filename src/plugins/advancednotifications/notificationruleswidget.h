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
#include <QList>
#include "ui_notificationruleswidget.h"
#include "notificationrule.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
struct ANFieldData;

namespace AdvancedNotifications
{
	class RulesManager;

	class NotificationRulesWidget : public QWidget
	{
		Q_OBJECT

		Ui::NotificationRulesWidget Ui_;

		RulesManager *RM_;

		QMap<QString, QStringList> Cat2Types_;

		FieldMatches_t Matches_;
		QStandardItemModel *MatchesModel_;
	public:
		NotificationRulesWidget (RulesManager*, QWidget* = 0);
	private:
		void ResetMatchesModel ();

		QString GetCurrentCat () const;
		QStringList GetSelectedTypes () const;

		NotificationRule GetRuleFromUI () const;
		QList<QStandardItem*> MatchToRow (const FieldMatch&) const;

		QMap<QObject*, QList<ANFieldData>> GetRelevantANFieldsWPlugins () const;
		QList<ANFieldData> GetRelevantANFields () const;
		QString GetArgumentText ();
	private slots:
		void handleItemSelected (const QModelIndex&, const QModelIndex&);
		void selectRule (const QModelIndex&);

		void on_AddRule__released ();
		void on_UpdateRule__released ();
		void on_MoveRuleUp__released ();
		void on_MoveRuleDown__released ();
		void on_RemoveRule__released ();
		void on_DefaultRules__released ();

		void on_AddMatch__released ();
		void on_ModifyMatch__released ();
		void on_RemoveMatch__released ();

		void on_EventCat__currentIndexChanged (int);

		void on_NotifyVisual__stateChanged (int);
		void on_NotifySysTray__stateChanged (int);
		void on_NotifyAudio__stateChanged (int);
		void on_NotifyCmd__stateChanged (int);

		void on_BrowseAudioFile__released ();
		void on_TestAudio__released ();

		void on_AddArgument__released ();
		void on_ModifyArgument__released ();
		void on_RemoveArgument__released ();

		void resetAudioFileBox ();
	};
}
}
