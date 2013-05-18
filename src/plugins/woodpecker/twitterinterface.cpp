/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
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

#include "twitterinterface.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QDebug>
#include <qjson/parser.h>
#include <QtKOAuth/QtKOAuth>

#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Woodpecker
{
	TwitterInterface::TwitterInterface (QObject *parent)
	: QObject (parent)
	{
		HttpClient_ = new QNetworkAccessManager (this);
		OAuthRequest_ = new KQOAuthRequest;
		OAuthManager_ = new KQOAuthManager (this);

		OAuthRequest_->setEnableDebugOutput (false); // DONE: Remove debug
		ConsumerKey_ = XmlSettingsManager::Instance ()->property ("consumer_key").toString ();
		ConsumerKeySecret_ = XmlSettingsManager::Instance ()->property ("consumer_key_secret").toString ();

		connect (OAuthManager_, SIGNAL (requestReady (QByteArray)),
				this, SLOT (onRequestReady (QByteArray)));

		connect (OAuthManager_, SIGNAL (authorizedRequestDone ()),
				this, SLOT (onAuthorizedRequestDone ()));

		connect (HttpClient_, SIGNAL (finished (QNetworkReply*)),
				this, SLOT (replyFinished (QNetworkReply*)));

	}

	TwitterInterface::~TwitterInterface ()
	{
		delete OAuthRequest_;
		delete OAuthManager_;
	}

	void TwitterInterface::RequestTwitter (QUrl requestAddress)
	{
		HttpClient_->get (QNetworkRequest (requestAddress));
		//           getAccess ();
		//           xauth ();
	}

	void TwitterInterface::replyFinished (QNetworkReply *reply)
	{
		QList<std::shared_ptr<Tweet>> tweets;
		QByteArray jsonText (reply->readAll ());
		reply->deleteLater ();

		tweets.clear ();
		tweets = ParseReply (jsonText);
		emit tweetsReady (tweets);
	}

	QList<std::shared_ptr<Tweet>> TwitterInterface::ParseReply (const QByteArray& json)
	{
		QJson::Parser parser;
		QList<std::shared_ptr<Tweet>> result;
		bool ok;

		QVariantList answers = parser.parse (json, &ok).toList ();

		if (!ok) 
			qWarning () << "Parsing error at parseReply " << QString::fromUtf8 (json);

		QVariantMap tweetMap;
		QVariantMap userMap;

		for (int i = answers.count () - 1; i >= 0 ; --i)
		{
			tweetMap = answers[i].toMap ();
			userMap = tweetMap["user"].toMap ();
			QLocale::setDefault (QLocale::English);
			Tweet_ptr tempTweet (new Tweet ());

			tempTweet->SetText (tweetMap["text"].toString ());
			tempTweet->Author ()->SetUsername (userMap["screen_name"].toString ());
			tempTweet->Author ()->DownloadAvatar (userMap["profile_image_url"].toString ());
			connect (tempTweet->Author ().get (), SIGNAL (userReady ()), 
					 parent (), SLOT (setUpdateReady ()));
			tempTweet->SetDateTime (QLocale ().toDateTime (tweetMap["created_at"].toString (), QLatin1String ("ddd MMM dd HH:mm:ss +0000 yyyy")));
			tempTweet->SetId (tweetMap["id"].toULongLong ());

			result.push_back (tempTweet);
		}

		return result;
	}

	void TwitterInterface::GetAccess () 
	{
		connect (OAuthManager_, SIGNAL (temporaryTokenReceived (QString, QString)),
				this, SLOT (onTemporaryTokenReceived (QString, QString)));

		connect (OAuthManager_, SIGNAL (authorizationReceived (QString, QString)),
				this, SLOT (onAuthorizationReceived (QString, QString)));

		connect (OAuthManager_, SIGNAL (accessTokenReceived (QString, QString)),
				this, SLOT (onAccessTokenReceived (QString, QString)));

		OAuthRequest_->initRequest (KQOAuthRequest::TemporaryCredentials, QUrl ("https://api.twitter.com/oauth/request_token"));
		OAuthRequest_->setConsumerKey (ConsumerKey_);
		OAuthRequest_->setConsumerSecretKey (ConsumerKeySecret_);
		OAuthManager_->setHandleUserAuthorization (true);

		OAuthManager_->executeRequest (OAuthRequest_);

	}

	void TwitterInterface::SignedRequest (TwitterRequest req, KQOAuthRequest::RequestHttpMethod method, KQOAuthParameters params)
	{
		QUrl reqUrl;

		if (Token_.isEmpty () || TokenSecret_.isEmpty ())
		{
			qWarning () << "No access tokens. Aborting.";
			return;
		}

		switch (req)
		{
		case TwitterRequest::TRHomeTimeline:
			reqUrl = "https://api.twitter.com/1/statuses/home_timeline.json";
			params.insert ("count", "50");
			params.insert ("include_entities", "true");
			break;
			
		case TwitterRequest::TRUserTimeline:
			reqUrl = "http://api.twitter.com/1/statuses/user_timeline.json";
			params.insert ("include_entities", "true");
			break;
			
		case TwitterRequest::TRUpdate:
			reqUrl = "http://api.twitter.com/1/statuses/update.json";
			break;
			
		case TwitterRequest::TRDirect:
			reqUrl = "https://api.twitter.com/1/direct_messages.json";
			
		case TwitterRequest::TRRetweet:
			reqUrl = QString ("http://api.twitter.com/1/statuses/retweet/").append (params.value ("rt_id")).append (".json");
			break;
			
		case TwitterRequest::TRReply:
			reqUrl = "http://api.twitter.com/1/statuses/update.json";
			break;
			
		case TwitterRequest::TRSPAMReport:
			reqUrl = "http://api.twitter.com/1/report_spam.json";
			break;
			
		default:
			return;
		}
		
		OAuthRequest_->initRequest (KQOAuthRequest::AuthorizedRequest, reqUrl);
		OAuthRequest_->setHttpMethod (method);
		OAuthRequest_->setConsumerKey (ConsumerKey_);
		OAuthRequest_->setConsumerSecretKey (ConsumerKeySecret_);
		OAuthRequest_->setToken (Token_);
		OAuthRequest_->setTokenSecret (TokenSecret_);
		OAuthRequest_->setAdditionalParameters (params);
		OAuthManager_->executeRequest (OAuthRequest_);
	}
	
	void TwitterInterface::SendTweet (const QString& tweet)
	{
		KQOAuthParameters param;
		param.insert ("status", tweet);
		SignedRequest (TwitterRequest::TRUpdate, KQOAuthRequest::POST, param);
	}

	void TwitterInterface::Retweet (qulonglong id)
	{
		KQOAuthParameters param;
		param.insert ("rt_id", QString::number (id));
		SignedRequest (TwitterRequest::TRRetweet, KQOAuthRequest::POST, param);
	}

	void TwitterInterface::Reply (long unsigned int replyid, QString tweet)
	{
		KQOAuthParameters param;
		param.insert ("status", tweet);
		param.insert ("in_reply_to_status_id", QString::number (replyid));
		SignedRequest (TwitterRequest::TRReply, KQOAuthRequest::POST, param);
	}


	void TwitterInterface::onAuthorizedRequestDone ()
	{
		qDebug () << "Request sent to Twitter!";
	}

	void TwitterInterface::onRequestReady (QByteArray response)
	{
		qDebug () << "Response from the service: recvd";// << response;
		emit tweetsReady (ParseReply (response));
	}

	void TwitterInterface::onAuthorizationReceived (QString token, QString verifier)
	{
		qDebug () << "User authorization received: " << token << verifier;

		OAuthManager_->getUserAccessTokens (QUrl ("https://api.twitter.com/oauth/access_token"));

		if (OAuthManager_->lastError () != KQOAuthManager::NoError) 
		{
		}

	}

	void TwitterInterface::onAccessTokenReceived (QString token, QString tokenSecret) 
	{
		qDebug () << "Access token received: " << token << tokenSecret;

		this->Token_ = token;
		this->TokenSecret_ = tokenSecret;

		qDebug () << "Access tokens now stored. You are ready to send Tweets from user's account!";

		emit authorized (token, tokenSecret);

	}

	void TwitterInterface::onTemporaryTokenReceived (QString token, QString tokenSecret)
	{
		qDebug () << "Temporary token received: " << token << tokenSecret;

		QUrl userAuthURL ("https://api.twitter.com/oauth/authorize");

		if (OAuthManager_->lastError () == KQOAuthManager::NoError) {
			qDebug () << "Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
			OAuthManager_->getUserAuthorization (userAuthURL);
		}

	}


	void TwitterInterface::Xauth () 
	{
		connect (OAuthManager_, SIGNAL (accessTokenReceived (QString, QString)),
				this, SLOT (onAccessTokenReceived (QString, QString)));

		KQOAuthRequest_XAuth *oauthRequest = new KQOAuthRequest_XAuth (this);
		oauthRequest->initRequest (KQOAuthRequest::AccessToken, QUrl ("https://api.twitter.com/oauth/access_token"));
		oauthRequest->setConsumerKey ("nbwLYUDIlgsMgDFCu6jfuA");
		oauthRequest->setConsumerSecretKey ("7TWYPzLUqZlihIRA2VWfZhCRfss2JNKvkSWMQx4");

		// oauthRequest->setXAuthLogin ("login", "password");

		OAuthManager_->executeRequest (oauthRequest);
	}

	void TwitterInterface::searchTwitter (QString text)
	{
		QString link ("http://search.twitter.com/search.json?q=" + text);
		SetLastRequestMode (FeedMode::FMSearchResult);
		RequestTwitter (link);
	}

	void TwitterInterface::getHomeFeed ()
	{
		qDebug () << "Getting home feed";
		SetLastRequestMode (FeedMode::FMHomeTimeline);
		SignedRequest (TwitterRequest::TRHomeTimeline, KQOAuthRequest::GET);
	}

	void TwitterInterface::getMoreTweets (QString last)
	{
		KQOAuthParameters param;

		qDebug () << "Getting more tweets from " << last;
		param.insert ("max_id", last);
		param.insert ("count", QString ("%1").arg (30));
		SetLastRequestMode (FeedMode::FMHomeTimeline);
		SignedRequest (TwitterRequest::TRHomeTimeline, KQOAuthRequest::GET, param);
	}


	void TwitterInterface::getUserTimeline (QString username)
	{
		KQOAuthParameters param;
		param.insert ("screen_name", username);
		SetLastRequestMode (FeedMode::FMUserTimeline);
		SignedRequest (TwitterRequest::TRUserTimeline, KQOAuthRequest::GET, param);
	}

	void TwitterInterface::Login (QString savedToken, QString savedTokenSecret)
	{
		Token_ = savedToken;
		TokenSecret_ = savedTokenSecret;
		qDebug () << "Successfully logged in";
	}

	void TwitterInterface::ReportSPAM (QString username, long unsigned int userid)
	{
		KQOAuthParameters param;

		param.insert ("screen_name", username);
		if (userid)
			param.insert ("user_id", QString::number (userid));
		SignedRequest (TwitterRequest::TRSPAMReport, KQOAuthRequest::POST, param);
	}
	
	FeedMode TwitterInterface::GetLastRequestMode ()
	{
		return  LastRequestMode_;
	}
	
	void TwitterInterface::SetLastRequestMode (FeedMode newLastRequestMode)
	{
		LastRequestMode_ = newLastRequestMode;
	}
}
}

