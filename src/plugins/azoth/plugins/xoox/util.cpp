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

#include "util.h"
#include <memory>
#include <QObject>
#include <QHash>
#include <QDomDocument>
#include <QDomElement>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QXmppPresence.h>
#include <QXmppUtils.h>
#include <QXmppGlobal.h>
#include "entrybase.h"
#include "core.h"
#include "capsdatabase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
namespace XooxUtil
{
	QString RoleToString (const QXmppMucItem::Role& role)
	{
		switch (role)
		{
		case QXmppMucItem::NoRole:
			return QObject::tr ("guest");
		case QXmppMucItem::VisitorRole:
			return QObject::tr ("visitor");
		case QXmppMucItem::ParticipantRole:
			return QObject::tr ("participant");
		case QXmppMucItem::ModeratorRole:
			return QObject::tr ("moderator");
		default:
			return QObject::tr ("unspecified");
		}
	}

	QString AffiliationToString (const QXmppMucItem::Affiliation& affiliation)
	{
		switch (affiliation)
		{
		case QXmppMucItem::OutcastAffiliation:
			return QObject::tr ("outcast");
		case QXmppMucItem::NoAffiliation:
			return QObject::tr ("newcomer");
		case QXmppMucItem::MemberAffiliation:
			return QObject::tr ("member");
		case QXmppMucItem::AdminAffiliation:
			return QObject::tr ("admin");
		case QXmppMucItem::OwnerAffiliation:
			return QObject::tr ("owner");
		default:
			return QObject::tr ("unspecified");
		}
	}

	namespace
	{
		struct Node2ClientID
		{
			QHash<QString, QString> Node2ClientID_;
			QHash<QString, QString> Node2ClientIDBegin_;

