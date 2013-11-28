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

#pragma once

#include <QDeclarativeItem>

namespace LeechCraft
{
namespace Util
{
	class PlotItem : public QDeclarativeItem
	{
		Q_OBJECT

		Q_PROPERTY (QList<QPointF> points READ GetPoints WRITE SetPoints NOTIFY pointsChanged)
		Q_PROPERTY (QColor color READ GetColor WRITE SetColor NOTIFY colorChanged)
		Q_PROPERTY (bool leftAxisEnabled READ GetLeftAxisEnabled WRITE SetLeftAxisEnabled NOTIFY leftAxisEnabledChanged)
		Q_PROPERTY (bool bottomAxisEnabled READ GetBottomAxisEnabled WRITE SetBottomAxisEnabled NOTIFY bottomAxisEnabledChanged)
		Q_PROPERTY (QString leftAxisTitle READ GetLeftAxisTitle WRITE SetLeftAxisTitle NOTIFY leftAxisTitleChanged)
		Q_PROPERTY (QString bottomAxisTitle READ GetBottomAxisTitle WRITE SetBottomAxisTitle NOTIFY bottomAxisTitleChanged)

		QList<QPointF> Points_;
		QColor Color_;

		bool LeftAxisEnabled_ = false;
		bool BottomAxisEnabled_ = false;

		QString LeftAxisTitle_;
		QString BottomAxisTitle_;
	public:
		PlotItem (QDeclarativeItem* = 0);

		QList<QPointF> GetPoints () const;
		void SetPoints (const QList<QPointF>&);

		QColor GetColor () const;
		void SetColor (const QColor&);

		bool GetLeftAxisEnabled () const;
		void SetLeftAxisEnabled (bool);
		bool GetBottomAxisEnabled () const;
		void SetBottomAxisEnabled (bool);

		QString GetLeftAxisTitle () const;
		void SetLeftAxisTitle (const QString&);
		QString GetBottomAxisTitle () const;
		void SetBottomAxisTitle (const QString&);

		void paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	signals:
		void pointsChanged ();
		void colorChanged ();

		void leftAxisEnabledChanged ();
		void bottomAxisEnabledChanged ();

		void leftAxisTitleChanged ();
		void bottomAxisTitleChanged ();
	};
}
}