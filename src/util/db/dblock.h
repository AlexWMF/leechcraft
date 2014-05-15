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

#include <QMutex>
#include <QString>
#include <QSet>
#include "dbconfig.h"

class QSqlError;
class QSqlQuery;
class QSqlDatabase;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief Provides database transaction lock.
		 *
		 * To use the lock, create an instance of it passing a non-const
		 * reference to the QSqlDatabase to be guarded. To initialize and start
		 * the locking mechanism, call Init(). To notify DBLock that everything
		 * is good and the database shouldn't be rolled back, call Good().
		 * Transaction would be either commited or rolled back in class'
		 * destructor.
		 *
		 * The state could be unusable, usable or correct. Unusable means that
		 * the class itself isn't usable, usable means that the class is usable,
		 * but the transaction state isn't necesseraly in a correct state, and
		 * correct means that the lock class is usable and the transaction state
		 * is correct.
		 *
		 * @ingroup DbUtil
		 */
		class DBLock
		{
			QSqlDatabase &Database_;

			bool Good_;
			bool Initialized_;

			static QMutex LockedMutex_;
			static QSet<QString> LockedBases_;
		public:
			DBLock (const DBLock&) = delete;
			DBLock& operator= (const DBLock&) = delete;

			/** @brief Constructor.
			 *
			 * Constructs the lock and prepares it to work with the database.
			 * Creating the lock doesn't begin the transaction. Lock is in
			 * usable state after that.
			 *
			 * @param[in] database Non-const reference to the database to be
			 * guarded.
			 */
			UTIL_DB_API DBLock (QSqlDatabase& database);
			/** @brief Destructor.
			 *
			 * Ends the transaction if the lock is in a correct state. If Good()
			 * was called, it commits the transaction, otherwise rolls back.
			 */
			UTIL_DB_API ~DBLock ();

			/** @brief Initializes the transaction.
			 *
			 * Tries to start the transaction. If this wasn't successful, the
			 * lock remains in a usable but not correct state.
			 *
			 * @throw std::runtime_error
			 */
			UTIL_DB_API void Init ();
			/** @brief Notifies the lock about successful higher-level
			 * operations.
			 *
			 * Calling this function makes the lock to commit the transaction
			 * upon destruction instead of rolling back.
			 */
			UTIL_DB_API void Good ();

			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] error The error class.
			 */
			UTIL_DB_API static void DumpError (const QSqlError& error);
			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] query The query that should be dumped.
			 */
			UTIL_DB_API static void DumpError (const QSqlQuery& query);
		};
	};
};