			Node2ClientID ()
			{
				Node2ClientID_ ["http://2010.qip.ru/caps"] = "qipinfium";
				Node2ClientID_ ["http://agent.mail.ru"] = "mailruagent";
				Node2ClientID_ ["http://bitlbee.org/xmpp/caps"] = "bitlbee";
				Node2ClientID_ ["http://bombus-im.org/java"] = "bombus";
				Node2ClientID_ ["http://bombusmod.net.ru/caps"] = "bombusmod";
				Node2ClientID_ ["http://bombusmod-qd.wen.ru/caps"] = "bombusmodqd";
				Node2ClientID_ ["http://code.google.com/p/qxmpp"] = "qxmpp";
				Node2ClientID_ ["http://emacs-jabber.sourceforge.net"] = "jabber-el";
				Node2ClientID_ ["http://emess.eqx.su/caps"] = "emess";
				Node2ClientID_ ["http://fatal-bot.spb.ru/caps"] = "fatal-bot";
				Node2ClientID_ ["http://fatal-dev.ru/bot/caps"] = "fatal-bot";
				Node2ClientID_ ["http://isida-bot.com"] = "isida-bot";
				Node2ClientID_ ["http://isida-bot.com/4"] = "isida-bot4";
				Node2ClientID_ ["httр://jabga.ru"] = "jtalk";
				Node2ClientID_ ["http://jabiru.mzet.net/caps"] = "jabiru";
				Node2ClientID_ ["http://jasmineicq.ru/caps"] = "jasmine";
				Node2ClientID_ ["http://jimm.net.ru/caps"] = "jimm";
				Node2ClientID_ ["http://jitsi.org"] = "sip-communicator";
				Node2ClientID_ ["http://jtalk.ustyugov.net/caps"] = "jtalk";
				Node2ClientID_ ["http://kadu.im/caps"] = "kadu";
				Node2ClientID_ ["http://kopete.kde.org/jabber/caps"] = "kopete";
				Node2ClientID_ ["http://leechcraft.org/azoth"] = "leechcraft-azoth";
				Node2ClientID_ ["http://mail.google.com/xmpp/client/caps"] = "mail.google.com";
				Node2ClientID_ ["http://mcabber.com/caps"] = "mcabber";
				Node2ClientID_ ["http://miranda-im.org/caps"] = "miranda";
				Node2ClientID_ ["http://miranda-ng.org/caps"] = "miranda-ng";
				Node2ClientID_ ["http://online.yandex.ru/caps"] = "yaonline";
				Node2ClientID_ ["http://palringo.com/caps"] = "palringo";
				Node2ClientID_ ["http://pda.qip.ru/caps"] = "qippda";
				Node2ClientID_ ["http://psi-im.org/caps"] = "psi";
				Node2ClientID_ ["http://psi-dev.googlecode.com/caps"] = "psiplus";
				Node2ClientID_ ["http://pyicqt.googlecode.com//protocol/caps"] = "pyicq-t";
				Node2ClientID_ ["http://qip.ru/caps"] = "qipinfium";
				Node2ClientID_ ["http://qip.ru/caps?QIP Mobile Java"] = "qipmobile";
				Node2ClientID_ ["http://sawim.ru/caps"] = "sawim";
				Node2ClientID_ ["http://sip-communicator.org"] = "sip-communicator";
				Node2ClientID_ ["http://spectrum.im/transport"] = "spectrum";
				Node2ClientID_ ["httр://stranger.kiev.ua/caps"] = "jtalk";
				Node2ClientID_ ["http://swift.im"] = "swift";
				Node2ClientID_ ["http://talk.google.com/xmpp/bot/caps"] = "talk.google.com";
				Node2ClientID_ ["http://talkgadget.google.com/client/caps"] = "talkgadget.google.com";
				Node2ClientID_ ["http://telepathy.freedesktop.org/caps"] = "telepathy.freedesktop.org";
				Node2ClientID_ ["http://trillian.im/caps"] = "trillian";
				Node2ClientID_ ["http://v4.isida-bot.com"] = "isida-bot4";
				Node2ClientID_ ["http://vacuum-im.googlecode.com"] = "vacuum";
				Node2ClientID_ ["http://www.android.com/gtalk/client/caps"] = "android";
				Node2ClientID_ ["http://www.android.com/gtalk/client/caps2"] = "android";
				Node2ClientID_ ["http://www.apple.com/ichat/caps"] = "ichat";
				Node2ClientID_ ["http://www.google.com/xmpp/client/caps"] = "talk.google.com";
				Node2ClientID_ ["http://www.eyecu.ru"] = "eyecu";
				Node2ClientID_ ["httр://www.freq-bot.net/"] = "freqbot";
				Node2ClientID_ ["http://www.igniterealtime.org/projects/smack/"] = "smack";
				Node2ClientID_ ["http://www.lonelycatgames.com/slick/caps"] = "slick";
				Node2ClientID_ ["https://www.jappix.com/"] = "jappix";

				Node2ClientIDBegin_ ["http://bombus-im.org/java#"] = "bombus";
				Node2ClientIDBegin_ ["http://gajim.org"] = "gajim";
				Node2ClientIDBegin_ ["http://pidgin.im/"] = "pidgin";
				Node2ClientIDBegin_ ["http://qutim.org"] = "qutim";
				Node2ClientIDBegin_ ["http://tkabber.jabber.ru"] = "tkabber";

				Node2ClientID_ ["none"] = "unknown";
			}
		};
	}

	QString GetClientIDName (const QString& node)
	{
		static Node2ClientID n2ci;
		const QString& result = n2ci.Node2ClientID_.value (node);
		if (!result.isEmpty ())
			return result;

		const auto& begins = n2ci.Node2ClientIDBegin_;
		for (auto i = begins.begin (), end = begins.end (); i != end; ++i)
			if (node.startsWith (i.key ()))
				return i.value ();

		return QString ();
	}

	namespace
	{
		struct Node2ClientHR
		{
			QHash<QString, QString> Node2ClientHR_;
			QHash<QString, QString> Node2ClientHRBegin_;

