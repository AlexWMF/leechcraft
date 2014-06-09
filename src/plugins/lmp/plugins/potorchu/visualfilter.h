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

#include <memory>
#include <QObject>
#include <interfaces/lmp/ifilterelement.h>
#include <interfaces/lmp/ilmpproxy.h>

typedef struct _GstPad GstPad;
typedef struct _GstBuffer GstBuffer;

class projectM;

namespace LeechCraft
{
namespace LMP
{
namespace Potorchu
{
	class VisWidget;
	class VisScene;

	class VisualFilter : public QObject
					   , public IFilterElement
	{
		Q_OBJECT

		const QByteArray EffectId_;
		const ILMPProxy_ptr LmpProxy_;

		const std::shared_ptr<VisWidget> Widget_;
		const std::shared_ptr<VisScene> Scene_;

		GstElement * const Elem_;
		GstElement * const Tee_;
		GstElement * const AudioQueue_;
		GstElement * const ProbeQueue_;
		GstElement * const Converter_;
		GstElement * const FakeSink_;
		IPath *Path_;

		std::shared_ptr<projectM> ProjectM_;
	public:
		VisualFilter (const QByteArray&, const ILMPProxy_ptr&);

		QByteArray GetEffectId () const;
		QByteArray GetInstanceId () const;
		IFilterConfigurator* GetConfigurator () const;
	protected:
		GstElement* GetElement () const;
	private:
		void InitProjectM ();

		void HandleBuffer (GstBuffer*);
	private slots:
		void handleSceneRectChanged (const QRectF&);

		void updateFrame ();

		void handlePrevVis ();
		void handleNextVis ();
	};
}
}
}
