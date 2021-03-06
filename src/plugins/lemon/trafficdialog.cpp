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

#include "trafficdialog.h"
#include <QtDebug>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_dyngrid_layout.h>
#if QWT_VERSION >= 0x060100
#include <qwt_plot_legenditem.h>
#endif
#include <util/util.h>
#include "trafficmanager.h"

namespace LeechCraft
{
namespace Lemon
{
	TrafficDialog::TrafficDialog (const QString& name, TrafficManager *manager, QWidget *parent)
	: QDialog (parent)
	, Manager_ (manager)
	, IfaceName_ (name)
	, DownTraffic_ (new QwtPlotCurve (tr ("RX")))
	, UpTraffic_ (new QwtPlotCurve (tr ("TX")))
	, DownAvg_ (new QwtPlotCurve (tr ("Average RX")))
	, UpAvg_ (new QwtPlotCurve (tr ("Average TX")))
	{
		Ui_.setupUi (this);
		setWindowTitle (tr ("Traffic for %1").arg (name));

		Ui_.TrafficPlot_->setAutoReplot (false);
		Ui_.TrafficPlot_->setAxisScale (QwtPlot::xBottom, 0, manager->GetBacktrackSize ());
		Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Traffic, KiB/s"));

		QColor downColor (Qt::blue);
		DownTraffic_->setPen (QPen (downColor));
		downColor.setAlpha (20);
		DownTraffic_->setBrush (downColor);
		DownTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		DownTraffic_->attach (Ui_.TrafficPlot_);

		QColor upColor (Qt::red);
		UpTraffic_->setPen (QPen (upColor));
		upColor.setAlpha (20);
		UpTraffic_->setBrush (upColor);

		UpTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		UpTraffic_->attach (Ui_.TrafficPlot_);

		downColor.setAlpha (100);
		DownAvg_->setPen (QPen (downColor, 2, Qt::DotLine));
		DownAvg_->setBrush (Qt::transparent);
		DownAvg_->setRenderHint (QwtPlotItem::RenderAntialiased, false);
		DownAvg_->attach (Ui_.TrafficPlot_);

		upColor.setAlpha (100);
		UpAvg_->setPen (QPen (upColor, 2, Qt::DotLine));
		UpAvg_->setBrush (Qt::transparent);
		UpAvg_->setRenderHint (QwtPlotItem::RenderAntialiased, false);
		UpAvg_->attach (Ui_.TrafficPlot_);

		auto grid = new QwtPlotGrid;
		grid->enableYMin (true);
		grid->enableX (false);
#if QWT_VERSION >= 0x060100
		grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
#else
		grid->setMinPen (QPen (Qt::gray, 1, Qt::DashLine));
#endif
		grid->attach (Ui_.TrafficPlot_);

#if QWT_VERSION >= 0x060100
		auto item = new QwtPlotLegendItem;
		item->setMaxColumns (1);
		item->setAlignment (Qt::AlignTop | Qt::AlignLeft);
		item->attach (Ui_.TrafficPlot_);

		auto bgColor = palette ().color (QPalette::Button);
		bgColor.setAlphaF (0.8);
		item->setBackgroundBrush (bgColor);
		item->setBorderRadius (3);
		item->setBorderPen (QPen (palette ().color (QPalette::Dark), 1));
#else
		QwtLegend *legend = new QwtLegend;
		legend->setItemMode (QwtLegend::CheckableItem);
		Ui_.TrafficPlot_->insertLegend (legend, QwtPlot::ExternalLegend);

		auto layout = qobject_cast<QwtDynGridLayout*> (legend->contentsWidget ()->layout ());
		if (layout)
			layout->setMaxCols (1);
		else
			qWarning () << Q_FUNC_INFO
					<< "legend contents layout is not a QwtDynGridLayout:"
					<< legend->contentsWidget ()->layout ();

		Ui_.StatsFrame_->layout ()->addWidget (legend);
#endif

		connect (manager,
				SIGNAL (updated ()),
				this,
				SLOT (handleUpdated ()));
		handleUpdated ();
	}

	void TrafficDialog::handleUpdated ()
	{
		const auto& downList = Manager_->GetDownHistory (IfaceName_);
		const auto& upList = Manager_->GetUpHistory (IfaceName_);

		QVector<double> xdata (downList.size ());
		QVector<double> down (downList.size ());
		QVector<double> up (downList.size ());

		for (int i = 0; i < downList.size (); ++i)
		{
			xdata [i] = i;
			down [i] = downList [i] / 1024.;
			up [i] = upList [i] / 1024.;
		}

		DownTraffic_->setSamples (xdata, down);
		UpTraffic_->setSamples (xdata, up);

		if (!downList.isEmpty ())
		{
			Ui_.StatsFrame_->setVisible (true);

			Ui_.RXSpeed_->setText (Util::MakePrettySize (downList.last ()) + tr ("/s"));
			Ui_.TXSpeed_->setText (Util::MakePrettySize (upList.last ()) + tr ("/s"));

			const auto maxRx = *std::max_element (downList.begin (), downList.end ());
			const auto maxTx = *std::max_element (upList.begin (), upList.end ());
			Ui_.MaxRXSpeed_->setText (Util::MakePrettySize (maxRx) + tr ("/s"));
			Ui_.MaxTXSpeed_->setText (Util::MakePrettySize (maxTx) + tr ("/s"));

			auto avgList = [] (const QList<qint64>& list)
				{ return std::accumulate (list.begin (), list.end (), 0.0) / list.size (); };
			const auto avgRx = avgList (downList);
			const auto avgTx = avgList (upList);

			Ui_.AvgRXSpeed_->setText (Util::MakePrettySize (avgRx) + tr ("/s"));
			Ui_.AvgTXSpeed_->setText (Util::MakePrettySize (avgTx) + tr ("/s"));

			DownAvg_->setSamples (xdata, QVector<double> (downList.size (), avgRx / 1024));
			UpAvg_->setSamples (xdata, QVector<double> (downList.size (), avgTx / 1024));
		}
		else
			Ui_.StatsFrame_->setVisible (false);

		Ui_.TrafficPlot_->replot ();
	}

	void TrafficDialog::on_TrafficPlot__legendChecked (QwtPlotItem *item, bool on)
	{
		item->setVisible (!on);
		Ui_.TrafficPlot_->replot ();
	}
}
}
