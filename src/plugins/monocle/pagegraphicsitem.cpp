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

#include "pagegraphicsitem.h"
#include <limits>
#include <cmath>
#include <QtDebug>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QWidgetAction>
#include "core.h"
#include "pixmapcachemanager.h"
#include "arbitraryrotationwidget.h"
#include "pageslayoutmanager.h"

namespace LeechCraft
{
namespace Monocle
{
	PageGraphicsItem::PageGraphicsItem (IDocument_ptr doc, int page, QGraphicsItem *parent)
	: QGraphicsPixmapItem (parent)
	, Doc_ (doc)
	, PageNum_ (page)
	, XScale_ (1)
	, YScale_ (1)
	, Invalid_ (true)
	, LayoutManager_ (0)
	{
		setTransformationMode (Qt::SmoothTransformation);
		setPixmap (QPixmap (Doc_->GetPageSize (page)));
		setAcceptHoverEvents (true);
	}

	PageGraphicsItem::~PageGraphicsItem ()
	{
		Core::Instance ().GetPixmapCacheManager ()->PixmapDeleted (this);
	}

	void PageGraphicsItem::SetLayoutManager (PagesLayoutManager *manager)
	{
		LayoutManager_ = manager;
	}

	void PageGraphicsItem::SetReleaseHandler (std::function<void (int, QPointF)> handler)
	{
		ReleaseHandler_ = handler;
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		if (std::abs (xs - XScale_) < std::numeric_limits<double>::epsilon () &&
			std::abs (ys - YScale_) < std::numeric_limits<double>::epsilon ())
			return;

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

				watcher->setFuture (QtConcurrent::run ([this]
						{ return Doc_->RenderPage (PageNum_, XScale_, YScale_); }));

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
			Invalid_ = false;

			Core::Instance ().GetPixmapCacheManager ()->PixmapChanged (this);
		}

		QGraphicsPixmapItem::paint (painter, option, w);
		Core::Instance ().GetPixmapCacheManager ()->PixmapPainted (this);
	}

	void PageGraphicsItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		if (!ReleaseHandler_)
			QGraphicsItem::mousePressEvent (event);
	}

	void PageGraphicsItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		QGraphicsItem::mouseReleaseEvent (event);
		if (ReleaseHandler_)
			ReleaseHandler_ (PageNum_, event->pos ());
	}

	void PageGraphicsItem::contextMenuEvent (QGraphicsSceneContextMenuEvent *event)
	{
		QMenu rotateMenu;

		auto ccwAction = rotateMenu.addAction (tr ("Rotate 90 degrees counter-clockwise"),
				this, SLOT (rotateCCW ()));
		ccwAction->setProperty ("ActionIcon", "object-rotate-left");

		auto cwAction = rotateMenu.addAction (tr ("Rotate 90 degrees clockwise"),
				this, SLOT (rotateCW ()));
		cwAction->setProperty ("ActionIcon", "object-rotate-right");

		auto arbAction = rotateMenu.addAction (tr ("Rotate arbitrarily..."));
		arbAction->setProperty ("ActionIcon", "transform-rotate");

		auto arbMenu = new QMenu ();
		arbAction->setMenu (arbMenu);

		ArbWidget_ = new ArbitraryRotationWidget;
		ArbWidget_->setValue (LayoutManager_->GetRotation () +
				LayoutManager_->GetRotation (PageNum_));
		connect (ArbWidget_,
				SIGNAL (valueChanged (double)),
				this,
				SLOT (requestRotation (double)));
		connect (LayoutManager_,
				SIGNAL (rotationUpdated (double, int)),
				this,
				SLOT (updateRotation (double, int)));
		auto actionWidget = new QWidgetAction (arbMenu);
		actionWidget->setDefaultWidget (ArbWidget_);
		arbMenu->addAction (actionWidget);

		rotateMenu.exec (event->screenPos ());
	}

	bool PageGraphicsItem::IsDisplayed () const
	{
		const auto& thisMapped = mapToScene (boundingRect ()).boundingRect ();

		for (auto view : scene ()->views ())
		{
			const auto& rect = view->viewport ()->rect ();
			const auto& mapped = view->mapToScene (rect).boundingRect ();

			if (mapped.intersects (thisMapped))
				return true;
		}

		return false;
	}

	void PageGraphicsItem::rotateCCW ()
	{
		LayoutManager_->AddRotation (-90, PageNum_);
	}

	void PageGraphicsItem::rotateCW ()
	{
		LayoutManager_->AddRotation (90, PageNum_);
	}

	void PageGraphicsItem::requestRotation (double rotation)
	{
		LayoutManager_->SetRotation (rotation, PageNum_);
	}

	void PageGraphicsItem::updateRotation (double rotation, int page)
	{
		if (page != PageNum_)
			return;

		if (!ArbWidget_)
			return;

		ArbWidget_->setValue (rotation + LayoutManager_->GetRotation ());
	}

	void PageGraphicsItem::handlePixmapRendered ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QImage>*> (sender ());
		watcher->deleteLater ();

		const auto& img = watcher->result ();
		setPixmap (QPixmap::fromImage (img));

		Core::Instance ().GetPixmapCacheManager ()->PixmapChanged (this);
	}
}
}