			Node2ClientHR ()
			{
				Node2ClientHR_ ["http://2010.qip.ru/caps"] = "QIP Infium";
				Node2ClientHR_ ["http://agent.mail.ru"] = "Mail.Ru Agent";
				Node2ClientHR_ ["http://bitlbee.org/xmpp/caps"] = "Bitlbee";
				Node2ClientHR_ ["http://bombus-im.org/java"] = "Bombus";
				Node2ClientHR_ ["http://bombusmod.net.ru/caps"] = "BombusMod";
				Node2ClientHR_ ["http://bombusmod-qd.wen.ru/caps"] = "BombusMod-QD";
				Node2ClientHR_ ["http://code.google.com/p/qxmpp"] = "QXmpp library";
				Node2ClientHR_ ["http://emacs-jabber.sourceforge.net"] = "jabber.el";
				Node2ClientHR_ ["http://emess.eqx.su/caps"] = "EMess";
				Node2ClientHR_ ["http://fatal-bot.spb.ru/caps"] = "Fatal-bot";
				Node2ClientHR_ ["http://fatal-dev.ru/bot/caps"] = "Fatal-bot";
				Node2ClientHR_ ["http://isida-bot.com"] = "iSida Bot";
				Node2ClientHR_ ["http://isida-bot.com/4"] = "iSida Bot";
				Node2ClientHR_ ["httр://jabga.ru"] = "Fin jabber";
				Node2ClientHR_ ["http://jabiru.mzet.net/caps"] = "Jabiru";
				Node2ClientHR_ ["http://jasmineicq.ru/caps"] = "Jasmine";
				Node2ClientHR_ ["http://jimm.net.ru/caps"] = "Jimm";
				Node2ClientHR_ ["http://jitsi.org"] = "Jitsi";
				Node2ClientHR_ ["http://jtalk.ustyugov.net/caps"] = "JTalk";
				Node2ClientHR_ ["http://kadu.im/caps"] = "Kadu IM";
				Node2ClientHR_ ["http://kopete.kde.org/jabber/caps"] = "Kopete";
				Node2ClientHR_ ["http://leechcraft.org/azoth"] = "LeechCraft Azoth";
				Node2ClientHR_ ["http://mail.google.com/xmpp/client/caps"] = "GMail chat widget";
				Node2ClientHR_ ["http://mcabber.com/caps"] = "MCabber";
				Node2ClientHR_ ["http://miranda-im.org/caps"] = "Miranda IM";
				Node2ClientHR_ ["http://miranda-ng.org/caps"] = "Miranda NG";
				Node2ClientHR_ ["http://online.yandex.ru/caps"] = "Ya.Online";
				Node2ClientHR_ ["http://palringo.com/caps"] = "Palringo";
				Node2ClientHR_ ["http://pda.qip.ru/caps"] = "QIP PDA";
				Node2ClientHR_ ["http://psi-im.org/caps"] = "Psi";
				Node2ClientHR_ ["http://psi-dev.googlecode.com/caps"] = "Psi+";
				Node2ClientHR_ ["http://pyicqt.googlecode.com//protocol/caps"] = "PyICQ-t";
				Node2ClientHR_ ["http://qip.ru/caps"] = "QIP Infium";
				Node2ClientHR_ ["http://qip.ru/caps?QIP Mobile Java"] = "QIP Mobile";
				Node2ClientHR_ ["http://sawim.ru/caps"] = "Sawim";
				Node2ClientHR_ ["http://sip-communicator.org"] = "SIP Communicator";
				Node2ClientHR_ ["http://spectrum.im/transport"] = "Spectrum XMPP Gateway";
				Node2ClientHR_ ["httр://stranger.kiev.ua/caps"] = "Fin Jimm";
				Node2ClientHR_ ["http://swift.im"] = "Swift";
				Node2ClientHR_ ["http://talk.google.com/xmpp/bot/caps"] = "Google Talk";
				Node2ClientHR_ ["http://talkgadget.google.com/client/caps"] = "Google Talk gadget";
				Node2ClientHR_ ["http://telepathy.freedesktop.org/caps"] = "Telepathy";
				Node2ClientHR_ ["http://trillian.im/caps"] = "Trillian";
				Node2ClientHR_ ["http://v4.isida-bot.com"] = "iSida Bot 4";
				Node2ClientHR_ ["http://vacuum-im.googlecode.com"] = "Vacuum-IM";
				Node2ClientHR_ ["http://www.android.com/gtalk/client/caps"] = "Android";
				Node2ClientHR_ ["http://www.android.com/gtalk/client/caps2"] = "Android";
				Node2ClientHR_ ["http://www.apple.com/ichat/caps"] = "iChat";
				Node2ClientHR_ ["http://www.google.com/xmpp/client/caps"] = "Google Talk";
				Node2ClientHR_ ["http://www.eyecu.ru"] = "EyeCU";
				Node2ClientHR_ ["httр://www.freq-bot.net/"] = "freQ bot";
				Node2ClientHR_ ["http://www.igniterealtime.org/projects/smack/"] = "Smack XMPP library";
				Node2ClientHR_ ["http://www.lonelycatgames.com/slick/caps"] = "Slick";
				Node2ClientHR_ ["https://www.jappix.com/"] = "Jappix";

				Node2ClientHRBegin_ ["http://bombus-im.org/java#"] = "Bombus";
				Node2ClientHRBegin_ ["http://gajim.org"] = "Gajim";
				Node2ClientHRBegin_ ["http://pidgin.im/"] = "Pidgin IM";
				Node2ClientHRBegin_ ["http://qutim.org"] = "QutIM";
				Node2ClientHRBegin_ ["http://tkabber.jabber.ru"] = "Tkabber";

				Node2ClientHR_ ["none"] = "Unknown";
			}
		};
	}

