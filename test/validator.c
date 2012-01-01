
#include "test.h"
#include "mvParser.h"

TEST validity_assign() {
	mvStrArray script(2);
	script.append(REQ23);
	script.append(REQ6);

	mvSession session;
	mvCommand cmd;
	try
	{
		session.perform(script);
		singletonParser.parse(cmd, REQ24);
	}
	catch (mvError* err)
	{
		FAIL(err);
	}

	try
	{
		session.execute(cmd);
	}
	catch (mvError* err)
	{
		ASSERT_ERROR(err, MVERROR_INVALID);
		mvError_release(err);
	}
}

