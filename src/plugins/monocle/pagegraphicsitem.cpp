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

#include "pagegraphicsitem.h"
#include <QtDebug>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "core.h"
#include "pixmapcachemanager.h"

namespace LeechCraft
{
namespace Monocle
{
	PageGraphicsItem::PageGraphicsItem (IDocument_ptr doc, int page, QGraphicsItem *parent)
	: QGraphicsPixmapItem (parent)
	, Doc_ (doc)
	, PageNum_ (page)
	, IsHoverLink_ (false)
	, Links_ (Doc_->GetPageLinks (PageNum_))
	, XScale_ (1)
	, YScale_ (1)
	, Invalid_ (true)
	{
		setTransformationMode (Qt::SmoothTransformation);
		setPixmap (QPixmap (Doc_->GetPageSize (page)));

		if (!Links_.isEmpty ())
			setAcceptHoverEvents (true);
	}

	PageGraphicsItem::~PageGraphicsItem ()
	{
		Core::Instance ().GetPixmapCacheManager ()->PixmapDeleted (this);
	}

	void PageGraphicsItem::SetReleaseHandler (std::function<void (int, QPointF)> handler)
	{
		ReleaseHandler_ = handler;
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		XScale_ = xs;
		YScale_ = ys;

		auto size = Doc_->GetPageSize (PageNum_);
		size.rwidth () *= xs;
		size.rheight () *= ys;
		setPixmap (QPixmap (size));

		Invalid_ = true;

		if (IsDisplayed ())
			update ();

		for (auto i = Item2RectInfo_.begin (); i != Item2RectInfo_.end (); ++i)
		{
			const auto& info = *i;
			info.Setter_ (MapFromDoc (info.DocRect_));
		}
	}

	int PageGraphicsItem::GetPageNum () const
	{
		return PageNum_;
	}

	QRectF PageGraphicsItem::MapFromDoc (const QRectF& rect) const
	{
		return
		{
			rect.x () * XScale_,
			rect.y () * YScale_,
			rect.width () * XScale_,
			rect.height () * YScale_
		};
	}

	QRectF PageGraphicsItem::MapToDoc (const QRectF& rect) const
	{
		return
		{
			rect.x () / XScale_,
			rect.y () / YScale_,
			rect.width () / XScale_,
			rect.height () / YScale_
		};
	}

	void PageGraphicsItem::RegisterChildRect (QGraphicsItem *item,
			const QRectF& docRect, RectSetter_f setter)
	{
		Item2RectInfo_ [item] = { docRect, setter };
		setter (MapFromDoc (docRect));
	}

	void PageGraphicsItem::UnregisterChildRect (QGraphicsItem *item)
	{
		Item2RectInfo_.remove (item);
	}

	void PageGraphicsItem::ClearPixmap ()
	{
		auto size = Doc_->GetPageSize (PageNum_);
		size.rwidth () *= XScale_;
		size.rheight () *= YScale_;
		setPixmap (QPixmap (size));

		Invalid_ = true;
	}

	void PageGraphicsItem::UpdatePixmap ()
	{
		Invalid_ = true;
		if (IsDisplayed ())
			update ();
	}

	void PageGraphicsItem::paint (QPainter *painter,
			const QStyleOptionGraphicsItem *option, QWidget *w)
	{
		if (Invalid_ && IsDisplayed ())
		{
			auto backendObj = Doc_->GetBackendPlugin ();
			if (qobject_cast<IBackendPlugin*> (backendObj)->IsThreaded ())
			{
				auto watcher = new QFutureWatcher<QImage> ();
				connect (watcher,
						SIGNAL (finished ()),
						this,
						SLOT (handlePixmapRendered ()));

				std::function<QImage ()> worker ([this] ()
						{
							return Doc_->RenderPage (PageNum_, XScale_, YScale_);
						});
				watcher->setFuture (QtConcurrent::run (worker));

				auto size = Doc_->GetPageSize (PageNum_);
				size.rwidth () *= XScale_;
				size.rheight () *= YScale_;
				QPixmap px (size);
				px.fill ();
				setPixmap (px);
			}
			else
			{
				const auto& img = Doc_->RenderPage (PageNum_, XScale_, YScale_);
				setPixmap (QPixmap::fromImage (img));
			}
			LayoutLinks ();
			Invalid_ = false;

			Core::Instance ().GetPixmapCacheManager ()->PixmapChanged (this);
		}

		QGraphicsPixmapItem::paint (painter, option, w);
		Core::Instance ().GetPixmapCacheManager ()->PixmapPainted (this);
	}

	void PageGraphicsItem::hoverMoveEvent (QGraphicsSceneHoverEvent *event)
	{
		if (!IsHoverLink_ && FindLink (event->pos ()))
		{
			QApplication::setOverrideCursor (QCursor (Qt::PointingHandCursor));
			IsHoverLink_ = true;
		}
		else if (IsHoverLink_ && !FindLink (event->pos ()))
		{
			QApplication::restoreOverrideCursor ();
			IsHoverLink_ = false;
		}

		QGraphicsItem::hoverMoveEvent (event);
	}

	void PageGraphicsItem::hoverLeaveEvent (QGraphicsSceneHoverEvent *event)
	{
		if (IsHoverLink_)
			QApplication::restoreOverrideCursor ();

		QGraphicsItem::hoverLeaveEvent (event);
	}

	void PageGraphicsItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		PressedLink_ = FindLink (event->pos ());
		if (PressedLink_ || ReleaseHandler_)
			return;

		QGraphicsItem::mousePressEvent (event);
	}

	void PageGraphicsItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		auto relLink = FindLink (event->pos ());
		const bool handle = relLink && relLink == PressedLink_;
		PressedLink_ = ILink_ptr ();
		if (!handle)
		{
			QGraphicsItem::mouseReleaseEvent (event);
			if (ReleaseHandler_)
				ReleaseHandler_ (PageNum_, event->pos ());
			return;
		}

		relLink->Execute ();
	}

	void PageGraphicsItem::LayoutLinks ()
	{
		Rect2Link_.clear ();

		const auto& rect = boundingRect ();
		const auto width = rect.width ();
		const auto height = rect.height ();
		for (auto link : Links_)
		{
			const auto& area = link->GetArea ();
			const QRect linkRect (width * area.left (), height * area.top (),
					width * area.width (), height * area.height ());
			Rect2Link_ << qMakePair (linkRect, link);
		}
	}

	ILink_ptr PageGraphicsItem::FindLink (const QPointF& point)
	{
		for (const auto& pair : Rect2Link_)
			if (pair.first.contains (point.toPoint ()))
				return pair.second;
		return ILink_ptr ();
	}

	bool PageGraphicsItem::IsDisplayed () const
	{
		for (auto view : scene ()->views ())
		{
			const auto& items = view->items (view->viewport ()->rect ());
			if (std::find (items.begin (), items.end (), this) != items.end ())
				return true;
		}

		return false;
	}

	void PageGraphicsItem::handlePixmapRendered ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QImage>*> (sender ());
		watcher->deleteLater ();

		const auto& img = watcher->result ();
		setPixmap (QPixmap::fromImage (img));
	}
}
}
