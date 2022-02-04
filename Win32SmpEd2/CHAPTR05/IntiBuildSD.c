/* PROGRAM 5-3 (ALTERNATIVE) STARTS HERE AND ENDS WHERE NOTED BELOW.
	Modified Version - Can only be built with Visual C++ 5.0 or later
	because it uses the BuildSecurityDescriptor function */
/*	WARNING: THIS DOES NOT WORK, BUT IT SHOULD (I THINK) ACCORDING TO
	THE DOCUMENTATION. FIXING IT WILL REMAIN AS AN "EXERCISE FOR THE READER."
/* InitBuildSD.c */

/* These functions maintain UNIX-style permissions
		in a Win32 (Windows NT only) SECURITY_ATTRIBUTES structure.
	There are three entries:
		InitializeUnixSD: Allocate & initialize a security descriptor.
		ChangeFilePermissions: Modify file's security.
		ReadFilePermissions: Interrogate a file's security.

	There are a number of assumptions and limitations:
		You can only modify and read a structure previously
			created by InitializeUnixSA.
		All permissions are the UNIX style:
			[R,W,X] - [User, Group, Other].
		The group is taken from the Group SID of the process.
			This may really represent several groups.
			You can, however, apply generic rights to ANY securable object.
		The change and read entries require the HANDLE of the
			object and obtain its SA. */

#include "EvryThng.h"
#include <accctrl.h>
#include <aclapi.h>

#define ACL_SIZE 1024
#define DOM_SIZE LUSIZE

#define NUM_BITS 9

static VOID FindGroup (DWORD, LPTSTR);


PSECURITY_DESCRIPTOR InitializeSD (DWORD UnixPerms,
		LPTSTR UsrNam, LPTSTR GrpNam, LPDWORD AceMasks)

/*	Create a structure and set the UNIX style permissions
	as specified in UnixPerms, which is 9-bits
	(low-order end) giving the required [R,W,X] settings
	for [User,Group,Other] in the familiar UNIX form. 
	Return a pointer to a security descriptor structure. */

{
	LONG iBit;
	DWORD ErrNum = 0;
	LPTSTR TrusteeNames [3] = { NULL, NULL, _T("EVERYONE")};
	TCHAR GroupName [MAX_NAME];

	TRUSTEE Trustees[2];  /* The two trustees - User, Group - for the SD */
//	{	{ NULL, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_NAME, TRUSTEE_IS_USER,  TrusteeNames[0] },
//		{ NULL, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_NAME, TRUSTEE_IS_GROUP, TrusteeNames[1] } };

	EXPLICIT_ACCESS AccessEntries [NUM_BITS];
	ULONG SizeNewSD = 0xFFFFFFFF;
	PSECURITY_DESCRIPTOR pNewSD = NULL;

	Trustees[0].pMultipleTrustee = Trustees[1].pMultipleTrustee = NULL;
	Trustees[0].MultipleTrusteeOperation = Trustees[1].MultipleTrusteeOperation =
		NO_MULTIPLE_TRUSTEE;
	Trustees[0].TrusteeForm = Trustees[1].TrusteeForm = TRUSTEE_IS_NAME;
	Trustees[0].TrusteeType = TRUSTEE_IS_USER;
	Trustees[1].TrusteeType = TRUSTEE_IS_GROUP;

	printf ("GetLastError: %d\n", GetLastError());

	TrusteeNames [0] = Trustees[0].ptstrName = _T("CURRENT_USER");/*UsrNam;*/
	if (GrpNam == NULL || _tcslen(GrpNam) == 0) { 
		/*  No group name specified. Get the user's primary group and
			override initial settings. */
		FindGroup (2, GroupName);
		Trustees[1].ptstrName = TrusteeNames [1] = GroupName;
	} else Trustees[1].ptstrName = TrusteeNames[1] = GrpNam;

	printf ("Group Name: %s\n", Trustees[1].ptstrName);
	
	/*	Set all the access entry members and submembers */
	for (iBit = 0; iBit < NUM_BITS /* 9 */; iBit++) {
		/* This is a strange function as it does not return any status
			AND IT DOES NOT APPEAR TO WORK. 
			OTHER THAN THAT, IT'S A GOOD FUNCTION AND CERTAINLY IS EASIER TO USE */
		BuildExplicitAccessWithName (
			&AccessEntries [iBit],
			TrusteeNames [iBit / 3],
			AceMasks [iBit % 3],
			((UnixPerms >> (8 - iBit) & 0x1) == 1) ? GRANT_ACCESS : DENY_ACCESS,
			NO_INHERITANCE);
	}

	if ((ErrNum = BuildSecurityDescriptor (&Trustees[0], &Trustees[1],
		NUM_BITS, AccessEntries,	/* Nine Access entries corresponding to 9 permission bits. */
		0, NULL,					/* No Audit entries */
		NULL,						/* No preexisting Security Descriptor */
		&SizeNewSD, &pNewSD)) != ERROR_SUCCESS) { 
			/* This function does not set the last error number */
			printf ("SizeNewSD: %d, pNewSD: %x", SizeNewSD, pNewSD);
			SetLastError (ErrNum);
	}

	return pNewSD;
}

