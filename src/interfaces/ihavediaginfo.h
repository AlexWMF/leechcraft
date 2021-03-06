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

#ifndef INTERFACES_IDIAGINFO_H
#define INTERFACES_IDIAGINFO_H
#include <QtPlugin>

class QString;

/** @brief Interface for plugins having human-readable diagnostic data.
 *
 * Library versions, the compiled-in features, etc., count as diagnostic
 * info. The info aggregated from all plugins will be sent to the issue
 * tracker with bug reports, for example, so returning proper data can
 * aid in debugging.
 */
class Q_DECL_EXPORT IHaveDiagInfo
{
public:
	virtual ~IHaveDiagInfo () {}

	/** @brief Returns the human-readable diagnostic info.
	 *
	 * @return The human-readable diagnostic info.
	 */
	virtual QString GetDiagInfoString () const = 0;
};

Q_DECLARE_INTERFACE (IHaveDiagInfo, "org.Deviant.LeechCraft.IHaveDiagInfo/1.0");

#endif
