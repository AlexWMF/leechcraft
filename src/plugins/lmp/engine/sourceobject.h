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

#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMutex>
#include <QWaitCondition>
#include "audiosource.h"

typedef struct _GstElement GstElement;
typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;

namespace LeechCraft
{
namespace LMP
{
	class AudioSource;
	class Path;

	enum class SourceError
	{
		MissingPlugin,
		Other
	};

	enum class SourceState
	{
		Error,
		Stopped,
		Paused,
		Buffering,
		Playing
	};

	class SourceObject : public QObject
	{
		Q_OBJECT

		GstElement *Dec_;

		Path *Path_;

		AudioSource CurrentSource_;
		AudioSource NextSource_;

		QMutex NextSrcMutex_;
		QWaitCondition NextSrcWC_;

		bool IsSeeking_;

		qint64 LastCurrentTime_;

		uint PrevSoupRank_;

	public:
		typedef QMap<QString, QString> TagMap_t;
	private:
		TagMap_t Metadata_;
	public:
		enum class Metadata
		{
			Artist,
			Album,
			Title,
			Genre,
			Tracknumber,
			NominalBitrate,
			MinBitrate,
			MaxBitrate
		};
	private:
		SourceState OldState_;
	public:
		SourceObject (QObject* = 0);
		~SourceObject ();

		SourceObject (const SourceObject&) = delete;
		SourceObject& operator= (const SourceObject&) = delete;

		bool IsSeekable () const;
		SourceState GetState () const;

		QString GetErrorString () const;

		QString GetMetadata (Metadata) const;

		qint64 GetCurrentTime ();
		qint64 GetRemainingTime () const;
		qint64 GetTotalTime () const;
		void Seek (qint64);

		void SetTransitionTime (int);

		AudioSource GetCurrentSource () const;
		void SetCurrentSource (const AudioSource&);
		void PrepareNextSource (const AudioSource&);

		void Play ();
		void Pause ();
		void Stop ();

		void Clear ();
		void ClearQueue ();

		void HandleAboutToFinish ();
		void HandleErrorMsg (GstMessage*);
		void HandleTagMsg (GstMessage*);
		void HandleBufferingMsg (GstMessage*);
		void HandleStateChangeMsg (GstMessage*);
		void HandleElementMsg (GstMessage*);
		void HandleEosMsg (GstMessage*);
		void SetupSource ();

		void AddToPath (Path*);
		void PostAdd (Path*);
	private slots:
		void updateTotalTime ();
		void handleTick ();
	signals:
		void stateChanged (SourceState, SourceState);
		void currentSourceChanged (const AudioSource&);
		void aboutToFinish ();
		void finished ();
		void metaDataChanged ();
		void bufferStatus (int);
		void totalTimeChanged (qint64);

		void tick (qint64);

		void error (const QString&, SourceError);
	};
}
}