	QString GetClientHRName (const QString& node)
	{
		static Node2ClientHR n2ch;
		const QString& result = n2ch.Node2ClientHR_.value (node);
		if (!result.isEmpty ())
			return result;

		const auto& begins = n2ch.Node2ClientHRBegin_;
		for (auto i = begins.begin (), end = begins.end (); i != end; ++i)
			if (node.startsWith (i.key ()))
				return i.value ();

		return QString ();
	}

	QDomElement XmppElem2DomElem (const QXmppElement& elem)
	{
		QByteArray arr;
		QXmlStreamWriter w (&arr);
		elem.toXml (&w);

		QDomDocument doc;
		doc.setContent (arr, true);
		return doc.documentElement ();
	}

	QXmppElement Form2XmppElem (const QXmppDataForm& form)
	{
		QByteArray formData;
		QXmlStreamWriter w (&formData);
		form.toXml (&w);
		QDomDocument doc;
		doc.setContent (formData);
		return doc.documentElement ();
	}

	bool RunFormDialog (QWidget *widget)
	{
		QDialog *dialog (new QDialog ());
		dialog->setWindowTitle (widget->windowTitle ());
		dialog->setLayout (new QVBoxLayout ());
		dialog->layout ()->addWidget (widget);
		QDialogButtonBox *box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		dialog->layout ()->addWidget (box);
		QObject::connect (box,
				SIGNAL (accepted ()),
				dialog,
				SLOT (accept ()));
		QObject::connect (box,
				SIGNAL (rejected ()),
				dialog,
				SLOT (reject ()));

		const bool result = dialog->exec () == QDialog::Accepted;
		dialog->deleteLater ();
		return result;
	}

	bool CheckUserFeature (EntryBase *base, const QString& variant, const QString& feature)
	{
		if (variant.isEmpty ())
			return true;

		const QByteArray& ver = base->GetVariantVerString (variant);
		if (ver.isEmpty ())
			return true;

		const QStringList& feats = Core::Instance ().GetCapsDatabase ()->Get (ver);
		if (feats.isEmpty ())
			return true;

		return feats.contains (feature);
	}

	QXmppMessage Forwarded2Message (const QXmppElement& wrapper)
	{
		const auto& forwardedElem = wrapper.tagName () == "forwarded" ?
				wrapper :
				wrapper.firstChildElement ("forwarded");
		if (forwardedElem.isNull ())
			return {};

		const auto& messageElem = forwardedElem.firstChildElement ("message");
		if (messageElem.isNull ())
			return {};

		QXmppMessage original;
#if QXMPP_VERSION >= 0x000800
		original.parse (messageElem.sourceDomElement ());
#else
#warning "You won't have good forwarded messages, Message Archive Management and Message Carbons will look like crap."
		original.parse (XmppElem2DomElem (messageElem));
#endif

		auto delayElem = forwardedElem.firstChildElement ("delay");
		if (!delayElem.isNull ())
		{
			const auto& sourceDT = QXmppUtils::datetimeFromString (delayElem.attribute ("stamp"));
			original.setStamp (sourceDT.toLocalTime ());
		}

		return original;
	}

	EntryStatus PresenceToStatus (const QXmppPresence& pres)
	{
		EntryStatus st (static_cast<State> (pres.availableStatusType () + 1), pres.statusText ());
		if (pres.type () == QXmppPresence::Unavailable)
			st.State_ = SOffline;
		return st;
	}

	QXmppPresence StatusToPresence (State state, const QString& text, int prio)
	{
		QXmppPresence::Type presType = state == SOffline ?
				QXmppPresence::Unavailable :
				QXmppPresence::Available;

		QXmppPresence pres (presType);
		if (state != SOffline)
			pres.setAvailableStatusType (static_cast<QXmppPresence::AvailableStatusType> (state - 1));
		pres.setStatusText (text);
		pres.setPriority (prio);

		return pres;
	}
}
}
}
}
