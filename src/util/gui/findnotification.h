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

#include <interfaces/core/icoreproxy.h>
#include "guiconfig.h"
#include "pagenotification.h"

namespace Ui
{
	class FindNotification;
}

namespace LeechCraft
{
namespace Util
{
	/** @brief A horizontal bar with typical widgets for text search.
	 *
	 * This widget provides typical features for text searching: a text
	 * input field, checkboxes for selecting find mode and buttons for
	 * searching and closing the notification, as well as convenience
	 * slots findNext() and findPrevious().
	 *
	 * The widget will automatically be embedded into the layout of
	 * the parent widget of \em near after the \em near widget (which is
	 * passed to the constructor).
	 *
	 * This class is typically used as following:
	 * -# It's subclassed, and an implementation of handleNext() function
	 *    is provided, which deals with the search process. For example,
	 *    a WebKit-based browser calls <code>QWebPage::findText()</code>.
	 *    The implementation may also call SetSuccessful() to indicate
	 *    whether anything has been found.
	 * -# An object of the subclass is created as a child of some page
	 *    containing searchable text, like a web page or a text document.
	 * -# It's hidden after that to not disturb the user until he
	 *    explicitly wishes to search for text.
	 * -# A QAction is created to trigger showing this notification, and
	 *    its <code>triggered()</code> signal is connected to this class'
	 *    show() and setFocus() slots (latter is needed so that user
	 *    can start typing his search query immediately).
	 * -# Optionally a couple of QShortCuts or QActions can be created
	 *    and connected to findNext() and findPrevious() slots to support
	 *    shortcuts for the corresponding actions.
	 *
	 * The FindNotificationWk class provides some utilities to aid
	 * integrating this class with a QWebPage.
	 *
	 * @sa FindNotificationWk
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API FindNotification : public PageNotification
	{
		Q_OBJECT

		Ui::FindNotification *Ui_;
	public:
		/** Various options controlling the search behavior.
		 */
		enum FindFlag
		{
			/** Search should be performed case sensitively.
			 */
			FindCaseSensitively,

			/** Search should be performed in the reverse direction.
			 */
			FindBackwards,

			/** Search should continue from the beginning when the end is
			 * reached (or from the end if the beginning is reached and
			 * FindBackwards is also set).
			 */
			FindWrapsAround
		};
		Q_DECLARE_FLAGS (FindFlags, FindFlag)

		/** @brief Creates the search widget in parent layout of \em near.
		 *
		 * Embedding is done only if possible — that is, if parent's
		 * layout is QVBoxLayout. Otherwise one should place this widget
		 * where needed himself.
		 *
		 * @param[in] proxy The core proxy to be used by this find
		 * notification.
		 * @param[in] near The widget near which to embed.
		 */
		FindNotification (ICoreProxy_ptr proxy, QWidget *near);
		~FindNotification ();

		/** @brief Sets the text in the find field.
		 *
		 * @param[in] text The text to set in find field.
		 */
		void SetText (const QString& text);

		/** @brief Returns the currently entered text in the find field.
		 *
		 * @return Currently entered text in the find field.
		 */
		QString GetText () const;

		/** @brief Updates the widget to show whether the search has been
		 * successful.
		 *
		 * @param[in] successful Whether the search has been successful.
		 */
		void SetSuccessful (bool successful);

		/** @brief Returns the current find flags except the direction.
		 *
		 * Please note that the direction flag (FindBackwards) never
		 * appears in the return result.
		 *
		 * @return The find flags corresponding to the user choices.
		 */
		FindFlags GetFlags () const;
	protected:
		/** @brief Called each time the user requests a search.
		 *
		 * Reimplement this function to perform the actual search.
		 *
		 * @param[in] text The text to search for.
		 * @param[in] flags The flags to search with.
		 */
		virtual void handleNext (const QString& text, FindFlags flags) = 0;
	public slots:
		/** @brief Search for the next occurrence of the current search.
		 */
		void findNext ();
		/** @brief Search for the previous occurrence of the current search.
		 */
		void findPrevious ();
	private slots:
		void on_Pattern__textChanged (const QString&);
		void on_FindButton__released ();
		virtual void reject ();
	};
}
}
