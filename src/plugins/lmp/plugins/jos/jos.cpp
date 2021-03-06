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

#include "jos.h"
#include "devmanager.h"
#include "uploadmanager.h"

namespace LeechCraft
{
namespace LMP
{
namespace jOS
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		DevManager_ = new DevManager (this);
		connect (DevManager_,
				SIGNAL (availableDevicesChanged ()),
				this,
				SIGNAL (availableDevicesChanged ()));

		UpManager_ = new UploadManager (this);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.jOS";
	}

	QString Plugin::GetName () const
	{
		return "LMP jOS";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with iOS-based devices like iPhone, iPod Touch and iPad.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CollectionSync";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	QString Plugin::GetSyncSystemName () const
	{
		return "iOS";
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	UnmountableDevInfos_t Plugin::AvailableDevices () const
	{
		return DevManager_->GetDevices ();
	}

	void Plugin::SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info)
	{
		UpManager_->SetInfo (origLocalPath, info);
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& to, const QByteArray&)
	{
		UpManager_->Upload (localPath, origLocalPath, to);
	}

	void Plugin::Refresh ()
	{
		DevManager_->refresh ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_jos, LeechCraft::LMP::jOS::Plugin);
