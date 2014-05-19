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

#include "eqbandwidget.h"
#include <cmath>
#include "bandinfo.h"

namespace LeechCraft
{
namespace LMP
{
namespace Fradj
{
	EqBandWidget::EqBandWidget (const BandInfo& info, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		Ui_.FreqBox_->setValue (info.Freq_);
		Ui_.FreqBox_->setSuffix (" " + tr ("Hz"));

		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SLOT (setGainSliderValue (double)));
		connect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));

		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
	}

	void EqBandWidget::SetGain (double value)
	{
		disconnect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
		Ui_.GainBox_->setValue (value);
		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
	}

	double EqBandWidget::GetGain ()
	{
		return Ui_.GainBox_->value ();
	}

	double EqBandWidget::GetFrequency () const
	{
		return Ui_.FreqBox_->value ();
	}

	void EqBandWidget::setGainSliderValue (double value)
	{
		disconnect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));

		Ui_.GainSlider_->setValue (std::round (value));

		connect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));
	}

	void EqBandWidget::setGainBoxValue (int value)
	{
		Ui_.GainBox_->setValue (value);
	}
}
}
}
