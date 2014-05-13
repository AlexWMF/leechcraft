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
#include <stdexcept>
#include <type_traits>
#include <memory>
#include <vector>
#include <QIcon>
#include <QString>
#include <QFile>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include <interfaces/azoth/iprotocol.h>

extern "C"
{
#include <libotr/privkey.h>
}

namespace LeechCraft
{
namespace Azoth
{
namespace OTRoid
{
	QIcon GetAccountIcon (IAccount *account)
	{
		const auto accObj = account->GetQObject ();

		const auto extSelf = qobject_cast<IExtSelfInfoAccount*> (accObj);
		auto icon = extSelf ? extSelf->GetAccountIcon () : QIcon {};

		if (icon.isNull ())
			icon = qobject_cast<IProtocol*> (account->GetParentProtocol ())->GetProtocolIcon ();

		return icon;
	}

	namespace
	{
		void SexpWrite (QFile& file, gcry_sexp_t sexp)
		{
			const auto buflen = gcry_sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, nullptr, 0);
			QByteArray buf (buflen, 0);
			gcry_sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, buf.data (), buflen);
			file.write (buf);
		}

		void WriteAcc (QFile& file, const char *accName, const char *proto, gcry_sexp_t privkey)
		{
			file.write (" (account\n");

			gcry_sexp_t names;
			if (gcry_sexp_build (&names, nullptr, "(name %s)", accName))
				throw std::runtime_error ("cannot save keys");
			std::shared_ptr<std::remove_pointer<gcry_sexp_t>::type> namesGuard { names, gcry_sexp_release };
			SexpWrite (file, names);

			gcry_sexp_t protos;
			if (gcry_sexp_build (&protos, nullptr, "(protocol %s)", proto))
				throw std::runtime_error ("cannot save keys");
			std::shared_ptr<std::remove_pointer<gcry_sexp_t>::type> protosGuard { protos, gcry_sexp_release };
			SexpWrite (file, protos);

			SexpWrite (file, privkey);

			file.write (" )\n");
		}
	}

	void WriteKeys (OtrlUserState state, const QString& filename)
	{
		const auto& tmpFilename = filename + ".new";
		QFile file { tmpFilename };
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
			throw std::runtime_error ("cannot open keys file");

		file.write ("(privkeys\n");

		for (auto pkey = state->privkey_root; pkey; pkey = pkey->next)
			WriteAcc (file, pkey->accountname, pkey->protocol, pkey->privkey);

		file.write (")\n");
		file.flush ();

		std::shared_ptr<std::remove_pointer<OtrlUserState>::type> testState
		{
			otrl_userstate_create (),
			&otrl_userstate_free
		};
		if (otrl_privkey_read (testState.get (), tmpFilename.toUtf8 ().constData ()))
			throw std::runtime_error ("failed to save the keys");

		QFile::remove (filename);
		file.rename (filename);
	}
}
}
}