/* PROGRAM 7-3 ENDS HERE. */


/* PROGRAM 7-4 STARTS HERE AND ENDS WHERE NOTED BELOW. */

DWORD ReadFilePermissions (LPCTSTR lpFileName, LPTSTR UsrNm, LPTSTR GrpNm)

/* Return the UNIX style permissions for a file. */
{
	PSECURITY_DESCRIPTOR pSD = NULL;
	DWORD LenNeeded, PBits, iAce;
	BOOL DaclF, AclDefF, OwnerDefF, GroupDefF;
	BYTE DAcl [ACL_SIZE];
	PACL pAcl = (PACL) &DAcl;
	ACL_SIZE_INFORMATION ASizeInfo;
	PACCESS_ALLOWED_ACE pAce;
	BYTE AType;
	HANDLE ProcHeap = GetProcessHeap ();
	PSID pOwnerSid, pGroupSid;
	TCHAR RefDomain [2] [DOM_SIZE];
	DWORD RefDomCnt [2] = {DOM_SIZE, DOM_SIZE};
	DWORD AcctSize [2] = {ACCT_NAME_SIZE, ACCT_NAME_SIZE};
	SID_NAME_USE sNamUse [] = {SidTypeUser, SidTypeGroup};

__try {
	/* Get the required size for the security descriptor. */

	GetFileSecurity (lpFileName, OWNER_SECURITY_INFORMATION |
			GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			pSD, 0, &LenNeeded);

	/* Create a security descriptor. */

	pSD = HeapAlloc (ProcHeap, HEAP_GENERATE_EXCEPTIONS, LenNeeded);
	if (!GetFileSecurity (lpFileName, OWNER_SECURITY_INFORMATION |
			GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			pSD, LenNeeded, &LenNeeded))
		ReportError (_T ("GetFileSecurity error"), 30, TRUE);
	
	if (!GetSecurityDescriptorDacl (pSD, &DaclF, &pAcl, &AclDefF))
		ReportError (_T ("GetSecurityDescriptorDacl error"), 31, TRUE);

	/* Get the number of ACEs in the ACL. */

	if (!GetAclInformation (pAcl, &ASizeInfo,
			sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
		ReportError (_T ("GetAclInformation error"), 32, TRUE);


	/* Get Each Ace. We know that this ACL was created by
		our InitializeUnixSA function, so the ACES are in the
		same order as the UNIX permission bits. */

	PBits = 0;
	for (iAce = 0; iAce < ASizeInfo.AceCount; iAce++) {
		GetAce (pAcl, iAce, &pAce);
		AType = pAce->Header.AceType;
		if (AType == ACCESS_ALLOWED_ACE_TYPE)
			PBits |= (0x1 << (8-iAce));
	}

	/* Find the name of the owner and owning group. */
	/* Find the SIDs first. */

	if (!GetSecurityDescriptorOwner (pSD, &pOwnerSid, &OwnerDefF))
		ReportException (_T ("GetSecurityDescOwner error"), 33);
	if (!GetSecurityDescriptorGroup (pSD, &pGroupSid, &GroupDefF))
		ReportException (_T ("GetSecurityDescGrp error"), 34);
	if (!LookupAccountSid (NULL, pOwnerSid, UsrNm, &AcctSize [0], RefDomain [0],
			&RefDomCnt [0], &sNamUse [0]))
		ReportException (_T ("LookUpAccountSid error"), 35);
	if (!LookupAccountSid (NULL, pGroupSid, GrpNm, &AcctSize [1], RefDomain [1],
			&RefDomCnt [1], &sNamUse [1]))
		ReportException (_T ("LookUpAccountSid error"), 36);

	return PBits;
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (pSD != NULL)
			HeapFree (ProcHeap, 0, pSD);
		return 0XFFFFFFFF;
	}
}

/* PROGRAM 7-4 ENDS HERE.  */


/* PROGRAM 7-5 STARTS HERE AND ENDS WHERE NOTED BELOW */

BOOL ChangeFilePermissions (DWORD fPerm, LPCTSTR FileName, LPDWORD AceMasks)

/* Change permissions in an existing file. The group is left unchanged. */

/* Strategy:
	1.	Obtain the existing security descriptor using
		the internal function ReadFilePermissions.
	2.	Create a security attribute for the owner and permission bits.
	3.	Extract the security descriptor.
	4.	Set the file security with the new descriptor. */
{
	TCHAR UsrNm [ACCT_NAME_SIZE], GrpNm [ACCT_NAME_SIZE];
	DWORD OldfPerm;
	PSECURITY_DESCRIPTOR pSD = NULL;

	/* Assure that the file exists.	*/

	if (_taccess (FileName, 0) != 0)	/* File does not exist. */
		return FALSE;
	OldfPerm = ReadFilePermissions (FileName, UsrNm, GrpNm);
	pSD = InitializeSD (fPerm, UsrNm, GrpNm, AceMasks);

	if (!SetFileSecurity (FileName, DACL_SECURITY_INFORMATION, pSD))
		ReportError (_T ("SetFileSecurity error"), 37, TRUE);
	return TRUE;
}

/* PROGRAM 7-5 ENDS HERE  */


/* This code is a hint as to how to solve Exercise 8-2. */

static VOID FindGroup (DWORD GroupNumber, LPTSTR GroupName)
/*	Find a group name associated with the owning user
	of the current process. */

{
	TCHAR RefDomain [DOM_SIZE];
	DWORD RefDomCnt = DOM_SIZE, AcctSize  = ACCT_NAME_SIZE;
	SID_NAME_USE GroupSidType  = SidTypeGroup;
	HANDLE tHandle;
	TOKEN_GROUPS TokenG[20]; /* You need some space for this. */
	DWORD TISize;

	if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &tHandle))
		ReportError (_T ("OpenProcessToken error"), 0, TRUE);
	if (!GetTokenInformation (tHandle, TokenGroups,
			&TokenG, sizeof (TokenG), &TISize)) {
		ReportError (_T ("GetTokenInfo error"), 0, TRUE);
	}

	/* Experimentation shows that the groups entered are
		as follows:
		0	 -	None
		1	 -	Everyone
		2	 -	The first non-trivial group
	 	3,.. -	Keep looking up to the count, which is part
				of the structure - see the documentation! */

	if (!LookupAccountSid (NULL, TokenG[0].Groups[GroupNumber].Sid,
			GroupName, &AcctSize, RefDomain, &RefDomCnt, &GroupSidType))
			ReportError (_T("Error looking up Account Name"), 0, TRUE);
	return;
}
