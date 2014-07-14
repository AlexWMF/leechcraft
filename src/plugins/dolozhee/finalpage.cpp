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

#include "finalpage.h"
#include <QNetworkReply>
#include <QtDebug>
#include <QDomDocument>
#include <QFileInfo>
#include <util/xpc/util.h>
#include <util/sys/mimedetector.h>
#include "reportwizard.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "xmlgenerator.h"
#include "fileattachpage.h"

namespace LeechCraft
{
namespace Dolozhee
{
	FinalPage::FinalPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
		Ui_.UploadProgress_->hide ();
	}

	void FinalPage::initializePage ()
	{
		auto wiz = qobject_cast<ReportWizard*> (wizard ());

		for (const auto& file : wiz->GetFilePage ()->GetFiles ())
			PendingFiles_.push_back ({ file, QString (), QString (), QString () });

		UploadPending ();
	}

	void FinalPage::UploadPending ()
	{
		auto wiz = qobject_cast<ReportWizard*> (wizard ());

		Ui_.UploadProgress_->hide ();

		if (!PendingFiles_.isEmpty ())
		{
			CurrentUpload_ = PendingFiles_.takeFirst ();
			QFile file (CurrentUpload_.Name_);
			if (!file.open (QIODevice::ReadOnly))
				return UploadPending ();

			auto reply = wiz->PostRequest ("/uploads.xml",
					file.readAll (), "application/octet-stream");
			connect (reply,
					SIGNAL (finished ()),
					this,
					SLOT (handleUploadReplyFinished ()));
			Ui_.Status_->setText (tr ("Sending %1...")
					.arg ("<em>" + QFileInfo (CurrentUpload_.Name_).fileName () + "</em>"));

			connect (reply,
					SIGNAL (uploadProgress (qint64, qint64)),
					this,
					SLOT (handleUploadProgress (qint64)));
			Ui_.UploadProgress_->setMaximum (file.size ());
			Ui_.UploadProgress_->show ();

			return;
		}

		Util::MimeDetector detector;
		for (auto& item : UploadedFiles_)
			item.Mime_ = detector (item.Name_);

		const auto typePage = wiz->GetReportTypePage ();
		const auto type = typePage->GetReportType ();
		const auto category = typePage->GetCategoryID ();

		QString title;
		QString desc;
		switch (type)
		{
		case ReportTypePage::Type::Bug:
			title = wiz->GetBugReportPage ()->GetTitle ();
			desc = wiz->GetBugReportPage ()->GetText ();
			break;
		case ReportTypePage::Type::Feature:
			title = wiz->GetFRPage ()->GetTitle ();
			desc = wiz->GetFRPage ()->GetText ();
			break;
		}

		const auto& data = XMLGenerator ().CreateIssue (title, desc,
				category, type, typePage->GetPriority (), UploadedFiles_);
		auto reply = wiz->PostRequest ("/issues.xml", data);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void FinalPage::handleUploadProgress (qint64 done)
	{
		Ui_.UploadProgress_->setValue (done);
	}

	void FinalPage::handleUploadReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& replyData = reply->readAll ();

		QDomDocument doc;
		if (!doc.setContent (replyData))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply"
					<< replyData;
			UploadPending ();
			return;
		}

		CurrentUpload_.Token_ = doc
				.documentElement ()
				.firstChildElement ("token")
				.text ();
		UploadedFiles_ << CurrentUpload_;

		UploadPending ();
	}

	void FinalPage::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		QString text;

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			text = tr ("I'm very sorry to say that, but seems like "
					"we're unable to handle your report at the time :(");
			Ui_.Status_->setText (text);
			return;
		}

		auto root = doc.documentElement ();
		const auto& id = root.firstChildElement ("id").text ();
		text = tr ("Report has been sent successfully. Thanks for your time!");
		if (!id.isEmpty ())
		{
			text += "<br />";
			text += (tr ("Your issue number is %1. You can view it here:") +
						" <a href='http://dev.leechcraft.org/issues/%1'>#%1</a>.<br/>"
						"You can also track it via an Atom feed reader by adding "
						"its <a href='http://dev.leechcraft.org/issues/%1.atom'>feed</a>.").arg (id);
		}
		Ui_.Status_->setText (text);
	}

	void FinalPage::on_Status__linkActivated (const QString& linkStr)
	{
		emit gotEntity (Util::MakeEntity (QUrl (linkStr),
					QString (),
					OnlyHandle | FromUserInitiated));
	}
}
}
